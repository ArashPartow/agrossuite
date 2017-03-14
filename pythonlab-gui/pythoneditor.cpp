// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "pythoneditor.h"
#include "pythonhighlighter.h"
#include "pythonconsole.h"
#include "pythonbrowser.h"
#include "pythoncompleter.h"

#include "other/util.h"
// #include "other/global.h"
#include "other/filebrowser.h"

#include "QPrintDialog"

const QString TABS = "    ";
const int TABS_SIZE = 4;

int firstNonSpace(const QString& text)
{
    int i = 0;
    while (i < text.size())
    {
        if (!text.at(i).isSpace())
            return i;
        ++i;
    }
    return i;
}

int indentedColumn(int column, bool doIndent)
{
    int aligned = (column / TABS_SIZE) * TABS_SIZE;
    if (doIndent)
        return aligned + TABS_SIZE;
    if (aligned < column)
        return aligned;
    return qMax(0, aligned - TABS_SIZE);
}

int columnAt(const QString& text, int position)
{
    int column = 0;
    for (int i = 0; i < position; ++i)
    {
        if (text.at(i) == QLatin1Char('\t'))
            column = column - (column % TABS_SIZE) + TABS_SIZE;
        else
            ++column;
    }
    return column;
}

PythonEditorTextEdit::PythonEditorTextEdit(PythonEngine *pythonEngine, QWidget *parent)
    : QWidget(parent), m_pythonEngine(pythonEngine), m_fileName("")
{
    txtEditor = new ScriptEditor(pythonEngine, this);
    searchWidget = new SearchWidget(txtEditor, this);

    QVBoxLayout *layoutEditor = new QVBoxLayout();
    layoutEditor->addWidget(txtEditor);
    layoutEditor->addWidget(searchWidget);

    QWidget *editor = new QWidget();
    editor->setLayout(layoutEditor);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(editor);

    // contents
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(1);
    layout->addWidget(splitter);

    setLayout(layout);

    QSettings settings;
    restoreGeometry(settings.value("PythonEditorWidget/Geometry", saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("PythonEditorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PythonEditorWidget/SplitterGeometry").toByteArray());
    txtEditor->resize(txtEditor->height(), settings.value("PythonEditorWidget/EditorHeight").toInt());

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pyFlakesAnalyse()));
    timer->start(4000);

    txtEditor->setAcceptDrops(false);
}

PythonEditorTextEdit::~PythonEditorTextEdit()
{
    QSettings settings;

    settings.setValue("PythonEditorWidget/Geometry", saveGeometry());
    settings.setValue("PythonEditorWidget/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorWidget/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("PythonEditorWidget/EditorHeight", txtEditor->height());
}

void PythonEditorTextEdit::pyFlakesAnalyse()
{
    if (isVisible() && !m_pythonEngine->isScriptRunning())
    {
        QString fn = tempProblemFileName() + ".pyflakes_str.py";
        QString str = txtEditor->toPlainText();
        writeStringContent(fn, &str);

        QStringList messages = m_pythonEngine->codePyFlakes(fn);

        txtEditor->errorMessagesPyFlakes.clear();
        foreach (QString line, messages)
        {
            if (!line.isEmpty())
            {
                int number;
                QString message;

                QStringList list = line.split(":");
                if (list.count() == 3)
                {
                    number = list[1].toInt();
                    message = list[2];

                    txtEditor->errorMessagesPyFlakes[number] = message;
                }
            }
        }

        QFile::remove(fn);

        txtEditor->repaint();
    }
}

void PythonEditorTextEdit::doHighlightLine(QTreeWidgetItem *item, int role)
{
    if (item)
    {
        int line = item->data(0, Qt::UserRole).value<int>();

        txtEditor->gotoLine(line, true);
    }
}

// ***********************************************************************************************************

PythonEditorWidget::PythonEditorWidget(PythonEditorView *parent) : QWidget(parent), pythonEditor(parent)
{
    createControls();
}

PythonEditorWidget::~PythonEditorWidget()
{
    QSettings settings;
    settings.setValue("PythonEditorWidget/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorWidget/SplitterGeometry", splitter->saveGeometry());
}

void PythonEditorWidget::createControls()
{
    fileBrowser = new FileBrowser(this);
    fileBrowser->setNameFilter("*.py");
    connect(fileBrowser, SIGNAL(fileItemDoubleClick(QString)), pythonEditor, SLOT(doFileItemDoubleClick(QString)));

    consoleHistory = new PythonScriptingHistory(pythonEditor->m_console, this);
    variableBrowser = new PythonBrowser(pythonEditor->m_console, this);

    QTabWidget *tab = new QTabWidget();
    tab->addTab(consoleHistory, tr("History"));
    tab->addTab(variableBrowser, tr("Variables"));

    splitter = new QSplitter();
    splitter->setContentsMargins(2, 2, 2, 2);
    splitter->setOrientation(Qt::Vertical);
    splitter->setStretchFactor(1, 3);
    splitter->setStretchFactor(2, 6);
    splitter->addWidget(fileBrowser);
    splitter->addWidget(tab);

    toolBar = new QToolBar(this);
    toolBar->addAction(pythonEditor->actFileNew);
    toolBar->addAction(pythonEditor->actFileOpen);
    toolBar->addAction(pythonEditor->actFileSave);
    toolBar->addSeparator();
    toolBar->addAction(pythonEditor->actRunPython);
    toolBar->addAction(pythonEditor->actStopPython);
    toolBar->addSeparator();
    toolBar->addAction(pythonEditor->actUseProfiler);
    toolBar->addAction(pythonEditor->actConsoleOutput);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(toolBar);
    layout->addWidget(splitter);

    setLayout(layout);

    QSettings settings;
    splitter->restoreState(settings.value("PythonEditorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PythonEditorWidget/SplitterGeometry").toByteArray());
}

// ***********************************************************************************************************

PythonEditorView::PythonEditorView(QWidget *parent)
    : QWidget(parent), pythonEngine(currentPythonEngine())
{
    createStatusBar();
    createActions();
    createControls();

    QSettings settings;
    m_pythonEditorWidget->fileBrowser->setDir(settings.value("PythonEditorDialog/WorkDir").value<QString>());
    m_pythonEditorWidget->fileBrowser->refresh();

    connect(actRunPython, SIGNAL(triggered()), this, SLOT(doRunPython()));
    connect(actStopPython, SIGNAL(triggered()), this, SLOT(doStopScript()));
    connect(actReplaceTabsWithSpaces, SIGNAL(triggered()), this, SLOT(doReplaceTabsWithSpaces()));
    connect(actGotoToFileDirectory, SIGNAL(triggered()), this, SLOT(doGotoFileDirectory()));

    // connect(pythonEngine, SIGNAL(startedScript()), this, SLOT(doStartedScript()));
    // connect(pythonEngine, SIGNAL(executedScript()), this, SLOT(doExecutedScript()));

    setAcceptDrops(true);

    splitter->restoreState(settings.value("PythonEditorDialog/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PythonEditorDialog/SplitterGeometry").toByteArray());
    restoreGeometry(settings.value("PythonEditorDialog/Geometry", saveGeometry()).toByteArray());

    // set recent files
    setRecentFiles();
}

PythonEditorView::~PythonEditorView()
{
    QSettings settings;
    settings.setValue("PythonEditorDialog/Geometry", saveGeometry());
    settings.setValue("PythonEditorDialog/SplitterState", splitter->saveState());
    settings.setValue("PythonEditorDialog/SplitterGeometry", splitter->saveGeometry());
}

void PythonEditorView::closeEvent(QCloseEvent *event)
{
    // check script editor
    closeTabs();

    if (isScriptModified())
    {
        // show script editor
        if (isScriptModified())
            show();
    }

    event->ignore();
}

void PythonEditorView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PythonEditorView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void PythonEditorView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QString fileName = QUrl(event->mimeData()->urls().at(0)).toLocalFile().trimmed();
        if (QFile::exists(fileName))
        {
            QFileInfo fileInfo(fileName);
            doFileOpen(fileInfo.absoluteFilePath());

            event->acceptProposedAction();
        }
    }
}

