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

#include "study.h"
#include "study_dialog.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "study_sweep.h"
#include "study_genetic.h"
#include "study_nlopt.h"
#include "study_bayesopt.h"

#include "qcustomplot/qcustomplot.h"

LogOptimizationDialog::LogOptimizationDialog(Study *study) : QDialog(QApplication::activeWindow()),
    m_study(study), m_chart(nullptr), m_objectiveGraph(nullptr), m_progress(nullptr)
{
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(study->name());
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();

    connect(btnAbort, SIGNAL(clicked()), m_study, SLOT(doAbortSolve()));
    connect(btnAbort, SIGNAL(clicked()), this, SLOT(aborted()));
    connect(m_study, SIGNAL(updateChart()), this, SLOT(updateChart()));
    connect(m_study, SIGNAL(solved()), this, SLOT(solved()));
    connect(m_study, SIGNAL(updateParameters(QList<Parameter>, const Computation *)), this, SLOT(updateParameters(QList<Parameter>, const Computation *)));

    int w = 2.0/3.0 * QApplication::desktop()->screenGeometry().width();
    int h = 2.0/3.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

LogOptimizationDialog::~LogOptimizationDialog()
{
}

void LogOptimizationDialog::closeEvent(QCloseEvent *e)
{
    if (m_study->isSolving())
        e->ignore();
}

void LogOptimizationDialog::reject()
{
    if (m_study->isSolving())
        m_study->doAbortSolve();
    else
        close();
}

void LogOptimizationDialog::tryClose()
{
    if (m_study->isSolving())
    {
        Agros2D::log()->printError(tr("Solver"), tr("Stydy is being aborted."));
    }
    else
    {
        close();
    }
}

void LogOptimizationDialog::createControls()
{
    m_logWidget = new LogWidget(this);

#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif

    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(tryClose()));
    btnClose->setEnabled(false);

    btnAbort = new QPushButton(tr("Abort"));

    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addStretch();
    layoutStatus->addWidget(btnAbort, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);

    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setWidth(2);

    QPen penError;
    penError.setColor(Qt::darkRed);
    penError.setWidth(2);

    QFont fontTitle(font());
    fontTitle.setBold(true);

    QFont fontChart(font());
    fontChart.setPointSize(fontSize);

    // transient
    m_chart = new QCustomPlot(this);
    QCPPlotTitle *timeTitle = new QCPPlotTitle(m_chart, tr("Optimization"));
    timeTitle->setFont(fontTitle);
    m_chart->plotLayout()->insertRow(0);
    m_chart->plotLayout()->addElement(0, 0, timeTitle);
    m_chart->legend->setVisible(true);
    m_chart->legend->setFont(fontChart);

    m_chart->xAxis->setTickLabelFont(fontChart);
    m_chart->xAxis->setLabelFont(fontChart);
    // m_chart->xAxis->setTickStep(1.0);
    m_chart->xAxis->setAutoTickStep(true);
    m_chart->xAxis->setLabel(tr("number of steps"));

    m_chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_chart->yAxis->setTickLabelFont(fontChart);
    m_chart->yAxis->setLabelFont(fontChart);
    m_chart->yAxis->setLabel(tr("objective function"));

    m_objectiveGraph = m_chart->addGraph(m_chart->xAxis, m_chart->yAxis);
    m_objectiveGraph->setLineStyle(QCPGraph::lsLine);
    m_objectiveGraph->setPen(pen);
    m_objectiveGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
    m_objectiveGraph->setName(tr("objective function"));

    m_progress = new QProgressBar(this);
    m_progress->setMaximum(10000);

    QVBoxLayout *layoutObjective = new QVBoxLayout();
    layoutObjective->addWidget(m_chart, 2);
    layoutObjective->addWidget(m_progress, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutObjective);
    layout->addWidget(m_logWidget);
    // layout->addStretch();
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogOptimizationDialog::updateChart()
{
    QVector<double> steps;
    QVector<double> objective;

    for (int i = 0; i < m_study->computationSets().count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = m_study->computationSets()[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            steps.append(steps.count() + 1);

            // evaluate goal functional
            double value = m_study->evaluateGoal(computations[j]);
            objective.append(value);
        }
    }

    m_objectiveGraph->setData(steps, objective);
    m_chart->rescaleAxes();
    m_chart->replot(QCustomPlot::rpImmediate);

    QApplication::processEvents();
}

void LogOptimizationDialog::solved()
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);

    tryClose();
}

