// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "optilab.h"

#include "util/constants.h"
#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/systemoutput.h"

#include "ctemplate/template.h"

#include <fstream>
#include <string>

#include <QXmlSimpleReader>

OptilabWindow::OptilabWindow() : QMainWindow()
{
    setWindowTitle(tr("Optilab"));

    m_fileName = "/home/karban/Projects/agros2d/data/sweep/actuator/problem.xml";

    createPythonEngine(new PythonEngineOptilab());
    scriptEditorDialog = new PythonEditorOptilabDialog(currentPythonEngine(), QStringList(), NULL);

    createActions();
    createToolBars();
    createMenus();
    createMain();

    QSettings settings;
    restoreGeometry(settings.value("OptilabWindow/Geometry", saveGeometry()).toByteArray());

    readVariants();

    /*
    try
    {
        string modelFile = "capacitor.py";
        string model = "Capacitor";

        XMLOptVariant::input input = XMLOptVariant::input();
        XMLOptVariant::output output = XMLOptVariant::output();

        input.parameter().push_back(XMLOptVariant::parameter("w", 0.02));
        input.parameter().push_back(XMLOptVariant::parameter("h", 0.05));

        XMLOptVariant::solution solution = XMLOptVariant::solution(0);

        XMLOptVariant::variant variant = XMLOptVariant::variant(input, output, solution, modelFile, model);

        std::string mesh_schema_location("");

        mesh_schema_location.append(QString("%1/opt_variant_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_mesh("XMLOptVariant", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("variant", namespace_info_mesh));

        QString file = "/home/karban/Projects/agros2d/data/sweep/pokus.rst";

        std::ofstream out(compatibleFilename(file).toStdString().c_str());
        XMLOptVariant::variant_(out, variant, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
    */
}

OptilabWindow::~OptilabWindow()
{
    QSettings settings;
    settings.setValue("OptilabWindow/Geometry", saveGeometry());

    removeDirectory(tempProblemDir());

    delete scriptEditorDialog;
}

void OptilabWindow::openInAgros2D()
{    
    XMLOptVariant::result *result = variantResult(lstProblems->currentItem()->data(0, Qt::UserRole).toInt());

    if (result)
    {
        // TODO: template?
        QString str;
        str += "from variant import model\n";
        str += QString("import sys; sys.path.insert(0, '%1');\n").arg(QFileInfo(m_fileName).absolutePath());
        str += "import problem\n";
        str += "p = problem.Model()\n";
        // parameters
        for (unsigned int i = 0; i < result->input().parameter().size(); i++)
        {
            XMLOptVariant::parameter parameter = result->input().parameter().at(i);
            str += QString("p.parameters['%1'] = %2\n").arg(QString::fromStdString(parameter.param())).arg(parameter.param_value());
        }
        str += "p.create()\n";

        QString id = QUuid::createUuid().toString().remove("{").remove("}");
        QString tempFN = QString("%1/%2.py").arg(tempProblemDir()).arg(id);

        writeStringContent(tempFN, str);

        // run agros2d
        QProcess *process = new QProcess();
        process->setStandardOutputFile(tempProblemDir() + "/agros2d.out");
        process->setStandardErrorFile(tempProblemDir() + "/agros2d.err");
        connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processOpenError(QProcess::ProcessError)));
        connect(process, SIGNAL(finished(int)), this, SLOT(processOpenFinished(int)));

        QString command = QString("\"%1/agros2d\" -s \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(tempFN);

        process->start(command);
    }
}

void OptilabWindow::processOpenError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D");
}