void PythonEditorView::createActions()
{
    QSettings settings;

    actFileNew = new QAction(iconAwesome(fa::fileo), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::AddTab);
    connect(actFileNew, SIGNAL(triggered()), this, SLOT(doFileNew()));

    actFileOpen = new QAction(iconAwesome(fa::folderopen), tr("&Open..."), this);
    actFileOpen->setShortcuts(QKeySequence::Open);
    connect(actFileOpen, SIGNAL(triggered()), this, SLOT(doFileOpen()));

    actFileSave = new QAction(iconAwesome(fa::save), tr("&Save"), this);
    actFileSave->setShortcuts(QKeySequence::Save);
    connect(actFileSave, SIGNAL(triggered()), this, SLOT(doFileSave()));

    actFileSaveAs = new QAction(tr("Save &as..."), this);
    actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

    actFileOpenRecentGroup = new QActionGroup(this);
    connect(actFileOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doFileOpenRecent(QAction *)));

    actFileClose = new QAction(tr("&Close"), this);
    actFileClose->setShortcuts(QKeySequence::Close);
    connect(actFileClose, SIGNAL(triggered()), this, SLOT(doFileClose()));

    actFilePrint = new QAction(tr("&Print"), this);
    actFilePrint->setShortcuts(QKeySequence::Print);
    connect(actFilePrint, SIGNAL(triggered()), this, SLOT(doFilePrint()));

    actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

    actUndo = new QAction(iconAwesome(fa::undo), tr("&Undo"), this);
    actUndo->setShortcut(QKeySequence::Undo);

    actRedo = new QAction(iconAwesome(fa::rotateleft), tr("&Redo"), this);
    actRedo->setShortcut(QKeySequence::Redo);

    actCut = new QAction(iconAwesome(fa::cut), tr("Cu&t"), this);
    actCut->setShortcut(QKeySequence::Cut);
    actCut->setEnabled(false);

    actCopy = new QAction(iconAwesome(fa::copy), tr("&Copy"), this);
    actCopy->setShortcut(QKeySequence::Copy);
    actCopy->setEnabled(false);

    actPaste = new QAction(iconAwesome(fa::paste), tr("&Paste"), this);
    actPaste->setShortcut(QKeySequence::Paste);

    actFind = new QAction(iconAwesome(fa::search), tr("&Find"), this);
    actFind->setShortcut(QKeySequence::Find);
    connect(actFind, SIGNAL(triggered()), this, SLOT(doFind()));

    actFindNext = new QAction(tr("Find &next"), this);
    actFindNext->setShortcut(QKeySequence::FindNext);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(doFindNext()));

    actReplace = new QAction(tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actReplace = new QAction(tr("Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actIndentSelection = new QAction(tr("Indent"), this);
    actIndentSelection->setShortcut(tr("Ctrl+>"));
    actUnindentSelection = new QAction(tr("Unindent"), this);
    actUnindentSelection->setShortcut(tr("Ctrl+<"));

    actCommentAndUncommentSelection = new QAction(tr("Toggle comment selection"), this);
    actCommentAndUncommentSelection->setShortcut(tr("Ctrl+/"));

    actGotoLine = new QAction(tr("Goto line"), this);
    actGotoLine->setShortcut(tr("Alt+G"));

    actGotoToFileDirectory = new QAction(icon("options-plugin"), tr("Goto file directory"), this);

    actRunPython = new QAction(iconAwesome(fa::play), tr("&Run script"), this);
    actRunPython->setShortcut(QKeySequence(tr("Ctrl+R")));

    actStopPython = new QAction(iconAwesome(fa::stop), tr("Stop Python script"), this);
    actStopPython->setEnabled(false);

    actReplaceTabsWithSpaces = new QAction(tr("Replace tabs with spaces"), this);

    actPrintSelection = new QAction(tr("Print selection"), this);
    actPrintSelection->setShortcut(tr("F5"));
    connect(actPrintSelection, SIGNAL(triggered()), this, SLOT(doPrintSelection()));

    actUseProfiler = new QAction(iconAwesome(fa::clocko), tr("Profiler"), this);
    actUseProfiler->setCheckable(true);
    actUseProfiler->setChecked(settings.value("PythonLab/UseProfiler", true).toBool());
    connect(actUseProfiler, SIGNAL(triggered()), this, SLOT(doUseProfiler()));

    actConsoleOutput = new QAction(iconAwesome(fa::filecodeo), tr("Console output"), this);
    actConsoleOutput->setCheckable(true);
    actUseProfiler->setChecked(settings.value("PythonLab/ConsoleOutput", true).toBool());
    connect(actConsoleOutput, SIGNAL(triggered()), this, SLOT(doConsoleOutput()));
}

void PythonEditorView::createControls()
{
    mnuRecentFiles = new QMenu(tr("&Recent files"), this);

    mnuFile = new QMenu(tr("&File"), this);
    mnuFile->addAction(actFileNew);
    mnuFile->addAction(actFileOpen);
    mnuFile->addAction(actFileSave);
    mnuFile->addAction(actFileSaveAs);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actFilePrint);
    mnuFile->addSeparator();
    mnuFile->addAction(actFileClose);
    mnuFile->addAction(actExit);

    mnuEdit = new QMenu(tr("&Edit"), this);
    mnuEdit->addAction(actFind);
    mnuEdit->addAction(actFindNext);
    mnuEdit->addAction(actReplace);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actIndentSelection);
    mnuEdit->addAction(actUnindentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCommentAndUncommentSelection);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actGotoLine);

    mnuTools = new QMenu(tr("&Tools"), this);
    mnuTools->addAction(actReplaceTabsWithSpaces);
    mnuTools->addSeparator();
    mnuTools->addAction(actPrintSelection);

    QAction *actAboutQt = new QAction(tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    mnuHelp = new QMenu(tr("&Help"), this);
    mnuHelp->addAction(actAboutQt); // will be added to "PythonLab" MacOSX menu

    // contents
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);

    QToolButton *btnNewTab = new QToolButton(this);
    btnNewTab->setAutoRaise(true);
    btnNewTab->setToolTip(tr("Add new document"));
    btnNewTab->setIcon(iconAwesome(fa::plus));
    btnNewTab->setIconSize(QSize(24, 24));
    btnNewTab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    tabWidget->setCornerWidget(btnNewTab, Qt::TopLeftCorner);
    connect(btnNewTab, SIGNAL(clicked()), this, SLOT(doFileNew()));

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(doCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(doCurrentPageChanged(int)));

    errorWidget = new ErrorWidget(tabWidget, this);

    m_console = new PythonScriptingConsole(pythonEngine, this);

    // main widget
    QVBoxLayout *layoutTab = new QVBoxLayout();
    layoutTab->addWidget(tabWidget);
    layoutTab->addWidget(errorWidget);
    QWidget *widgetTab = new QWidget(this);
    widgetTab->setLayout(layoutTab);

    splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(widgetTab);
    splitter->addWidget(m_console);

    m_pythonEditorWidget = new PythonEditorWidget(this);

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->setContentsMargins(2, 2, 2, 2);
    layoutLab->addWidget(m_pythonEditorWidget);
    layoutLab->addWidget(splitter);

    setLayout(layoutLab);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));

    // new file
    doFileNew();
    // recent files
    setRecentFiles();
}

