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

#include "gui.h"

#include "scene.h"
#include "util.h"

#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/solver.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_counter.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_renderer.h>
#include "qwt_plot_magnifier.h"
#include "qwt_plot_rescaler.h"

void readPixmap(QLabel *lblEquation, const QString &name)
{
    QPixmap pixmap;
    pixmap.load(name);
    lblEquation->setPixmap(pixmap);
    lblEquation->setMaximumSize(pixmap.size());
    lblEquation->setMinimumSize(pixmap.size());
}

QLabel *createLabel(const QString &label, const QString &toolTip)
{
    QLabel *lblEquation = new QLabel(label + ":");
    lblEquation->setToolTip(toolTip);
    lblEquation->setMinimumWidth(100);
    return lblEquation;
}

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, QString("%1 ").arg(unit));
    item->setTextAlignment(2, Qt::AlignLeft);
}

void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo)
{
    // store variable
    QString fieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    // clear combo
    cmbFieldInfo->blockSignals(true);
    cmbFieldInfo->clear();
    foreach (FieldInfo *fieldInfo, Util::problem()->fieldInfos())
        cmbFieldInfo->addItem(fieldInfo->name(), fieldInfo->fieldId());

    cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldId));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    cmbFieldInfo->blockSignals(false);
}

void fillComboBoxScalarVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxScalarVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxContourVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxContourVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxVectorVariable(FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    fieldInfo->module()->fillComboBoxVectorVariable(cmbFieldVariable);

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxTimeStep(FieldInfo* fieldInfo, QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    cmbFieldVariable->blockSignals(true);

    // store variable
    int timeStep = cmbFieldVariable->currentIndex();
    double timeValue;
    if (timeStep == -1){
        timeStep = Util::solutionStore()->lastTimeStep(fieldInfo, SolutionMode_Normal);
        timeValue = Util::problem()->config()->timeStepToTime(timeStep);
    }
    else
    {
        timeValue = cmbFieldVariable->currentText().toDouble();
    }

    // clear combo
    cmbFieldVariable->clear();

    QList<double> timeLevels = Util::solutionStore()->timeLevels(fieldInfo);
    int i = 0;
    timeStep = 0;
    foreach(double time, timeLevels)
    {
        cmbFieldVariable->addItem(QString::number(time, 'e', 2), i++);
        if(time < timeValue)
            timeStep = i;
    }

    cmbFieldVariable->setCurrentIndex(timeStep);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxAdaptivityStep(QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    cmbFieldVariable->blockSignals(true);

    int lastAdaptiveStep = Util::solutionStore()->lastAdaptiveStep(Util::scene()->activeViewField(), SolutionMode_Normal, Util::scene()->activeTimeStep());

    // store variable
    int adaptivityStep = cmbFieldVariable->currentIndex();
    if (adaptivityStep == -1)
        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbFieldVariable->clear();

    for (int step = 0; step <= lastAdaptiveStep; step++)
    {
        cmbFieldVariable->addItem(QString::number(step + 1), step);
    }

    cmbFieldVariable->setCurrentIndex(adaptivityStep);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxSolutionType(QComboBox *cmbFieldVariable)
{
    if (!Util::problem()->isSolved())
        return;

    cmbFieldVariable->blockSignals(true);

    // store variable
    SolutionMode solutionType = (SolutionMode) cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toInt();
    //    if (adaptivityStep == -1)
    //        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbFieldVariable->clear();

    int timeStep = Util::scene()->activeTimeStep();
    int adaptivityStep = Util::scene()->activeAdaptivityStep();

    //TODO je potreba udelat dynamicky, jak se vybira casovy a adaptivni krok ve formulari
    if(Util::solutionStore()->contains(FieldSolutionID(Util::scene()->activeViewField(), timeStep, adaptivityStep, SolutionMode_Normal)))
    {
        cmbFieldVariable->addItem(/*tr*/(solutionTypeToStringKey(SolutionMode_Normal)), SolutionMode_Normal);
    }
    if(Util::solutionStore()->contains(FieldSolutionID(Util::scene()->activeViewField(), timeStep, adaptivityStep, SolutionMode_Reference)))
    {
        cmbFieldVariable->addItem(/*tr*/(solutionTypeToStringKey(SolutionMode_Reference)), SolutionMode_Reference);
    }

    //    for(int step = 0; step <= lastAdaptiveStep; step++)
    //    {
    //        cmbFieldVariable->addItem(QString::number(step), step);
    //    }

    cmbFieldVariable->setCurrentIndex(solutionType);
    cmbFieldVariable->blockSignals(false);
}

// ***********************************************************************************************************

Chart::Chart(QWidget *parent, bool showPicker) : QwtPlot(parent)
{
    //  chart style
    setAutoReplot(false);
    setTitle("");
    setMinimumSize(420, 260);

    // panning with the left mouse button
    // QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    // panner->setMouseButton(Qt::MidButton);

    // zoom in/out with the wheel
    // QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
    // magnifier->setWheelFactor(1 / magnifier->wheelFactor());
    // magnifier->setMouseFactor(1 / magnifier->mouseFactor());

    // QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
    // zoomer->setRubberBandPen(QColor(Qt::red));
    // zoomer->setTrackerPen(Qt::NoPen);
    // zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    // zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    // legend
    /*
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);
    */

    // canvas
    QPalette canvasPalette(Qt::white);
    canvasPalette.setColor(QPalette::Foreground, QColor(0, 0, 0));
    canvas()->setPalette(canvasPalette);
    canvas()->setFrameShape(QFrame::NoFrame);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::NoPen));
    grid->enableX(true);
    grid->enableY(true);
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->attach(this);

    // axes
    QFont fnt = QFont(QApplication::font().family(), QApplication::font().pointSize() - 1, QFont::Normal);
    setAxisFont(QwtPlot::xBottom, fnt);
    setAxisTitle(QwtPlot::xBottom, QwtText(" "));
    setAxisFont(QwtPlot::yLeft, fnt);
    setAxisTitle(QwtPlot::yLeft, QwtText(" "));

    // curve styles
    QwtSymbol sym;

    sym.setStyle(QwtSymbol::Cross);
    sym.setPen(QColor(Qt::black));
    sym.setSize(5);

    // curve
    m_curve = new QwtPlotCurve();
    m_curve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    m_curve->setPen(QPen(Qt::darkBlue, 1.5));
    m_curve->setCurveAttribute(QwtPlotCurve::Inverted);
    m_curve->setYAxis(QwtPlot::yLeft);
    m_curve->attach(this);


    // chart picker
    QwtPlotPicker *pickerValue = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                                   QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                                   canvas());
    pickerValue->setRubberBandPen(QColor(Qt::green));
    pickerValue->setRubberBand(QwtPicker::CrossRubberBand);
    pickerValue->setTrackerMode(QwtPicker::AlwaysOn);
    pickerValue->setTrackerPen(QColor(Qt::black));

    connect(pickerValue, SIGNAL(moved(const QPoint &)), SLOT(pickerValueMoved(const QPoint &)));

    setMinimumSize(sizeHint());
}