void LogOptimizationDialog::aborted()
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
}

void LogOptimizationDialog::updateParameters(QList<Parameter> parameters, const Computation *computation)
{
    QString params = "";
    foreach (Parameter parameter, parameters)
    {
        params += QString("%1 = %2, ").arg(parameter.name()).arg(computation->config()->parameter(parameter.name()));
    }
    if (params.size() > 0)
        params = params.left(params.size() - 2);

    QString res = "";
    foreach (QString name, computation->results()->items().keys())
    {
        res += QString("%1 = %2, ").arg(name).arg(computation->results()->resultValue(name));
    }
    if (res.size() > 0)
        res = res.left(res.size() - 2);

    Agros2D::log()->printMessage(tr("Study"), tr("Parameters: %1, results: %2").arg(params).arg(res));
}

// ******************************************************************************************************************

StudySelectDialog::StudySelectDialog(QWidget *parent) : QDialog(parent), m_selectedStudyType(StudyType_Undefined)
{
    setWindowTitle(tr("Add study"));
    setModal(true);

    lstStudies = new QListWidget(this);
    lstStudies->setIconSize(QSize(24, 24));
    lstStudies->setMinimumHeight(26*3);

    foreach (QString name, studyTypeStringKeys())
    {
        QListWidgetItem *item = new QListWidgetItem(lstStudies);
        item->setIcon(iconAlphabet(studyTypeString(studyTypeFromStringKey(name)).at(0), AlphabetColor_Bluegray));
        item->setText(studyTypeString(studyTypeFromStringKey(name)));
        item->setData(Qt::UserRole, studyTypeFromStringKey(name));

        lstStudies->addItem(item);
    }

    connect(lstStudies, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstStudies, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstStudies, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstStudies);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstStudies->count() > 0)
    {
        lstStudies->setCurrentRow(0);
        doItemSelected(lstStudies->currentItem());
    }

    int w = sizeHint().width();
    int h = 1.0/3.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void StudySelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedStudyType = (StudyType) item->data(Qt::UserRole).toInt();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void StudySelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstStudies->currentItem())
    {
        m_selectedStudyType = (StudyType) item->data(Qt::UserRole).toInt();
        accept();
    }
}

// *******************************************************************************************************************

StudyDialog *StudyDialog::factory(Study *study, QWidget *parent)
{
    if (study->type() == StudyType_BayesOptAnalysis)
        return new StudyBayesOptAnalysisDialog(study, parent);
    else if (study->type() == StudyType_NLoptAnalysis)
        return new StudyNLoptAnalysisDialog(study, parent);

    assert(0);
    return nullptr;
}

StudyDialog::StudyDialog(Study *study, QWidget *parent) : QDialog(parent),
    m_study(study)
{
    setWindowTitle(tr("%1").arg(studyTypeString(study->type())));
    setAttribute(Qt::WA_DeleteOnClose);
}

int StudyDialog::showDialog()
{
    createControls();
    load();

    return exec();
}

void StudyDialog::createControls()
{
    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    tabStudy = new QTabWidget();
    tabStudy->addTab(createStudyControls(), tr("Study"));
    tabStudy->addTab(createParameters(), tr("Parameters"));
    tabStudy->addTab(createFunctionals(), tr("Functionals"));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabStudy);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    readParameters();
    readFunctionals();
}