void PythonEditorView::createStatusBar()
{
    // lblCurrentPosition = new QLabel("Status");

    // statusBar()->showMessage(tr("Ready"));
    // statusBar()->addPermanentWidget(lblCurrentPosition);
}

void PythonEditorView::doRunPython()
{
    if (pythonEngine->isScriptRunning())
        return;

    // clear errors
    scriptEditorWidget()->txtEditor->errorMessagesError.clear();

    if (!scriptEditorWidget()->fileName().isEmpty())
        m_pythonEditorWidget->fileBrowser->setDir(QFileInfo(scriptEditorWidget()->fileName()).absolutePath());

    // connect stdout and set current path
    m_console->connectStdOut(QFile::exists(scriptEditorWidget()->fileName()) ?
                                 QFileInfo(scriptEditorWidget()->fileName()).absolutePath() : "");

    // run script
    QTime time;
    time.start();
    m_console->consoleMessage(tr("Run script: %1\n").arg(tabWidget->tabText(tabWidget->currentIndex()).replace("* ", "")), Qt::gray);

    bool successfulRun = false;
    if (txtEditor->textCursor().hasSelection())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        successfulRun = pythonEngine->runScript(txtEditor->textCursor().selectedText().replace(0x2029, "\n"),
                                                fileInfo.exists() ? fileInfo.absoluteFilePath() : "");
    }
    else if (scriptEditorWidget()->fileName().isEmpty())
    {
        successfulRun = pythonEngine->runScript(txtEditor->toPlainText());
    }
    else
    {
        if (!scriptEditorWidget()->fileName().isEmpty() &&
                QFile::exists(scriptEditorWidget()->fileName()))
            doFileSave();

        // set profiler
        QSettings settings;
        bool useProfiler = actUseProfiler->isChecked(); // settings.value("PythonLab/UseProfiler", true).toBool();

        // set profiler
        pythonEngine->useProfiler(useProfiler);
        successfulRun = pythonEngine->runScript(txtEditor->toPlainText(),
                                                QFileInfo(scriptEditorWidget()->fileName()).absoluteFilePath());
        // reset profiler
        pythonEngine->useProfiler(!useProfiler);

        // set profiled
        txtEditor->setProfiled(useProfiler);

        txtEditor->setProfilerAccumulatedLines(currentPythonEngine()->profilerAccumulatedLines());
        txtEditor->setProfilerAccumulatedTimes(currentPythonEngine()->profilerAccumulatedTimes());

        txtEditor->setProfilerMaxAccumulatedLine(currentPythonEngine()->profilerMaxAccumulatedLine());
        txtEditor->setProfilerMaxAccumulatedTime(currentPythonEngine()->profilerMaxAccumulatedTime());
        txtEditor->setProfilerMaxAccumulatedCallLine(currentPythonEngine()->profilerMaxAccumulatedCallLine());
        txtEditor->setProfilerMaxAccumulatedCall(currentPythonEngine()->profilerMaxAccumulatedCall());

        // refresh
        txtEditor->updateLineNumberAreaWidth();
    }

    // run script
    m_console->consoleMessage(tr("Finish script: %1\n").arg(milisecondsToTime(time.elapsed()).toString("hh:mm:ss.zzz")), Qt::gray);

    // disconnect stdout
    m_console->disconnectStdOut();

    if (!successfulRun)
    {
        // parse error
        ErrorResult result = pythonEngine->parseError();

        m_console->stdErr(result.error());
        m_console->stdErr("\nStacktrace:\n");
        m_console->stdErr(result.tracebackToString());

        if (!txtEditor->textCursor().hasSelection() && result.line() >= 0)
            txtEditor->gotoLine(result.line(), true);

        errorWidget->showError(result);
    }
    else
    {
        errorWidget->setVisible(false);
    }

    m_console->appendCommandPrompt();

    txtEditor->setFocus();
}