Chart::~Chart()
{
    delete m_curve;
}

void Chart::saveImage(const QString &fileName)
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileNameTemp;
    if (fileName.isEmpty())
    {
        const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();

        QStringList filter;
        filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
        filter += "SVG Documents (*.svg)";
#endif
        filter += "Postscript Documents (*.ps)";

        if (imageFormats.size() > 0)
        {
            QString imageFilter("Images (");
            for ( int i = 0; i < imageFormats.size(); i++ )
            {
                if ( i > 0 )
                    imageFilter += " ";
                imageFilter += "*.";
                imageFilter += imageFormats[i];
            }
            imageFilter += ")";

            filter += imageFilter;
        }
        fileNameTemp = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, filter.join(";;"));

        QFileInfo fileInfo(fileNameTemp);
        if (!fileNameTemp.isEmpty() && fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
    else
    {
        fileNameTemp = fileName;
    }

    if (!fileNameTemp.isEmpty())
    {
        // QFileInfo fileInfo(fileNameTemp);
        // if (fileInfo.suffix().toLower() != "png") fileNameTemp += ".png";

        QwtPlotRenderer renderer;

        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);

        renderer.renderDocument(this, fileNameTemp, QSizeF(300, 200), 85);
    }
}

void Chart::setData(double *xval, double *yval, int count)
{
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setSamples(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

void Chart::setData(QList<double> xval, QList<double> yval)
{
    double *txval = new double[xval.count()];
    double *tyval = new double[xval.count()];

    for (int i = 0; i < xval.count(); i++)
    {
        txval[i] = xval[i];
        tyval[i] = yval[i];
    }

    setData(txval, tyval, xval.count());

    delete [] txval;
    delete [] tyval;

    replot();
}

void Chart::pickerValueMoved(const QPoint &pos)
{
    QString info;
    info.sprintf("x=%g, y=%g",
                 invTransform(QwtPlot::xBottom, pos.x()),
                 invTransform(QwtPlot::yLeft, pos.y()));
}

// ****************************************************************************************************

FileBrowser::FileBrowser(QWidget *parent) : QListWidget(parent)
{
    setNameFilter("");
    setDir(QDir::currentPath());
    setContextMenuPolicy(Qt::CustomContextMenu);

    actCreateDirectory = new QAction(icon("file-folder"), tr("&Create directory"), this);
    connect(actCreateDirectory, SIGNAL(triggered()), this, SLOT(createDir()));

    actCreateFile = new QAction(icon("file-text"), tr("Create &file"), this);
    connect(actCreateFile, SIGNAL(triggered()), this, SLOT(createFile()));

    actRename = new QAction(icon(""), tr("&Rename"), this);
    connect(actRename, SIGNAL(triggered()), this, SLOT(renameObject()));

    actDelete = new QAction(icon(""), tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteObject()));

    mnuContext = new QMenu(this);
    mnuContext->addAction(actCreateDirectory);
    mnuContext->addAction(actCreateFile);
    mnuContext->addSeparator();
    mnuContext->addAction(actRename);
    mnuContext->addAction(actDelete);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(doFileItemActivated(QListWidgetItem *)));
    connect(this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(doFileItemActivated(QListWidgetItem *)));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doFileItemDoubleClick(QListWidgetItem *)));
}