QWidget *StudyDialog::createParameters()
{
    trvParameterWidget = new QTreeWidget(this);
    trvParameterWidget->setExpandsOnDoubleClick(false);
    trvParameterWidget->setHeaderHidden(false);
    trvParameterWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Lower bound") << tr("Upper bound"));
    trvParameterWidget->setColumnCount(3);
    trvParameterWidget->setIndentation(2);
    trvParameterWidget->headerItem()->setTextAlignment(1, Qt::AlignRight);
    trvParameterWidget->headerItem()->setTextAlignment(2, Qt::AlignRight);

    // connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvParameterWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doParameterItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvParameterWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doParameterItemDoubleClicked(QTreeWidgetItem *, int)));

    btnParameterAdd = new QPushButton(tr("Add"), this);
    btnParameterEdit = new QPushButton(tr("Edit"), this);
    btnParameterRemove = new QPushButton(tr("Remove"), this);
    connect(btnParameterRemove, SIGNAL(clicked(bool)), this, SLOT(doParameterRemove(bool)));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnParameterAdd);
    buttonsLayout->addWidget(btnParameterEdit);
    buttonsLayout->addWidget(btnParameterRemove);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvParameterWidget);
    layout->addLayout(buttonsLayout);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    return widget;
}

void StudyDialog::readParameters()
{
    trvParameterWidget->clear();

    foreach (Parameter parameter, m_study->parameters())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvParameterWidget);

        item->setText(0, QString("%1").arg(parameter.name()));
        item->setData(0, Qt::UserRole, parameter.name());
        item->setText(1, QString("%1").arg(parameter.lowerBound()));
        item->setText(2, QString("%1").arg(parameter.upperBound()));
        item->setTextAlignment(1, Qt::AlignRight);
        item->setTextAlignment(2, Qt::AlignRight);
    }

    doParameterItemChanged(nullptr, nullptr);
}

void StudyDialog::doParameterItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    btnParameterEdit->setEnabled(trvParameterWidget->currentItem());
    btnParameterRemove->setEnabled(trvParameterWidget->currentItem());

    if (trvParameterWidget->currentItem())
    {

    }
}

void StudyDialog::doParameterItemDoubleClicked(QTreeWidgetItem *item, int role)
{

}

void StudyDialog::doParameterRemove(bool checked)
{
    if (trvParameterWidget->currentItem())
    {
        m_study->removeParameter(trvParameterWidget->currentItem()->data(0, Qt::UserRole).toString());

        readParameters();
    }
}

QWidget *StudyDialog::createFunctionals()
{
    trvFunctionalWidget = new QTreeWidget(this);
    trvFunctionalWidget->setExpandsOnDoubleClick(false);
    trvFunctionalWidget->setHeaderHidden(false);
    trvFunctionalWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Weight") << tr("Expression"));
    trvFunctionalWidget->setColumnCount(3);
    trvFunctionalWidget->setIndentation(2);

    // connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvFunctionalWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doFunctionalItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvFunctionalWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doFunctionalItemDoubleClicked(QTreeWidgetItem *, int)));

    btnFunctionalAdd = new QPushButton(tr("Add"), this);
    connect(btnFunctionalAdd, SIGNAL(clicked(bool)), this, SLOT(doFunctionalAdd(bool)));
    btnFunctionalEdit = new QPushButton(tr("Edit"), this);
    connect(btnFunctionalEdit, SIGNAL(clicked(bool)), this, SLOT(doFunctionalEdit(bool)));
    btnFunctionalRemove = new QPushButton(tr("Remove"), this);
    connect(btnFunctionalRemove, SIGNAL(clicked(bool)), this, SLOT(doFunctionalRemove(bool)));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnFunctionalAdd);
    buttonsLayout->addWidget(btnFunctionalEdit);
    buttonsLayout->addWidget(btnFunctionalRemove);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvFunctionalWidget);
    layout->addLayout(buttonsLayout);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);


    return widget;
}

void StudyDialog::readFunctionals()
{
    trvFunctionalWidget->clear();

    foreach (Functional functional, m_study->functionals())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvFunctionalWidget);

        item->setText(0, QString("%1").arg(functional.name()));
        item->setData(0, Qt::UserRole, functional.name());
        item->setText(1, QString("%1 \%").arg(functional.weight()));
        item->setText(2, QString("%1").arg(functional.expression()));
    }

    doFunctionalItemChanged(nullptr, nullptr);
}