void PythonEditorView::doStopScript()
{
    actStopPython->setEnabled(false);

    // run script
    m_console->consoleMessage(tr("\nScript is being aborted.\n"), Qt::blue);

    currentPythonEngine()->abortScript();
    QApplication::processEvents();
}

void PythonEditorView::doStartedScript()
{
    // disable controls
    setEnabledControls(false);
    scriptEditorWidget()->setCursor(Qt::BusyCursor);

    actRunPython->setEnabled(false);
    actStopPython->setEnabled(true);
}

void PythonEditorView::doExecutedScript()
{
    // enable controls
    setEnabledControls(true);
    scriptEditorWidget()->setCursor(Qt::ArrowCursor);

    actRunPython->setEnabled(true);
    actStopPython->setEnabled(false);
}

void PythonEditorView::setEnabledControls(bool state)
{
    txtEditor->setEnabled(state);
    m_console->setEnabled(state);
}

void PythonEditorView::doReplaceTabsWithSpaces()
{
    txtEditor->replaceTabsWithSpaces();
}

void PythonEditorView::doGotoFileDirectory()
{
    if (!scriptEditorWidget()->fileName().isEmpty())
        m_pythonEditorWidget->fileBrowser->setDir(QFileInfo(scriptEditorWidget()->fileName()).absolutePath());
}

void PythonEditorView::doFileItemDoubleClick(const QString &path)
{
    QFileInfo fileInfo(path);

    QSettings settings;
    if (QDir(path).exists())
        settings.setValue("PythonEditorDialog/WorkDir", path);
    else
    {
        settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());

        if (fileInfo.suffix() == "py")
            doFileOpen(fileInfo.absoluteFilePath());
    }
}

void PythonEditorView::doPathChangeDir()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select directory"), m_pythonEditorWidget->fileBrowser->basePath(), options);
    if (!directory.isEmpty())
        m_pythonEditorWidget->fileBrowser->setDir(directory);
}

void PythonEditorView::doFileNew()
{
    PythonEditorTextEdit *editor = new PythonEditorTextEdit(pythonEngine, this);
    tabWidget->addTab(editor, tr("Untitled"));
    tabWidget->setCurrentIndex(tabWidget->count()-1);
    doCurrentPageChanged(tabWidget->count()-1);
}

void PythonEditorView::doFileOpen(const QString &file)
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), dir, tr("Python scripts (*.py)"));

    // read text
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "py")
            return;

        PythonEditorTextEdit *scriptEditor = scriptEditorWidget();

        for (int i = 0; i < tabWidget->count(); i++)
        {
            PythonEditorTextEdit *scriptEditorWidgetTmp = dynamic_cast<PythonEditorTextEdit *>(tabWidget->widget(i));
            if (scriptEditorWidgetTmp->fileName() == fileName)
            {
                tabWidget->setCurrentIndex(i);
                // QMessageBox::information(this, tr("Information"), tr("Script is already opened."));
                return;
            }
        }

        // check empty document
        if (!scriptEditor->txtEditor->toPlainText().isEmpty())
        {
            doFileNew();
            // new widget
            scriptEditor = scriptEditorWidget();
        }

        scriptEditor->setFileName(fileName);
        txtEditor->setPlainText(readFileContent(scriptEditor->fileName()));

        setRecentFiles();

        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());

        if (fileInfo.absoluteDir() != tempProblemDir() && !fileName.contains("resources/examples"))
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorView::doFileOpenAndFind(const QString &file, const QString &find)
{
    doFileOpen(file);

    // search
    QTextCursor cursor = txtEditor->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    cursor = txtEditor->document()->find(find, txtEditor->textCursor());

    if (!cursor.isNull())
    {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
        txtEditor->setTextCursor(cursor);
        txtEditor->ensureCursorVisible();
        txtEditor->highlightCurrentLine(true);
        setFocus();
    }
}


void PythonEditorView::doFileOpenRecent(QAction *action)
{
    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        doFileOpen(fileName);
        setRecentFiles();
    }
}

void PythonEditorView::doFileSave()
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    // save dialog
    if (scriptEditorWidget()->fileName().isEmpty())
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python scripts (*.py)"));

        if (!fileName.isEmpty())
        {
            if (QFileInfo(fileName).suffix() != "py")
                fileName += ".py";

            scriptEditorWidget()->setFileName(fileName);
        }
    }

    // write text
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        if (fileInfo.suffix() != "py")
            scriptEditorWidget()->setFileName(scriptEditorWidget()->fileName() + ".py");

        QFile fileName(scriptEditorWidget()->fileName());
        if (fileName.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&fileName);
            out << txtEditor->toPlainText();
            fileName.close();

            setRecentFiles();

            tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
            txtEditor->document()->setModified(false);
        }
        else
        {
            // throw AgrosException(tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->fileName));
            qDebug() << tr("File '%1' cannot be saved.").arg(scriptEditorWidget()->fileName());
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorView::doFileSaveAs()
{
    QSettings settings;
    QString dir = settings.value("PythonEditorDialog/WorkDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Python scripts (*.py)"));
    if (!fileName.isEmpty())
    {
        if (QFileInfo(fileName).suffix() != "py")
            fileName += ".py";

        scriptEditorWidget()->setFileName(fileName);
        doFileSave();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("PythonEditorDialog/WorkDir", fileInfo.absolutePath());
    }
}