void FileBrowser::setNameFilter(const QString &nameFilter)
{
    m_nameFilter = nameFilter;
}

QString FileBrowser::basePath()
{
    return QString(m_basePath);
}

void FileBrowser::setDir(const QString &path)
{
    clear();

    QDir dir(path, m_nameFilter, QDir::DirsFirst);
    dir.setFilter(QDir::AllEntries | QDir::AllDirs);

    QStringList entries = dir.entryList();
    foreach(QString item, entries)
    {
        if (dir.isRoot() && (item == "..")) continue;
        if (item != ".")
        {
            QFileInfo fileInfo(path + QDir::separator() + item);
            if (QDir(path + QDir::separator() + item).exists())
            {
                addItem(new QListWidgetItem(icon("file-folder"), item));
            }
            else
            {
                if (fileInfo.suffix() == "py")
                    addItem(new QListWidgetItem(icon("file-python"), item));
                else
                    addItem(new QListWidgetItem(icon("file-text"), item));
            }
        }
    }

    m_basePath = dir.canonicalPath();
    emit directoryChanged(m_basePath);
}

void FileBrowser::refresh()
{
    setDir(m_basePath);
}

void FileBrowser::doFileItemActivated(QListWidgetItem *item)
{

}

void FileBrowser::doFileItemDoubleClick(QListWidgetItem *item)
{


    QString path = m_basePath + QDir::separator() + item->text();
    QDir dir(path);
    if (dir.exists())
    {
        setDir(dir.canonicalPath());
        emit fileItemDoubleClick(dir.canonicalPath());
    }
    else
    {
        emit fileItemDoubleClick(path);
    }
}

void FileBrowser::doContextMenu(const QPoint &point)
{
    actDelete->setEnabled(false);
    actRename->setEnabled(false);

    QListWidgetItem *item = itemAt(point);
    if (item)
    {
        if (item->text() != "..")
        {
            actDelete->setEnabled(true);
            actRename->setEnabled(true);
        }
    }
    mnuContext->exec(QCursor::pos());
}

void FileBrowser::createDir(const QString &dirName)
{
    QString str;
    if (dirName.isEmpty())
        str = QInputDialog::getText(QApplication::activeWindow(), tr("Create directory"), tr("Enter directory name"));
    else
        str = dirName;

    if (str.isEmpty()) return;

    QDir(m_basePath).mkdir(str);
    setDir(m_basePath + QDir::separator() + str);
}

void FileBrowser::createFile(const QString &fileName)
{
    QString str;
    if (fileName.isEmpty())
        str = QInputDialog::getText(QApplication::activeWindow(), tr("Create file"), tr("Enter file name"));
    else
        str = fileName;

    if (str.isEmpty()) return;

    // add extension
    QFileInfo fileInfo(str);
    if (fileInfo.suffix() != "py") str += ".py";

    QFile file(m_basePath + QDir::separator() + str);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        QMessageBox::critical(this, tr("Create file"), file.errorString());
    else
    {
        QTextStream out(&file);
        out << "";
        file.close();

        refresh();
    }
}