void StudyDialog::doFunctionalItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    btnFunctionalEdit->setEnabled(trvFunctionalWidget->currentItem());
    btnFunctionalRemove->setEnabled(trvFunctionalWidget->currentItem());

    if (trvFunctionalWidget->currentItem())
    {

    }
}

void StudyDialog::doFunctionalItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    if (trvFunctionalWidget->currentItem())
    {
        doFunctionalEdit(true);
    }
}

void StudyDialog::doFunctionalAdd(bool checked)
{
    Functional functional;

    FunctionalDialog dialog(m_study, &functional);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_study->addFunctional(functional);
        readFunctionals();
    }
}

void StudyDialog::doFunctionalEdit(bool checked)
{
    if (trvFunctionalWidget->currentItem())
    {
        FunctionalDialog dialog(m_study, &m_study->functional(trvFunctionalWidget->currentItem()->data(0, Qt::UserRole).toString()));
        if (dialog.exec() == QDialog::Accepted)
        {
            readFunctionals();
        }
    }
}

void StudyDialog::doFunctionalRemove(bool checked)
{
    if (trvFunctionalWidget->currentItem())
    {
        m_study->removeFunctional(trvFunctionalWidget->currentItem()->data(0, Qt::UserRole).toString());

        readFunctionals();
    }
}

void StudyDialog::doAccept()
{
    save();
    accept();
}

// **************************************************************************************************************

FunctionalDialog::FunctionalDialog(Study *study, Functional *functional, QWidget *parent)
    : m_study(study), m_functional(functional)
{
    createControls();
}

void FunctionalDialog::createControls()
{
    setWindowTitle(tr("Functional: %1").arg(m_functional->name()));

    lblError = new QLabel();

    txtName = new QLineEdit(m_functional->name());
    connect(txtName, SIGNAL(textChanged(QString)), this, SLOT(functionalNameTextChanged(QString)));
    txtExpression = new QLineEdit(m_functional->expression());
    txtWeight = new QSpinBox();
    txtWeight->setRange(0, 100);
    txtWeight->setValue(m_functional->weight());

    QGridLayout *layoutEdit = new QGridLayout();
    layoutEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutEdit->addWidget(txtName, 0, 1);
    layoutEdit->addWidget(new QLabel(tr("Expression")), 1, 0);
    layoutEdit->addWidget(txtExpression, 1, 1);
    layoutEdit->addWidget(new QLabel(tr("Weight")), 2, 0);
    layoutEdit->addWidget(txtWeight, 2, 1);

    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QVBoxLayout *layoutWidget = new QVBoxLayout();
    layoutWidget->addLayout(layoutEdit);
    layoutWidget->addWidget(lblError);
    layoutWidget->addStretch();
    layoutWidget->addWidget(buttonBox);

    setLayout(layoutWidget);

    if (!m_functional->name().isEmpty())
        txtName->setFocus();
}

void FunctionalDialog::functionalNameTextChanged(const QString &str)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checkFunctional(str));
}

bool FunctionalDialog::checkFunctional(const QString &str)
{
    try
    {
        Agros2D::problem()->config()->checkVariableName(str);
    }
    catch (AgrosException &e)
    {
        lblError->setText(e.toString());
        lblError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }

    foreach (Functional functional, m_study->functionals())
    {
        if (str == m_functional->name())
            continue;

        if (str == functional.name())
        {
            lblError->setText(tr("Functional already exists."));
            lblError->setVisible(true);

            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return false;
        }
    }

    lblError->setVisible(false);


    return true;
}

void FunctionalDialog::doAccept()
{
    if (checkFunctional(txtName->text()))
    {
        m_functional->setName(txtName->text());
        m_functional->setExpression(txtExpression->text());
        m_functional->setWeight(txtWeight->value());

        accept();
    }
}