void PythonEditorView::doFileClose()
{
    doCloseTab(tabWidget->currentIndex());
}

void PythonEditorView::doFilePrint()
{
    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog printDialog(&printer, this);
    printDialog.addEnabledOption(QAbstractPrintDialog::PrintCollateCopies);
    printDialog.setWindowTitle(tr("Print Document"));
    if (printDialog.exec() == QDialog::Accepted)
    {
        txtEditor->print(&printer);
    }
}

void PythonEditorView::doFind()
{
    QTextCursor cursor = txtEditor->textCursor();
    scriptEditorWidget()->searchWidget->showFind(cursor.selectedText());
}

void PythonEditorView::doFindNext()
{        
    scriptEditorWidget()->searchWidget->findNext();
}

void PythonEditorView::doReplace()
{
    QTextCursor cursor = txtEditor->textCursor();
    scriptEditorWidget()->searchWidget->showReplaceAll(cursor.selectedText());
}

void PythonEditorView::doDataChanged()
{
    actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void PythonEditorView::doPrintSelection()
{
    // connect stdout and set current path
    m_console->connectStdOut();

    m_console->stdOut(QString("%1: ").arg(txtEditor->textCursor().selectedText()));

    QString str = QString("print(%1)").arg(txtEditor->textCursor().selectedText());
    currentPythonEngine()->runScript(str);
    m_console->appendCommandPrompt();

    // disconnect stdout
    m_console->disconnectStdOut();
}

void PythonEditorView::onOtherInstanceMessage(const QString &msg)
{
    QStringList args = msg.split("#!#");
    for (int i = 1; i < args.count()-1; i++)
    {
        QString fileName =
                QFile::exists(args[i]) ? args[i] : QApplication::applicationDirPath() + QDir::separator() + args[i];

        if (QFile::exists(fileName))
            doFileOpen(fileName);
    }

    // setWindowState(Qt::WindowMinimized);
    // setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    show();
}

void PythonEditorView::doCloseTab(int index)
{
    tabWidget->setCurrentIndex(index);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }

    while (txtEditor->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"), tr("File '%1' has been modified.\nDo you want to save your changes?").arg(fileName),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            doFileSave();
        else if (ret == QMessageBox::Discard)
            break;
        else if (ret == QMessageBox::Cancel)
            return;
    }

    if (tabWidget->count() == 1)
    {
        doFileNew();
    }

    tabWidget->removeTab(index);
}

void PythonEditorView::doCurrentPageChanged(int index)
{
    txtEditor = scriptEditorWidget()->txtEditor;

    actCut->disconnect();
    connect(actCut, SIGNAL(triggered()), txtEditor, SLOT(cut()));
    actCopy->disconnect();
    connect(actCopy, SIGNAL(triggered()), txtEditor, SLOT(copy()));
    actPaste->disconnect();
    connect(actPaste, SIGNAL(triggered()), txtEditor, SLOT(paste()));
    actUndo->disconnect();
    connect(actUndo, SIGNAL(triggered()), txtEditor, SLOT(undo()));
    actRedo->disconnect();
    connect(actRedo, SIGNAL(triggered()), txtEditor, SLOT(redo()));

    actIndentSelection->disconnect();
    connect(actIndentSelection, SIGNAL(triggered()), txtEditor, SLOT(indentSelection()));
    actUnindentSelection->disconnect();
    connect(actUnindentSelection, SIGNAL(triggered()), txtEditor, SLOT(unindentSelection()));
    actCommentAndUncommentSelection->disconnect();
    connect(actCommentAndUncommentSelection, SIGNAL(triggered()), txtEditor, SLOT(commentAndUncommentSelection()));
    actGotoLine->disconnect();
    connect(actGotoLine, SIGNAL(triggered()), txtEditor, SLOT(gotoLine()));

    actPrintSelection->disconnect();
    connect(actPrintSelection, SIGNAL(triggered()), this, SLOT(doPrintSelection()));

    txtEditor->document()->disconnect(actUndo);
    txtEditor->document()->disconnect(actRedo);
    connect(txtEditor->document(), SIGNAL(undoAvailable(bool)), actUndo, SLOT(setEnabled(bool)));
    connect(txtEditor->document(), SIGNAL(redoAvailable(bool)), actRedo, SLOT(setEnabled(bool)));
    txtEditor->disconnect(actCut);
    txtEditor->disconnect(actCopy);
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    // modifications
    connect(txtEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(doCurrentDocumentChanged(bool)));

    actUndo->setEnabled(txtEditor->document()->isUndoAvailable());
    actRedo->setEnabled(txtEditor->document()->isRedoAvailable());

    // tabWidget->setTabsClosable(tabWidget->count() > 1);
    tabWidget->setTabsClosable(true);
    tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(true);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("PythonLab - %1").arg(fileName));

    txtEditor->setFocus();
}

void PythonEditorView::doCurrentDocumentChanged(bool changed)
{
    // modified
    QString fileName = tr("Untitled");
    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        fileName = fileInfo.completeBaseName();
    }

    if (changed)
        tabWidget->setTabText(tabWidget->currentIndex(), QString("* %1").arg(fileName));
    else
        tabWidget->setTabText(tabWidget->currentIndex(), fileName);
}

void PythonEditorView::doUseProfiler()
{
    QSettings settings;
    settings.setValue("PythonLab/UseProfiler", !settings.value("PythonLab/UseProfiler", true).toBool());
}

void PythonEditorView::doConsoleOutput()
{
    QSettings settings;
    settings.setValue("PythonLab/ConsoleOutput", !settings.value("PythonLab/ConsoleOutput", true).toBool());
}