void FileBrowser::deleteDir(const QString &dirName)
{
    QDir dir(dirName);
    if (dir.exists())
    {
        dir.rmdir(dirName);
        refresh();
    }
}

void FileBrowser::deleteFile(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFile::remove(fileName);
        refresh();
    }
}

void FileBrowser::deleteObject(const QString &name)
{
    QString str;
    if (name.isEmpty())
        str = m_basePath + QDir::separator() + currentItem()->text();
    else
        str = name;

    QDir dir(str);
    if (dir.exists())
        deleteDir(str);
    else
        deleteFile(str);
}

void FileBrowser::renameDir(const QString &dirName)
{
    QString str = QInputDialog::getText(QApplication::activeWindow(), tr("Rename directory"), tr("Enter directory name"),
                                        QLineEdit::Normal, QDir(dirName).dirName());

    if (str.isEmpty()) return;

    if (!QDir(m_basePath).rename(dirName, str))
        QMessageBox::warning(this, tr("Rename directory"), tr("Directory could not be renamed."));
    else
        refresh();
}

void FileBrowser::renameFile(const QString &fileName)
{
    QString str = QInputDialog::getText(QApplication::activeWindow(), tr("Rename file"), tr("Enter file name"),
                                        QLineEdit::Normal, QFileInfo(fileName).baseName());

    if (str.isEmpty()) return;

    // add extension
    QFileInfo fileInfo(str);
    if (fileInfo.suffix() != "py") str += ".py";

    if (!QFile::rename(fileName, QFileInfo(fileName).absolutePath() + QDir::separator() + str))
        QMessageBox::warning(this, tr("Rename file"), tr("File could not be renamed."));
    else
        refresh();
}

void FileBrowser::renameObject(const QString &name)
{
    QString str;
    if (name.isEmpty())
        str = m_basePath + QDir::separator() + currentItem()->text();
    else
        str = name;

    QDir dir(str);
    if (dir.exists())
        renameDir(str);
    else
        renameFile(str);
}

// *******************************************************************************************************************************

ImageLoaderDialog::ImageLoaderDialog(QWidget *parent) : QDialog(parent)
{
    // setWindowIcon(icon("browser"));
    setWindowTitle(tr("Image"));
    setWindowFlags(Qt::Window);

    createControls();

    QSettings settings;
    restoreGeometry(settings.value("ImageLoaderDialog/Geometry", saveGeometry()).toByteArray());

    txtX->setValue(settings.value("ImageLoaderDialog/PositionX", 0.0).toDouble());
    txtY->setValue(settings.value("ImageLoaderDialog/PositionY", 0.0).toDouble());
    txtWidth->setValue(settings.value("ImageLoaderDialog/PositionWith", 1.0).toDouble());
    txtHeight->setValue(settings.value("ImageLoaderDialog/PositionHeight", 1.0).toDouble());
    doLoadFile(settings.value("ImageLoaderDialog/FileName").toString());
}

ImageLoaderDialog::~ImageLoaderDialog()
{
    QSettings settings;
    settings.setValue("ImageLoaderDialog/Geometry", saveGeometry());
}