void OptilabWindow::processOpenFinished(int exitCode)
{
    if (exitCode == 0)
    {
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

void OptilabWindow::solveInSolver()
{
    int index = lstProblems->currentItem()->data(0, Qt::UserRole).toInt();
    XMLOptVariant::result *result = variantResult(index);

    if (result)
    {
        // TODO: template?
        QString str;
        str += "from variant import model\n";
        str += QString("import sys; sys.path.insert(0, '%1')\n").arg(QFileInfo(m_fileName).absolutePath());
        str += "import problem\n";
        str += "p = problem.Model()\n";
        // parameters
        for (unsigned int i = 0; i < result->input().parameter().size(); i++)
        {
            XMLOptVariant::parameter parameter = result->input().parameter().at(i);
            str += QString("p.parameters['%1'] = %2\n").arg(QString::fromStdString(parameter.param())).arg(parameter.param_value());
        }
        str += "p.create()\n";
        str += "p.solve()\n";
        str += "p.process()\n";
        str += QString("models = model.ModelDict(); models.load('%1'); models.models[%2] = p; models.save('%1'); ").arg(QFileInfo(m_fileName).absoluteFilePath()).arg(index);

        QString command = QString("\"%1/agros2d_solver\" -l -c \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(str);

        SystemOutputWidget *systemOutput = new SystemOutputWidget(this);
        connect(systemOutput, SIGNAL(finished(int)), this, SLOT(processSolveFinished(int)));
        systemOutput->execute(command);
    }
}

void OptilabWindow::processSolveError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D Solver");
}

void OptilabWindow::processSolveFinished(int exitCode)
{
    if (exitCode == 0)
    {
        QApplication::processEvents();
        readVariants();
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

void OptilabWindow::createActions()
{
    actAbout = new QAction(icon("about"), tr("About &Optilab"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    // connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actScriptEditor = new QAction(icon("script-python"), tr("PythonLab"), this);
    actScriptEditor->setShortcut(Qt::Key_F9);
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(doScriptEditor()));

    actReadVariants = new QAction(icon("edit-find"), tr("Refresh"), this);
    connect(actReadVariants, SIGNAL(triggered()), this, SLOT(readVariants()));

    actOpenInAgros2D = new QAction(icon("agros2d"), tr("Open in Agros2D"), this);
    actOpenInAgros2D->setEnabled(false);
    connect(actOpenInAgros2D, SIGNAL(triggered()), this, SLOT(openInAgros2D()));

    actSolverInSolver = new QAction(icon("run"), tr("Solve"), this);
    actSolverInSolver->setEnabled(false);
    connect(actSolverInSolver, SIGNAL(triggered()), this, SLOT(solveInSolver()));
}

void OptilabWindow::createMenus()
{
    menuBar()->clear();

    QMenu *mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addSeparator();
#ifndef Q_WS_MAC
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);
#endif

    QMenu *mnuHelp = menuBar()->addMenu(tr("&Help"));
#ifndef Q_WS_MAC
    mnuHelp->addSeparator();
#else
    // mnuHelp->addAction(actOptions); // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actExit);    // will be added to "Agros2D" MacOSX menu
#endif
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void OptilabWindow::createToolBars()
{
    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    QToolBar *tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->setOrientation(Qt::Horizontal);
    tlbFile->setAllowedAreas(Qt::TopToolBarArea);
    tlbFile->setMovable(false);
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbFile->addAction(actReadVariants);

    QToolBar *tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->setOrientation(Qt::Horizontal);
    tlbTools->setAllowedAreas(Qt::TopToolBarArea);
    tlbTools->setMovable(false);
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->addAction(actOpenInAgros2D);
    tlbTools->addAction(actSolverInSolver);
    tlbTools->addSeparator();
    tlbTools->addAction(actScriptEditor);
}

void OptilabWindow::createMain()
{
    console = new PythonScriptingConsole(currentPythonEngineOptilab(), this);

    chart = new QCustomPlot(this);
    chart->setMinimumHeight(300);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    chart->addGraph();
    // chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    // chart->graph(0)->setPen(QColor(50, 50, 50, 255));
    chart->graph(0)->setLineStyle(QCPGraph::lsNone);
    chart->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
    connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, QMouseEvent*)));

    cmbX = new QComboBox(this);
    cmbY = new QComboBox(this);
    QPushButton *btnPlot = new QPushButton(tr("Plot"), this);
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(setChart()));

    QGridLayout *layoutChart = new QGridLayout();
    layoutChart->addWidget(new QLabel(tr("X:")), 0, 0);
    layoutChart->addWidget(cmbX, 0, 1);
    layoutChart->addWidget(new QLabel(tr("Y:")), 1, 0);
    layoutChart->addWidget(cmbY, 1, 1);
    layoutChart->addWidget(chart, 3, 0, 1, 2);
    layoutChart->addWidget(btnPlot, 4, 1);

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    lstProblems = new QTreeWidget(this);
    lstProblems->setMouseTracking(true);
    lstProblems->setColumnCount(2);
    lstProblems->setIndentation(15);
    lstProblems->setIconSize(QSize(16, 16));
    lstProblems->setHeaderHidden(true);
    lstProblems->setMinimumWidth(320);
    lstProblems->setColumnWidth(0, 150);

    connect(lstProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    lblProblems = new QLabel("");

    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(lstProblems);
    layoutLeft->addWidget(lblProblems);

    QVBoxLayout *layoutChartConsole = new QVBoxLayout();
    layoutChartConsole->addWidget(console, 0.3);
    layoutChartConsole->addLayout(layoutChart);

    QHBoxLayout *layoutRight = new QHBoxLayout();
    layoutRight->addWidget(webView);
    layoutRight->addLayout(layoutChartConsole);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addLayout(layoutLeft);
    layout->addLayout(layoutRight, 1);

    QWidget *main = new QWidget();
    main->setLayout(layout);

    setCentralWidget(main);
}

void OptilabWindow::doScriptEditor()
{
    scriptEditorDialog->showDialog();
}

void OptilabWindow::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    webView->setHtml("");

    actOpenInAgros2D->setEnabled(false);
    actSolverInSolver->setEnabled(false);

    if (current)
    {
        variantInfo(current->data(0, Qt::UserRole).toInt());
    }
}

void OptilabWindow::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item)
        openInAgros2D();
}