void PythonEditorView::setRecentFiles()
{
    mnuRecentFiles->clear();

    // recent files
    QSettings settings;
    QStringList recentScripts = settings.value("RecentScripts").value<QStringList>();

    if (!scriptEditorWidget()->fileName().isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget()->fileName());
        if (recentScripts.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentScripts.insert(0, fileInfo.absoluteFilePath());
        else
            recentScripts.move(recentScripts.indexOf(fileInfo.absoluteFilePath()), 0);
    }

    while (recentScripts.count() > 15) recentScripts.removeLast();

    settings.setValue("RecentScripts", recentScripts);

    for (int i = 0; i < recentScripts.count(); i++)
    {
        QFileInfo fileInfo(recentScripts[i]);
        if (fileInfo.isDir())
            continue;
        if (!QFile::exists(fileInfo.absoluteFilePath()))
            continue;

        QAction *actMenuRecentItem = new QAction(recentScripts[i], this);
        actFileOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

void PythonEditorView::closeTabs()
{
    for (int i = tabWidget->count()-1; i >= 0 ; i--)
        doCloseTab(i);
}

bool PythonEditorView::isScriptModified()
{
    return txtEditor->document()->isModified();
}

void PythonEditorView::printHeading(const QString &message)
{
    m_console->consoleMessage(QString("%1\n").arg(message), Qt::darkGray);
}

void PythonEditorView::printMessage(const QString &module, const QString &message)
{
    m_console->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::gray);
}

void PythonEditorView::printError(const QString &module, const QString &message)
{
    m_console->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::red);
}

void PythonEditorView::printWarning(const QString &module, const QString &message)
{
    m_console->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::green);
}

void PythonEditorView::printDebug(const QString &module, const QString &message)
{
#ifndef QT_NO_DEBUG_OUTPUT
    m_console->consoleMessage(QString("%1: %2\n").arg(module).arg(message), Qt::lightGray);
#endif
}

// ********************************************************************************

ScriptEditor::ScriptEditor(PythonEngine *pythonEngine, QWidget *parent)
    : PlainTextEditParenthesis(parent), pythonEngine(pythonEngine), m_isProfiled(false), m_isLineNumbersVisible(true)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

    setFont(FONT);
    setTabStopWidth(fontMetrics().width(TABS));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    // highlighter
    new PythonHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth();
    highlightCurrentLine();

    completer = new PythonCompleter();
    completer->setWidget(this);
    connect(completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
}

ScriptEditor::~ScriptEditor()
{
    delete completer;
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void ScriptEditor::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();
    int oldPos = cursor.position();
    int indent = firstNonSpace(cursor.block().text());

    if (completer && completer->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:

            event->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // insert 4 spaces instead of tab
            textCursor().insertText(QString(4, ' '));
        }
        else
        {
            // indent the selection
            indentSelection();
        }
    }
    else if (event->key() == Qt::Key_Backtab && (event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // moves position backward 4 spaces
            QTextCursor cursor = textCursor();
            cursor.setPosition(cursor.position() - 4, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
        }
        else
        {
            // unindent the selection
            unindentSelection();
        }
    }
    else if ((event->key() == Qt::Key_Backspace) && (document()->characterAt(oldPos - 1) == ' ')
             && (document()->characterAt(oldPos - 2) == ' ')
             && (document()->characterAt(oldPos - 3) == ' ')
             && (document()->characterAt(oldPos - 4) == ' '))
    {
        cursor.beginEditBlock();
        // determine selection to delete
        int newPos = oldPos - 4;
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        int startPosOfLine = cursor.position();
        if (newPos < startPosOfLine)
            newPos = startPosOfLine;
        // make selection
        cursor.setPosition(oldPos, QTextCursor::MoveAnchor);
        cursor.setPosition(newPos, QTextCursor::KeepAnchor);
        cursor.deleteChar();
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else if ((event->key() == Qt::Key_Return) && (indent))
    {
        cursor.beginEditBlock();

        // add 1 extra indent if current line begins a code block
        bool inCodeBlock = false;
        if (QRegExp("\\:").indexIn(cursor.block().text()) != -1)
        {
            indent += TABS_SIZE;
            inCodeBlock = true;
        }

        cursor.insertBlock();
        QString spaces(indent, true ? QLatin1Char(' ') : QLatin1Char('\t'));
        cursor.insertText(spaces);

        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }

    if ((event->key() == Qt::Key_Space && event->modifiers() & Qt::ControlModifier)
            || completer->popup()->isVisible())
    {
        QTextCursor tc = textCursor();
        QStringList found = pythonEngine->codeCompletionScript(toPlainText(),
                                                               tc.blockNumber() + 1,
                                                               tc.columnNumber() + 1);

        QString textToComplete = textCursor().block().text().trimmed();
        QStringList foundInterpreter = pythonEngine->codeCompletionInterpreter(textToComplete);

        found << foundInterpreter;
        found.removeDuplicates();

        if (!found.isEmpty())
        {
            // completer->setCompletionPrefix(textToComplete);
            completer->setModel(new QStringListModel(found, completer));
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::StartOfWord);
            QRect cr = cursorRect(c);
            cr.setWidth(completer->popup()->sizeHintForColumn(0)
                        + completer->popup()->verticalScrollBar()->sizeHint().width() + 30);
            cr.translate(lineNumberAreaWidth(), 4);
            completer->complete(cr);
        }
        else
        {
            completer->popup()->hide();
        }
    }
}

void ScriptEditor::indentSelection()
{
    indentAndUnindentSelection(true);
}

void ScriptEditor::unindentSelection()
{
    indentAndUnindentSelection(false);
}