void ImageLoaderDialog::createControls()
{
    lblImage = new QLabel();
    lblImage->setBackgroundRole(QPalette::Base);
    lblImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    lblImage->setScaledContents(true);
    lblImage->setMinimumSize(400, 300);

    lblImageFileName = new QLabel();

    QPushButton *btnLoadImage = new QPushButton(icon("three-dots"), "");
    btnLoadImage->setMaximumSize(btnLoadImage->sizeHint());
    connect(btnLoadImage, SIGNAL(clicked()), this, SLOT(doLoadFile()));

    QPushButton *btnRemoveImage = new QPushButton(icon("remove-item"), "");
    connect(btnRemoveImage, SIGNAL(clicked()), this, SLOT(doRemoveFile()));

    QHBoxLayout *layoutFileName = new QHBoxLayout();
    layoutFileName->addWidget(new QLabel(tr("Filename:")));
    layoutFileName->addWidget(lblImageFileName, 1);
    layoutFileName->addWidget(btnRemoveImage);
    layoutFileName->addWidget(btnLoadImage);

    txtX = new SLineEditDouble(0.0);
    txtY = new SLineEditDouble(0.0);
    txtWidth = new SLineEditDouble(1.0);
    txtHeight = new SLineEditDouble(1.0);

    QGridLayout *layoutPosition = new QGridLayout();
    layoutPosition->addWidget(new QLabel(tr("X:")), 0, 0);
    layoutPosition->addWidget(txtX, 0, 1);
    layoutPosition->addWidget(new QLabel(tr("Y:")), 1, 0);
    layoutPosition->addWidget(txtY, 1, 1);
    layoutPosition->addWidget(new QLabel(tr("Width:")), 0, 2);
    layoutPosition->addWidget(txtWidth, 0, 3);
    layoutPosition->addWidget(new QLabel(tr("Height:")), 1, 2);
    layoutPosition->addWidget(txtHeight, 1, 3);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutFileName);
    layout->addLayout(layoutPosition);
    layout->addWidget(lblImage, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ImageLoaderDialog::doRemoveFile()
{
    doLoadFile("");
}

void ImageLoaderDialog::doLoadFile()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), m_fileName, tr("Images (*.png *.bmp *.jpg)"));
    if (!fileName.isEmpty())
    {
        doLoadFile(fileName);
        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void ImageLoaderDialog::doLoadFile(const QString &fileName)
{
    m_fileName = fileName;
    lblImageFileName->setText(m_fileName);

    if (QFile::exists(m_fileName))
    {
        QPixmap pixmap(m_fileName);
        lblImage->setPixmap(pixmap);
        lblImage->resize(lblImage->pixmap()->size());
    }
    else
    {
        lblImage->setPixmap(QPixmap());
    }
}

void ImageLoaderDialog::doAccept()
{
    m_position.setX(txtX->value());
    m_position.setY(txtY->value());
    m_position.setWidth(txtWidth->value());
    m_position.setHeight(txtHeight->value());

    QSettings settings;
    settings.setValue("ImageLoaderDialog/PositionX", txtX->value());
    settings.setValue("ImageLoaderDialog/PositionY", txtY->value());
    settings.setValue("ImageLoaderDialog/PositionWith", txtWidth->value());
    settings.setValue("ImageLoaderDialog/PositionHeight", txtHeight->value());
    settings.setValue("ImageLoaderDialog/FileName", m_fileName);

    accept();
}

void ImageLoaderDialog::doReject()
{
    reject();
}

// ***************************************************************************************************************

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("agros2d"));
    setWindowTitle(tr("About..."));

    createControls();

    setMinimumSize(450, 550);
    setMaximumSize(sizeHint());
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::createControls()
{
    QTabWidget *tab = new QTabWidget();
    tab->addTab(createAgros2D(), icon(""), tr("Agros2D"));
    tab->addTab(createHermes2D(), tr("Hermes2D"));
    tab->addTab(createLibraries(), tr("Libraries"));
    tab->addTab(createLicense(), tr("License"));

    QPushButton *buttonClose = new QPushButton(tr("Close"));
    QPushButton *buttonCheckForNewVersion = new QPushButton(tr("Check version"));

    QHBoxLayout *buttonBox = new QHBoxLayout();
    buttonBox->addStretch();
    buttonBox->addWidget(buttonCheckForNewVersion);
    buttonBox->addWidget(buttonClose);

    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(buttonCheckForNewVersion, SIGNAL(clicked()), this, SLOT(checkVersion()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tab);
    // layout->addStretch();
    layout->addLayout(buttonBox);

    setLayout(layout);
}

QWidget *AboutDialog::createAgros2D()
{
    QLabel *labelContent = new QLabel(tr("<h3>Agros2D %1</h3>"
                                         "Agros2D is a multiplatform multiphysics application for the solution of partial differential equations (PDE) "
                                         "based on the <a href=\"http://hpfem.org/hermes2d/\">Hermes2D</a> library using higher order finite element method "
                                         "(<i>hp</i>-FEM) with automatic adaptivity.<br/><br/>"
                                         "Web page: <a href=\"http://agros2d.org/\">http://agros2d.org/</a><br/>"
                                         "Facebook: <a href=\"http://www.facebook.com/pages/Agros2D/132524130149770?sk=info\">http://www.facebook.com/pages/Agros2D/...</a><br/>"
                                         "Issues: <a href=\"http://github.com/hpfem/agros2d/issues\">http://github.com/hpfem/agros2d/issues</a><br/><br/>"
                                         "<b>Authors:</b>"
                                         "<p>"
                                         "<b>Pavel Karban</b> - main developer (University of West Bohemia, Pilsen)<br/>"
                                         "<b>František Mach</b> - developer, documentation (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Pavel Kůs</b> - developer (Academy of Sciences of the Czech Republic, Prague)<br/>"
                                         "<b>David Pánek</b> - developer (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Lukáš Koudela</b> - developer - RF module (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Bartosz Sawicki</b> - Polish translation (Warsaw University of Technology, Warsaw)<br/>"
                                         "<b>Václav Kotlan</b> - German translation (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Petr Kropík</b> - University of West Bohemia, Pilsen<br/>"
                                         "</p>")
                                      .arg(QApplication::applicationVersion()));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);


    QLabel *labelIcon = new QLabel();
    labelIcon->setPixmap(icon("agros2d").pixmap(64, 64));

    QGridLayout *layoutIcon = new QGridLayout();
    layoutIcon->addWidget(labelIcon, 0, 0, 1, 1, Qt::AlignTop);
    layoutIcon->addWidget(labelContent, 0, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutIcon);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createHermes2D()
{
    QLabel *labelContent = new QLabel(tr("<h3>Hermes2D</h3>"
                                         "Web page: <a href=\"http://hpfem.org/hermes/\">http://hpfem.org/hermes/</a><br/><br/>"
                                         "<b>Authors:</b>"
                                         "<p>"
                                         "<b>Pavel Solin</b> (project leader, University of Nevada, Reno)<br/>"
                                         "<b>Martin Zitka</b> (formerly University of Texas at El Paso)<br/>"
                                         "<b>Tomas Vejchodsky</b> (Academy of Sciences of the Czech Republic, Prague)<br/>"
                                         "<b>Svatava Vyvialova</b> (formerly University of Texas at El Paso)<br/>"
                                         "<b>Jakub Cerveny</b> (formerly University of Texas at El Paso)<br/>"
                                         "<b>Lenka Dubcova</b> (Academy of Sciences of the Czech Republic, Prague)<br/>"
                                         "<b>Ondrej Certik</b> (University of Nevada, Reno)<br/>"
                                         "<b>Robert Cimrman</b> (New Technologies Research Centre, Pilsen)<br/>"
                                         "<b>Ivo Hanak</b> (University of West Bohemia, Pilsen)<br/>"
                                         "<b>David Andrs</b> (formerly University of Nevada, Reno)<br/>"
                                         "<b>Pavel Karban</b> (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Frantisek Mach</b> (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Sameer Regmi</b> (University of Nevada, Reno)<br/>"
                                         "<b>David Puga</b>l (University of Nevada, Reno)<br/>"
                                         "<b>Ma Zhonghua</b> (China University of Petroleum, Beijing)<br/>"
                                         "<b>Milan Hanus</b> (University of West Bohemia, Pilsen)<br/>"
                                         "<b>Damien Lebrun-Grandie</b> (Texas A&M University)<br/>"
                                         "<b>Lukas Korous</b> (Charles University, Prague)<br/>"
                                         "<b>Valmor de Almeida</b> (Oak Ridge National Laboratory, Oak Ridge)<br/>"
                                         "</p>"
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLibraries()
{
    QLabel *labelContent = new QLabel(tr("<h3>Libraries</h3>"
                                         "<b>Nokia Qt:</b> Nokia Qt (<a href=\"http://qt.nokia.com/\">Qt - A cross-platform framework</a>)<br/>"
                                         "<b>Qwt:</b> Qwt (<a href=\"http://qwt.sourceforge.net/\">Qt Widgets for Technical Applications</a>)<br/>"
                                         "<b>Python:</b> Python Programming Language (<a href=\"http://www.python.org\">Python</a>)<br/>"
                                         "<b>dxflib:</b> Andrew Mustun (<a href=\"http://www.ribbonsoft.com/dxflib.html\">RibbonSoft</a>)<br/>"
                                         "<b>Triangle:</b> Jonathan Richard Shewchuk (<a href=\"http://www.cs.cmu.edu/~quake/triangle.html\">Triangle</a>)<br/>"
                                         "<b>FFmpeg:</b> FFmpeg group (<a href=\"http://ffmpeg.org/\">FFmpeg</a>)<br/>"
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLicense()
{
    QLabel *labelContent = new QLabel(tr("<h3>License:</h3>"
                                         "<p>Agros2D is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.</p><p>Agros2D is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p><p>You should have received a copy of the GNU General Public License along with Agros2D. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

void AboutDialog::checkVersion()
{
    checkForNewVersion();
}