void OptilabWindow::linkClicked(const QUrl &url)
{
    QString search = "/open?";
    if (url.toString().contains(search))
    {
#if QT_VERSION < 0x050000
        QString fileName = url.queryItemValue("filename");
        QString form = url.queryItemValue("form");
#else
        QString fileName = QUrlQuery(url).queryItemValue("filename");
        QString form = QUrlQuery(url).queryItemValue("form");
#endif

        // m_selectedFilename = QUrl(fileName).toLocalFile();

        // accept();
    }
}

void OptilabWindow::readVariants()
{
    // save current item
    int selectedItem;
    if (lstProblems->currentItem())
        selectedItem = lstProblems->currentItem()->data(0, Qt::UserRole).toInt();

    // QTime time;
    // time.start();

    // clear listview
    lstProblems->clear();
    lstProblems->setUpdatesEnabled(false);
    // clear cache
    outputVariables.clear();

    int count = 0;
    int countSolved = 0;

    try
    {
        variant_xsd = XMLOptVariant::variant_(compatibleFilename(m_fileName).toStdString(), xml_schema::flags::dont_validate);
        XMLOptVariant::variant *var = variant_xsd.get();

        for (unsigned int i = 0; i < var->results().result().size(); i++)
        {
            XMLOptVariant::result result = var->results().result().at(i);

            QTreeWidgetItem *variantItem = new QTreeWidgetItem(lstProblems->invisibleRootItem());
            if (result.solution().solved() == 1)
                variantItem->setIcon(0, icon("browser-other"));
            else
                variantItem->setIcon(0, icon("browser-class"));
            variantItem->setText(0, QString::number(i));
            // variantItem->setText(1, fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            variantItem->setData(0, Qt::UserRole, i);

            if (result.solution().solved())
            {
                QList<OutputVariable> variables;

                for (unsigned int j = 0; j < result.output().variable().size(); j++)
                {
                    XMLOptVariant::variable var = result.output().variable().at(j);
                    variables.append(OutputVariable(QString::fromStdString(var.var()),
                                                    QString::fromStdString(var.var_value())));
                }

                outputVariables.append(i, variables);
            }

            if (i == selectedItem)
            {
                variantItem->setSelected(true);
                lstProblems->setCurrentItem(variantItem);
            }

            // increase counter
            count++;
            if (result.solution().solved() == 1)
                countSolved++;
        }
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }


    lstProblems->setUpdatesEnabled(true);
    // qDebug() << time.elapsed();

    lblProblems->setText(tr("Solutions: %1/%2").arg(countSolved).arg(count));

    // set Python variables
    setPythonVariables();

    // plot chart
    setChart();
}

void OptilabWindow::setPythonVariables()
{
    XMLOptVariant::variant *variant = variant_xsd.get();

    if (variant)
    {
        for (int i = 0; i < variant->results().result().size(); i++)
        {
            XMLOptVariant::result result = variant->results().result().at(i);

            for (int j = 0; j < result.output().variable().size(); j++)
            {
                XMLOptVariant::variable var = result.output().variable().at(j);

                QVector<double> values = outputVariables.values(QString::fromStdString(var.var()));

                QString lst;
                lst = "[";
                for (int j = 0; j < values.size(); j++)
                    lst += QString::number(values[j]) + ", ";
                lst += "]";

                QString str = QString("%1 = %2").
                        arg(QString::fromStdString(var.var())).
                        arg(lst);

                currentPythonEngineOptilab()->runExpression(str);
            }
        }
    }
}

void OptilabWindow::setChart()
{
    XMLOptVariant::variant *var = variant_xsd.get();

    if (var)
    {
        QString selectedX = cmbX->currentText();
        QString selectedY = cmbY->currentText();

        cmbX->clear();
        cmbY->clear();

        if (var->results().result().size() > 0)
        {
            for (unsigned int i = 0; i < var->results().result().size(); i++)
            {
                XMLOptVariant::result result = var->results().result().at(i);
                if (result.solution().solved() == 1)
                {

                    for (unsigned int j = 0; j < result.output().variable().size(); j++)
                    {
                        XMLOptVariant::variable variable = result.output().variable().at(j);

                        cmbX->addItem(QString::fromStdString(variable.var()));
                        cmbY->addItem(QString::fromStdString(variable.var()));
                    }

                    break;
                }
            }
        }

        if (!selectedX.isEmpty())
            cmbX->setCurrentIndex(cmbX->findText(selectedX));
        if (!selectedY.isEmpty())
            cmbY->setCurrentIndex(cmbY->findText(selectedY));

        chart->xAxis->setLabel(cmbX->currentText());
        chart->yAxis->setLabel(cmbY->currentText());

        chart->graph(0)->setData(outputVariables.values(cmbX->currentText()),
                                 outputVariables.values(cmbY->currentText()));
        chart->rescaleAxes();
        chart->replot();
    }
}