void ScriptEditor::indentAndUnindentSelection(bool doIndent)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // indent or unindent the selected lines
    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    QTextDocument *doc = document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end-1).next();

    for (QTextBlock block = startBlock; block != endBlock; block = block.next())
    {
        QString text = block.text();
        if (doIndent)
        {
            int indentPosition = firstNonSpace(text);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(QString(TABS_SIZE, ' '));
        }
        else
        {
            int indentPosition = firstNonSpace(text);
            int targetColumn = indentedColumn(columnAt(text, indentPosition), false);
            cursor.setPosition(block.position() + indentPosition);
            cursor.setPosition(block.position() + targetColumn, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
    }

    // reselect the selected lines
    cursor.setPosition(startBlock.position());
    cursor.setPosition(endBlock.previous().position(), QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::commentAndUncommentSelection()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // previous selection state
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    int blockEnd = cursor.blockNumber();

    // extend selStart to first blocks's start-of-block
    // extend selEnd to last block's end-of-block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    selStart = cursor.position();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    selEnd = cursor.position();

    // process first block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    QRegExp commentPattern("^#");
    if (commentPattern.indexIn(cursor.block().text()) == -1)
    {
        // comment it, if the block does not starts with '#'
        cursor.insertText("#");
        selEnd += 1;
    }
    else
    {
        // else uncomment it
        cursor.setPosition(selStart + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
        cursor.deleteChar();
        selEnd -= 1;
    }

    // loop through all blocks
    while (cursor.blockNumber() < blockEnd)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        if (commentPattern.indexIn(cursor.block().text()) == -1)
        {
            cursor.insertText("#");
            selEnd += 1;
        }
        else
        {
            cursor.setPosition(cursor.position() + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
            cursor.deleteChar();
            selEnd -= 1;
        }
    }

    // restore selection state
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.setPosition(selEnd, QTextCursor::KeepAnchor);

    // update
    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::gotoLine(int line, bool isError)
{
    // use dialog when (line == -1)
    if (line == -1)
    {
        bool ok;
        int lineDialog = QInputDialog::getInt(this, tr("Goto line"), tr("Line number:"),
                                              0, 1, document()->blockCount(), 1, &ok);
        if (ok)
            line = lineDialog;
    }

    if (line >= 0 && line <= document()->blockCount())
    {
        int pos = document()->findBlockByNumber(line - 1).position();
        QTextCursor cur = textCursor();
        cur.setPosition(pos, QTextCursor::MoveAnchor);
        setTextCursor(cur);
        ensureCursorVisible();
        highlightCurrentLine(true);
        setFocus();
    }
}

void ScriptEditor::highlightCurrentLine(bool isError)
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(253, 235, 227);
        if (isError)
            lineColor = QColor(Qt::red).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);

    matchParentheses('(', ')');
    // matchParentheses('[', ']');
    // matchParentheses('{', '}');
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // line numbers
    const QBrush bookmarkBrushPyFlakes(QColor(Qt::red).lighter());
    const QBrush bookmarkBrushError(QColor(Qt::red).rgb());

    int timesWidth = 0;
    int callWidth = 0;
    if (isProfiled())
    {
        timesWidth = fontMetrics().width(QLatin1Char('9')) * QString::number(profilerMaxAccumulatedTime()).length() + 1;
        callWidth = fontMetrics().width(QLatin1Char('9')) * (QString::number(profilerMaxAccumulatedCall()).length() + 1) + 1;
    }

    QPainter painterLineArea(lineNumberArea);
    painterLineArea.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            // line number
            QString lineNumber = QString::number(blockNumber + 1);

            // draw rect - pyFlakes
            if (errorMessagesPyFlakes.contains(blockNumber + 1))
                painterLineArea.fillRect(0, top, lineNumberArea->width(), fontMetrics().height(),
                                         bookmarkBrushPyFlakes);

            // draw rect - error
            if (errorMessagesError.contains(blockNumber + 1))
                painterLineArea.fillRect(0, top, lineNumberArea->width(), fontMetrics().height(),
                                         bookmarkBrushError);

            // draw line number
            painterLineArea.setPen(Qt::black);
            painterLineArea.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                                     Qt::AlignRight, lineNumber);

            // draw profiler number
            if (isProfiled())
            {
                if (profilerAccumulatedTimes().value(blockNumber + 1) > 0)
                {
                    QString number = QString::number(profilerAccumulatedTimes().value(blockNumber + 1));
                    painterLineArea.setPen(Qt::darkBlue);
                    painterLineArea.drawText(0, top, timesWidth,
                                             fontMetrics().height(),
                                             Qt::AlignRight, number);

                    number = QString::number(profilerAccumulatedLines().value(blockNumber + 1)) + "x";
                    painterLineArea.setPen(Qt::darkGreen);
                    painterLineArea.drawText(0, top, timesWidth + callWidth + 3, fontMetrics().height(),
                                             Qt::AlignRight, number);
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ScriptEditor::lineNumberAreaMouseMoveEvent(QMouseEvent *event)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();

    int line = blockNumber + event->pos().y() / (int) blockBoundingRect(block).height() + 1;

    if (line <= document()->blockCount())
    {
        if (errorMessagesPyFlakes.contains(line))
            QToolTip::showText(event->globalPos(), errorMessagesPyFlakes[line]);

        if (errorMessagesError.contains(line))
            QToolTip::showText(event->globalPos(), errorMessagesError[line]);
    }
}

int ScriptEditor::lineNumberAreaWidth()
{
    if (m_isLineNumbersVisible)
    {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        if (isProfiled())
        {
            digits += QString::number(profilerMaxAccumulatedTime()).length() +
                    QString::number(profilerMaxAccumulatedCall()).length();
        }

        int space = 15 + fontMetrics().width(QLatin1Char('9')) * digits;

        return space;
    }
    else
    {
        return 0;
    }
}

void ScriptEditor::insertCompletion(const QString& completion)
{
    QString str = completion.left(completion.indexOf("(") - 1);

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    // if (tc.selectedText() == ".")
    // {
    //     tc.insertText(QString(".") + str);
    // }
    // else
    {
        tc = textCursor();
        tc.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        tc.insertText(str);
        setTextCursor(tc);
    }
}

void ScriptEditor::replaceTabsWithSpaces()
{
    QString text = document()->toPlainText();
    text = text.replace("\t", TABS);
    document()->setPlainText(text);
}

// ********************************************************************************************************

SearchWidget::SearchWidget(ScriptEditor *txtEditor, QWidget *parent)
    : QWidget(parent), txtEditor(txtEditor)
{
    lblFind = new QLabel(tr("Search for:"));
    lblReplace = new QLabel(tr("Replace with:"));

    txtFind = new QLineEdit();
    connect(txtFind, SIGNAL(returnPressed()), this, SLOT(findNext()));
    txtReplace = new QLineEdit();
    connect(txtReplace, SIGNAL(returnPressed()), this, SLOT(replaceAll()));

    btnFind = new QPushButton(tr("Find"), this);
    btnFind->setDefault(true);
    connect(btnFind, SIGNAL(clicked()), this, SLOT(findNext()));

    btnReplace = new QPushButton(tr("Replace all"), this);
    connect(btnReplace, SIGNAL(clicked()), this, SLOT(replaceAll()));

    btnHide = new QPushButton(tr("Hide"), this);
    connect(btnHide, SIGNAL(clicked()), this, SLOT(hideWidget()));

    QGridLayout *findReplaceLayout = new QGridLayout();
    findReplaceLayout->setMargin(2);

    findReplaceLayout->addWidget(lblFind, 0, 0);
    findReplaceLayout->addWidget(txtFind, 0, 1);
    findReplaceLayout->addWidget(btnFind, 0, 2);
    findReplaceLayout->addWidget(btnHide, 0, 3);
    findReplaceLayout->addWidget(lblReplace, 1, 0);
    findReplaceLayout->addWidget(txtReplace, 1, 1);
    findReplaceLayout->addWidget(btnReplace, 1, 2);

    setLayout(findReplaceLayout);

    lblReplace->setVisible(false);
    txtReplace->setVisible(false);
    btnReplace->setVisible(false);

    setVisible(false);
}

void SearchWidget::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    switch (key)
    {
    case Qt::Key_Escape:
        hideWidget();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void SearchWidget::showFind(const QString &text)
{
    if (!text.isEmpty())
        txtFind->setText(text);

    txtFind->setFocus();
    txtFind->selectAll();
    lblReplace->setVisible(false);
    txtReplace->setVisible(false);
    btnReplace->setVisible(false);

    show();
}

void SearchWidget::showReplaceAll(const QString &text)
{
    if (!text.isEmpty())
        txtFind->setText(text);

    txtFind->setFocus();
    txtFind->selectAll();
    lblReplace->setVisible(true);
    txtReplace->setVisible(true);
    btnReplace->setVisible(true);

    show();
}

void SearchWidget::findNext()
{
    if (!txtFind->text().isEmpty())
    {
        // search
        QTextCursor cursor = txtEditor->document()->find(txtFind->text(), txtEditor->textCursor());

        if (cursor.isNull())
        {
            // restart from the beginning
            cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            if (!txtEditor->document()->find(txtFind->text(), cursor).isNull())
            {
                txtEditor->setTextCursor(cursor);
                findNext();
            }
        }
        else
        {
            txtEditor->setTextCursor(cursor);
            txtEditor->setFocus();
        }

        if (isVisible())
            txtFind->setFocus();
    }
}

void SearchWidget::replaceAll()
{
    if (!txtFind->text().isEmpty())
    {
        QTextCursor cursor = txtEditor->textCursor();

        QString text = txtEditor->document()->toPlainText();
        text.replace(txtFind->text(), txtReplace->text());
        txtEditor->document()->setPlainText(text);

        txtEditor->setTextCursor(cursor);

        hideWidget();
    }
}

void SearchWidget::hideWidget()
{
    hide();
    txtEditor->setFocus();
}

// *********************************************************************************************

ErrorWidget::ErrorWidget(QTabWidget *tabWidget, QWidget *parent)
    : QWidget(parent), tabWidget(tabWidget)
{
    dialog = dynamic_cast<PythonEditorView *>(parent);

    setVisible(false);

    trvErrors = new QTreeWidget(this);
    trvErrors->setHeaderHidden(false);
    trvErrors->setMouseTracking(true);
    trvErrors->setColumnCount(3);
    trvErrors->setColumnWidth(1, 200);
    connect(trvErrors, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doHighlightLineError(QTreeWidgetItem *, int)));

    QStringList headers;
    headers << tr("Line") << tr("Name") << tr("Filename");
    trvErrors->setHeaderLabels(headers);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    errorLabel = new QLabel(this);
    errorLabel->setPalette(palette);

    QVBoxLayout *layoutError = new QVBoxLayout();
    layoutError->addWidget(errorLabel, 0);
    layoutError->addWidget(trvErrors, 1);

    setMaximumHeight(150);
    setMinimumHeight(150);

    setLayout(layoutError);
}

void ErrorWidget::showError(ErrorResult result)
{
    trvErrors->clear();
    setVisible(true);

    PythonEditorTextEdit *scriptEditorWidget = dynamic_cast<PythonEditorTextEdit *>(tabWidget->currentWidget());

    errorLabel->setText(result.error());

    foreach (ErrorResult::ErrorTraceback trace, result.traceback())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvErrors);

        item->setIcon(0, icon("check-error"));
        item->setText(0, QString::number(trace.line));
        item->setData(0, Qt::UserRole, trace.line);
        item->setText(1, trace.name);
        item->setData(1, Qt::UserRole, trace.name);
        item->setText(2, trace.fileName);
        item->setData(2, Qt::UserRole, trace.fileName);

        // lines
        int line = item->data(0, Qt::UserRole).value<int>();
        QString name = item->data(1, Qt::UserRole).toString();
        QString fileName = item->data(2, Qt::UserRole).toString();

        if (scriptEditorWidget->fileName() == fileName)
        {
            scriptEditorWidget->txtEditor->errorMessagesError.insert(line, name);
        }
    }
}

void ErrorWidget::doHighlightLineError(QTreeWidgetItem *item, int role)
{
    if (item)
    {
        int line = item->data(0, Qt::UserRole).value<int>();
        QString fileName = item->data(2, Qt::UserRole).toString();

        PythonEditorTextEdit *scriptEditorWidget = NULL;
        for (int i = 0; i < tabWidget->count(); i++)
        {
            scriptEditorWidget = dynamic_cast<PythonEditorTextEdit *>(tabWidget->widget(i));

            if (scriptEditorWidget->fileName() == fileName)
            {
                tabWidget->setCurrentIndex(i);
                break;
            }
            else
            {
                scriptEditorWidget = NULL;
            }
        }

        // open existing file
        if (!scriptEditorWidget)
        {
            if (QFile::exists(fileName))
            {
                dialog->doFileOpen(fileName);
                scriptEditorWidget = dynamic_cast<PythonEditorTextEdit *>(tabWidget->widget(tabWidget->count() - 1));
            }
        }

        if (scriptEditorWidget)
            scriptEditorWidget->txtEditor->gotoLine(line, true);
    }
}