void OptilabWindow::variantInfo(int index)
{
    XMLOptVariant::result *result = variantResult(index);

    if (result)
    {
        // template
        std::string info;
        ctemplate::TemplateDictionary variantInfo("info");

        // problem info
        variantInfo.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

        variantInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
        variantInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());
        variantInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

        variantInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
        variantInfo.SetValue("NAME", QString::number(index).toStdString());

        try
        {
            variantInfo.SetValue("SOLVED", result->solution().solved() == 1 ? "YES" : "NO");

            // input
            variantInfo.SetValue("PARAMETER_LABEL", tr("Input parameters").toStdString());
            for (unsigned int i = 0; i < result->input().parameter().size(); i++)
            {
                ctemplate::TemplateDictionary *paramSection = variantInfo.AddSectionDictionary("PARAM_SECTION");

                XMLOptVariant::parameter parameter = result->input().parameter().at(i);

                paramSection->SetValue("PARAM_LABEL", parameter.param());
                paramSection->SetValue("PARAM_VALUE", QString::number(parameter.param_value()).toStdString());
                // paramSection->SetValue("PARAM_UNIT", parameter.param_unit());
            }

            // output
            variantInfo.SetValue("VARIABLE_LABEL", tr("Output variables").toStdString());
            for (unsigned int i = 0; i < result->output().variable().size(); i++)
            {
                XMLOptVariant::variable variable = result->output().variable().at(i);

                OutputVariable result(QString::fromStdString(variable.var()),
                                      QString::fromStdString(variable.var_value()));

                if (result.isNumber())
                {
                    ctemplate::TemplateDictionary *varSection = variantInfo.AddSectionDictionary("VAR_VALUE_SECTION");

                    // double value
                    varSection->SetValue("VAR_LABEL", result.name().toStdString());
                    varSection->SetValue("VAR_VALUE", QString::number(result.number()).toStdString());
                    // varSection->SetValue("VAR_UNIT", variable.var_unit());
                }
                else
                {
                    ctemplate::TemplateDictionary *varSection = variantInfo.AddSectionDictionary("VAR_CHART_SECTION");

                    QString chartData = "[";
                    for (int j = 0; j < result.size(); j++)
                        chartData += QString("[%1, %2], ").arg(result.x().at(j)).arg(result.y().at(j));
                    chartData += "]";

                    // chart time step vs. steps
                    QString chart = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'N' } ], yaxes: [ { axisLabel: '%3' } ] });});</script>").
                            arg(i).
                            arg(chartData).
                            arg(QString::fromStdString(variable.var()));

                    varSection->SetValue("VAR_CHART_DIV", QString("chart_%1").arg(i).toStdString());
                    varSection->SetValue("VAR_CHART", chart.toStdString());
                }
            }
        }
        catch (const xml_schema::exception& e)
        {
            std::cerr << e << std::endl;
        }

        QString templateName = "variant.tpl";
        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/" + templateName.toStdString(), ctemplate::DO_NOT_STRIP, &variantInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/variant.html", QString::fromStdString(info));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/variant.html"));

        actOpenInAgros2D->setEnabled(true);
        actSolverInSolver->setEnabled(true);
    }
}

void OptilabWindow::graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event)
{
    double x = chart->xAxis->pixelToCoord(event->pos().x());
    double y = chart->yAxis->pixelToCoord(event->pos().y());

    int index = -1;
    QMap<double, QCPData>::const_iterator pointX = chart->graph(0)->data()->lowerBound(x);
    for (QMap<double, QCPData>::const_iterator i = chart->graph(0)->data()->constBegin(); i != chart->graph(0)->data()->constEnd(); ++i)
    {
        if (pointX == i)
            break;

        index++;
    }

    if (index != -1)
    {
        // qDebug() << outputVariables.variables().at(index) << x << y << index;
        variantInfo(index);
    }
}

XMLOptVariant::result *OptilabWindow::variantResult(int variant)
{
    XMLOptVariant::variant *var = variant_xsd.get();

    if (var && variant != -1)
        return &var->results().result().at(variant);
    else
        return NULL;
}
