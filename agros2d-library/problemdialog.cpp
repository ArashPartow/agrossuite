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

#include "problemdialog.h"

#include "solver/plugin_interface.h"

#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
// #include "moduledialog.h"
#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/coupling.h"
#include "solver/problem_config.h"

#include "gui/lineeditdouble.h"
#include "gui/valuelineedit.h"
#include "gui/groupbox.h"
#include "gui/common.h"

#include "pythonlab/pythoneditor.h"

FieldSelectDialog::FieldSelectDialog(QList<QString> fields, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Select field"));
    setModal(true);

    m_selectedFieldId = "";

    lstFields = new QListWidget(this);
    lstFields->setIconSize(QSize(32, 32));
    lstFields->setMinimumHeight(36*9);

    QMapIterator<QString, QString> it(Module::availableModules());
    while (it.hasNext())
    {
        it.next();
        // add only missing fields
        if (!fields.contains(it.key()))
        {
            QListWidgetItem *item = new QListWidgetItem(lstFields);
            item->setIcon(icon("fields/" + it.key()));
            item->setText(it.value());
            item->setData(Qt::UserRole, it.key());

            lstFields->addItem(item);
        }
    }

    connect(lstFields, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstFields, SIGNAL(itemPressed(QListWidgetItem *)),
            this, SLOT(doItemSelected(QListWidgetItem *)));

    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstFields);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstFields->count() > 0)
    {
        lstFields->setCurrentRow(0);
        doItemSelected(lstFields->currentItem());
    }

    int w = sizeHint().width();
    int h = 1.0/2.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void FieldSelectDialog::doAccept()
{
    accept();
}

void FieldSelectDialog::doReject()
{
    reject();
}

int FieldSelectDialog::showDialog()
{
    return exec();
}

void FieldSelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedFieldId = item->data(Qt::UserRole).toString();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void FieldSelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstFields->currentItem())
    {
        m_selectedFieldId = lstFields->currentItem()->data(Qt::UserRole).toString();
        accept();
    }
}

// ********************************************************************************************************

FieldWidget::FieldWidget(FieldInfo *fieldInfo, QWidget *parent)
    : QWidget(parent), m_fieldInfo(fieldInfo)
{
    createContent();
    load();
}

void FieldWidget::createContent()
{
    // equations
    // equationLaTeX = new LaTeXViewer(this);
    // equationLaTeX->setMaximumWidth(400);
    equationImage = new QLabel();

    cmbAdaptivityType = new QComboBox();
    cmbAnalysisType = new QComboBox();
    cmbLinearityType = new QComboBox();
    cmbLinearSolver = new QComboBox();

    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    connect(cmbLinearityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearityTypeChanged(int)));
    connect(cmbLinearSolver, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearSolverChanged(int)));

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(5);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);

    // table
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Analysis:")), 0, 0);
    layoutGeneral->addWidget(cmbAnalysisType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Solver:")), 1, 0);
    layoutGeneral->addWidget(cmbLinearityType, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Matrix solver:")), 2, 0);
    layoutGeneral->addWidget(cmbLinearSolver, 2, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // mesh
    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 0, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 1, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 1, 1);
    layoutMesh->addWidget(new QLabel(tr("Space adaptivity:")), 2, 0);
    layoutMesh->addWidget(cmbAdaptivityType, 2, 1);
    layoutMesh->setRowStretch(50, 1);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh parameters"));
    grpMesh->setLayout(layoutMesh);

    // left
    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(grpGeneral);
    layoutLeft->addStretch();

    // right
    QVBoxLayout *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpMesh);
    layoutRight->addStretch();

    // both
    QHBoxLayout *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // equation
    QVBoxLayout *layoutEquation = new QVBoxLayout();
    // layoutEquation->addWidget(equationLaTeX);
    layoutEquation->addWidget(equationImage);
    layoutEquation->addStretch();

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(createSolverWidget(), tr("Solver"));
    tabWidget->addTab(createAdaptivityWidget(), tr("Space adaptivity"));
    tabWidget->addTab(createTransientAnalysisWidget(), tr("Transient analysis"));
    tabWidget->addTab(createLinearSolverWidget(), tr("Matrix solver"));

    QGroupBox *grpEquation = new QGroupBox(tr("Partial differential equation"));
    grpEquation->setLayout(layoutEquation);

    QVBoxLayout *layoutProblem = new QVBoxLayout();
    layoutProblem->addWidget(grpEquation);
    layoutProblem->addLayout(layoutPanel);
    layoutProblem->addWidget(tabWidget);

    // fill combobox
    fillComboBox();

    setLayout(layoutProblem);
}

QWidget *FieldWidget::createSolverWidget()
{
    // linearity
    chkNonlinearResidual = new QCheckBox(this);
    connect(chkNonlinearResidual, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearResidual(int)));
    txtNonlinearResidual = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearResidualNorm).toDouble());
    txtNonlinearResidual->setBottom(0.0);

    chkNonlinearRelativeChangeOfSolutions = new QCheckBox(this);
    connect(chkNonlinearRelativeChangeOfSolutions, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearRelativeChangeOfSolutions(int)));
    txtNonlinearRelativeChangeOfSolutions = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    txtNonlinearRelativeChangeOfSolutions->setBottom(0.0);

    QGridLayout *layoutSolverConvergence = new QGridLayout();
    layoutSolverConvergence->setColumnMinimumWidth(0, columnMinimumWidth());

    layoutSolverConvergence->addWidget(new QLabel(tr("Relative change of solutions (%):")), 0, 0);
    layoutSolverConvergence->addWidget(chkNonlinearRelativeChangeOfSolutions, 0, 1);
    layoutSolverConvergence->addWidget(txtNonlinearRelativeChangeOfSolutions, 0, 2);
    layoutSolverConvergence->addWidget(new QLabel(tr("Residual:")), 1, 0);
    layoutSolverConvergence->addWidget(chkNonlinearResidual, 1, 1);
    layoutSolverConvergence->addWidget(txtNonlinearResidual, 1, 2);

    QGroupBox *grpSolverConvergence = new QGroupBox(tr("Convergence (all selected conditions have to be satisfied)"));
    grpSolverConvergence->setLayout(layoutSolverConvergence);

    cmbNonlinearDampingType = new QComboBox();
    connect(cmbNonlinearDampingType, SIGNAL(currentIndexChanged(int)), this, SLOT(doNonlinearDampingChanged(int)));
    txtNonlinearDampingCoeff = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNonlinearDampingCoeff->setBottom(0.0);
    txtNonlinearDampingStepsForFactorIncrease = new QSpinBox(this);
    txtNonlinearDampingStepsForFactorIncrease->setMinimum(1);
    txtNonlinearDampingStepsForFactorIncrease->setMaximum(5);
    txtNonlinearDampingRatioForFactorDecrease = new LineEditDouble();

    QGridLayout *layoutSolverDamping = new QGridLayout();

    layoutSolverDamping->addWidget(new QLabel(tr("Damping type:")), 0, 0);
    layoutSolverDamping->addWidget(cmbNonlinearDampingType, 0, 1);
    layoutSolverDamping->addWidget(new QLabel(tr("Factor:")), 1, 0);
    layoutSolverDamping->addWidget(txtNonlinearDampingCoeff, 1, 1);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. residual ratio for factor decrease:")), 0, 2);
    layoutSolverDamping->addWidget(txtNonlinearDampingRatioForFactorDecrease, 0, 3);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. steps for factor increase:")), 1, 2);
    layoutSolverDamping->addWidget(txtNonlinearDampingStepsForFactorIncrease, 1, 3);

    QGroupBox *grpSolverDamping = new QGroupBox(tr("Damping"));
    grpSolverDamping->setLayout(layoutSolverDamping);

    chkNewtonReuseJacobian = new QCheckBox(tr("Reuse Jacobian if possible"));
    connect(chkNewtonReuseJacobian, SIGNAL(toggled(bool)), this, SLOT(doNewtonReuseJacobian(bool)));
    txtNewtonSufficientImprovementFactorForJacobianReuse = new LineEditDouble();
    txtNewtonMaximumStepsWithReusedJacobian = new QSpinBox(this);
    txtNewtonMaximumStepsWithReusedJacobian->setMinimum(0);
    txtNewtonMaximumStepsWithReusedJacobian->setMaximum(100);

    // Newton's solver
    QGridLayout *layoutNewtonSolverReuse = new QGridLayout();
    layoutNewtonSolverReuse->addWidget(chkNewtonReuseJacobian, 0, 0);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. residual ratio for Jacobian reuse:")), 1, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonSufficientImprovementFactorForJacobianReuse, 1, 1);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. steps with the same Jacobian:")), 2, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonMaximumStepsWithReusedJacobian, 2, 1);
    layoutNewtonSolverReuse->setRowStretch(50, 1);

    QWidget *widgetNewtonSolver = new QWidget(this);
    widgetNewtonSolver->setLayout(layoutNewtonSolverReuse);

    // Picard's solver
    chkPicardAndersonAcceleration = new QCheckBox(tr("Use Anderson acceleration"));
    connect(chkPicardAndersonAcceleration, SIGNAL(stateChanged(int)), this, SLOT(doPicardAndersonChanged(int)));
    txtPicardAndersonBeta = new LineEditDouble(0.2);
    txtPicardAndersonBeta->setBottom(0.0);
    txtPicardAndersonBeta->setTop(1.0);
    txtPicardAndersonNumberOfLastVectors = new QSpinBox(this);
    txtPicardAndersonNumberOfLastVectors->setMinimum(1);
    txtPicardAndersonNumberOfLastVectors->setMaximum(5);

    QGridLayout *layoutPicardSolver = new QGridLayout(this);
    layoutPicardSolver->addWidget(chkPicardAndersonAcceleration, 0, 0, 1, 2);
    layoutPicardSolver->addWidget(new QLabel(tr("Anderson beta:")), 1, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonBeta, 1, 1);
    layoutPicardSolver->addWidget(new QLabel(tr("Num. of last used iter.:")), 2, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonNumberOfLastVectors, 2, 1);
    layoutPicardSolver->setRowStretch(50, 1);

    QWidget *widgetPicardSolver = new QWidget(this);
    widgetPicardSolver->setLayout(layoutPicardSolver);

    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(widgetNewtonSolver, tr("Newton's solver"));
    tab->addTab(widgetPicardSolver, tr("Picard's solver"));

    QVBoxLayout *layoutSolver = new QVBoxLayout(this);
    layoutSolver->addWidget(grpSolverConvergence);
    layoutSolver->addWidget(grpSolverDamping);
    layoutSolver->addWidget(tab);

    QWidget *widSolver = new QWidget(this);
    widSolver->setLayout(layoutSolver);

    return widSolver;
}

QWidget *FieldWidget::createAdaptivityWidget()
{
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivitySteps->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance = new QCheckBox(this);
    connect(chkAdaptivityTolerance, SIGNAL(stateChanged(int)), this, SLOT(doAdaptivityTolerance(int)));
    txtAdaptivityTolerance = new LineEditDouble(1.0);
    txtAdaptivityTolerance->setBottom(0.0);
    cmbAdaptivityEstimator = new QComboBox();
    foreach (QString type, adaptivityEstimatorStringKeys())
        if (adaptivityEstimatorFromStringKey(type) != AdaptivityEstimator_Undefined)
            cmbAdaptivityEstimator->addItem(adaptivityEstimatorString(adaptivityEstimatorFromStringKey(type)),
                                            adaptivityEstimatorFromStringKey(type));
    cmbAdaptivityStrategy = new QComboBox();
    foreach (QString type, adaptivityStrategyStringKeys())
        if (adaptivityStrategyFromStringKey(type) != AdaptivityStrategy_Undefined)
            cmbAdaptivityStrategy->addItem(adaptivityStrategyString(adaptivityStrategyFromStringKey(type)),
                                           adaptivityStrategyFromStringKey(type));
    connect(cmbAdaptivityStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStrategyChanged(int)));

    cmbAdaptivityStrategyHP = new QComboBox();
    foreach (QString type, adaptivityStrategyHPStringKeys())
        if (adaptivityStrategyHPFromStringKey(type) != AdaptivityStrategyHP_Undefined)
            cmbAdaptivityStrategyHP->addItem(adaptivityStrategyHPString(adaptivityStrategyHPFromStringKey(type)),
                                             adaptivityStrategyHPFromStringKey(type));
    connect(cmbAdaptivityStrategyHP, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStrategyHPChanged(int)));

    txtAdaptivityFineFraction = new QSpinBox(this);
    txtAdaptivityFineFraction->setMinimum(1);
    txtAdaptivityFineFraction->setMaximum(100);
    txtAdaptivityFineFraction->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivityFinePercentage).toInt());
    txtAdaptivityCoarseFraction = new QSpinBox(this);
    txtAdaptivityCoarseFraction->setMinimum(1);
    txtAdaptivityCoarseFraction->setMaximum(100);
    txtAdaptivityCoarseFraction->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivityCoarsePercentage).toInt());
    txtAdaptivityBackSteps = new QSpinBox(this);
    txtAdaptivityBackSteps->setMinimum(0);
    txtAdaptivityBackSteps->setMaximum(100);
    txtAdaptivityRedoneEach = new QSpinBox(this);
    txtAdaptivityRedoneEach->setMinimum(1);
    txtAdaptivityRedoneEach->setMaximum(100);

    // control
    QGridLayout *layoutAdaptivityControl = new QGridLayout();
    layoutAdaptivityControl->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivityControl->addWidget(new QLabel(tr("Maximum steps:")), 0, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivitySteps, 0, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Tolerance (%):")), 1, 0);
    layoutAdaptivityControl->addWidget(chkAdaptivityTolerance, 1, 1, 1, 1, Qt::AlignRight);
    layoutAdaptivityControl->addWidget(txtAdaptivityTolerance, 1, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Error estimator:")), 2, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityEstimator, 2, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Control strategy:")), 3, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityStrategy, 3, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Percentage to be refined:")), 4, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivityFineFraction, 4, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Percentage to be coarsened:")), 5, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivityCoarseFraction, 5, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("<i>hp</i>-adaptivity strategy:")), 6, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityStrategyHP, 6, 2);
    layoutAdaptivityControl->setRowStretch(50, 1);

    QGroupBox *grpControl = new QGroupBox(tr("Control"), this);
    grpControl->setLayout(layoutAdaptivityControl);

    // transient
    QGridLayout *layoutAdaptivityTransient = new QGridLayout();
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Steps back in transient:")), 0, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityBackSteps, 0, 1);
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Redone each trans. step:")), 1, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityRedoneEach, 1, 1);
    layoutAdaptivityTransient->setRowStretch(50, 1);

    QGroupBox *grpTransient = new QGroupBox(tr("Transient analysis"), this);
    grpTransient->setLayout(layoutAdaptivityTransient);

    QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addWidget(grpControl);
    layoutAdaptivity->addWidget(grpTransient);
    layoutAdaptivity->addStretch(1);

    QWidget *widAdaptivity = new QWidget(this);
    widAdaptivity->setLayout(layoutAdaptivity);

    return widAdaptivity;
}

QWidget *FieldWidget::createTransientAnalysisWidget()
{
    // transient
    txtTransientInitialCondition = new LineEditDouble(0.0);
    txtTransientTimeSkip = new LineEditDouble(0.0);
    txtTransientTimeSkip->setBottom(0.0);

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 0, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Time skip (s):")), 1, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeSkip, 1, 1);
    layoutTransientAnalysis->setRowStretch(50, 1);

    QWidget *widTransientAnalysis = new QWidget(this);
    widTransientAnalysis->setLayout(layoutTransientAnalysis);

    return widTransientAnalysis;
}

QWidget *FieldWidget::createLinearSolverWidget()
{
    cmbIterLinearSolverDealIIMethod = new QComboBox();
    cmbIterLinearSolverDealIIPreconditioner = new QComboBox();
    txtIterLinearSolverToleranceAbsolute = new LineEditDouble(1e-15);
    txtIterLinearSolverToleranceAbsolute->setBottom(0.0);
    txtIterLinearSolverIters = new QSpinBox();
    txtIterLinearSolverIters->setMinimum(1);
    txtIterLinearSolverIters->setMaximum(10000);
    cmbExternalLinearSolverCommand = new QComboBox();
    connect(cmbExternalLinearSolverCommand, SIGNAL(currentIndexChanged(int)), this, SLOT(doExternalSolverChanged(int)));
    txtExternalLinearSolverCommandEnvironment = new QLineEdit();
    txtExternalLinearSolverCommandParameters = new QLineEdit();
    lblExternalLinearSolverHint = new QLabel();
    lblExternalLinearSolverHint->setWordWrap(false);
    lblExternalLinearSolverHint->setTextFormat(Qt::RichText);
    QFont fnt = lblExternalLinearSolverHint->font();
    fnt.setPointSize(fnt.pointSize() - 2);
    lblExternalLinearSolverHint->setFont(fnt);
    QPalette palette = lblExternalLinearSolverHint->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::darkBlue));
    lblExternalLinearSolverHint->setPalette(palette);

    QGridLayout *iterSolverDealIILayout = new QGridLayout();
    iterSolverDealIILayout->addWidget(new QLabel(tr("Method:")), 0, 0);
    iterSolverDealIILayout->addWidget(cmbIterLinearSolverDealIIMethod, 0, 1);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Preconditioner:")), 1, 0);
    iterSolverDealIILayout->addWidget(cmbIterLinearSolverDealIIPreconditioner, 1, 1);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Absolute tolerance:")), 0, 0);
    iterSolverDealIILayout->addWidget(txtIterLinearSolverToleranceAbsolute, 0, 1);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Maximum number of iterations:")), 1, 0);
    iterSolverDealIILayout->addWidget(txtIterLinearSolverIters, 1, 1);

    QGroupBox *iterSolverDealIIGroup = new QGroupBox(tr("deal.II"));
    iterSolverDealIIGroup->setLayout(iterSolverDealIILayout);

    QGridLayout *externalSolverLayout = new QGridLayout();
    externalSolverLayout->addWidget(new QLabel(tr("Solver:")), 0, 0);
    externalSolverLayout->addWidget(cmbExternalLinearSolverCommand, 0, 1);
    externalSolverLayout->addWidget(new QLabel(tr("Environment:")), 1, 0);
    externalSolverLayout->addWidget(txtExternalLinearSolverCommandEnvironment, 1, 1);
    externalSolverLayout->addWidget(new QLabel(tr("Parameters:")), 2, 0);
    externalSolverLayout->addWidget(txtExternalLinearSolverCommandParameters, 2, 1);
    externalSolverLayout->addWidget(lblExternalLinearSolverHint, 3, 0, 1, 2);

    QGroupBox *externalSolverGroup = new QGroupBox(tr("External (out of core)"));
    externalSolverGroup->setLayout(externalSolverLayout);

    QVBoxLayout *layoutLinearSolver = new QVBoxLayout();
    layoutLinearSolver->addWidget(iterSolverDealIIGroup);
    layoutLinearSolver->addWidget(externalSolverGroup);
    layoutLinearSolver->addStretch();

    QWidget *widLinearSolver = new QWidget(this);
    widLinearSolver->setLayout(layoutLinearSolver);

    return widLinearSolver;
}

void FieldWidget::fillComboBox()
{
    cmbNonlinearDampingType->clear();
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Automatic), DampingType_Automatic);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Fixed), DampingType_Fixed);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Off), DampingType_Off);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_None), AdaptivityMethod_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_H), AdaptivityMethod_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_P), AdaptivityMethod_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_HP), AdaptivityMethod_HP);

    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes())
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);
    }

    QMapIterator<AnalysisType, QString> it(m_fieldInfo->analyses());
    while (it.hasNext())
    {
        it.next();
        cmbAnalysisType->addItem(it.value(), it.key());
    }

    cmbLinearSolver->clear();
    foreach (QString solver, matrixSolverTypeStringKeys())
        cmbLinearSolver->addItem(matrixSolverTypeString(matrixSolverTypeFromStringKey(solver)), matrixSolverTypeFromStringKey(solver));

    cmbIterLinearSolverDealIIMethod->clear();
    foreach (QString method, iterLinearSolverDealIIMethodStringKeys())
        cmbIterLinearSolverDealIIMethod->addItem(iterLinearSolverDealIIMethodString(iterLinearSolverDealIIMethodFromStringKey(method)), iterLinearSolverDealIIMethodFromStringKey(method));

    cmbIterLinearSolverDealIIPreconditioner->clear();
    foreach (QString type, iterLinearSolverDealIIPreconditionerStringKeys())
        cmbIterLinearSolverDealIIPreconditioner->addItem(iterLinearSolverDealIIPreconditionerString(iterLinearSolverDealIIPreconditionerFromStringKey(type)), iterLinearSolverDealIIPreconditionerFromStringKey(type));

    // read from files
    cmbExternalLinearSolverCommand->clear();
    QDirIterator itDir(QString("%1/libs/").arg(datadir()), QStringList() << "*.ext", QDir::Files);
    while (itDir.hasNext())
    {
        QString fileName = QFileInfo(itDir.next()).fileName();

        // read command parameters
        QFile f(QString("%1/libs/%2").arg(datadir()).arg(fileName));
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&f);

            QStringList commandContent = in.readAll().split("\n");

            // command at second line
            QString name = commandContent.at(0);

            cmbExternalLinearSolverCommand->addItem(name, fileName);
        }
    }
    if (cmbExternalLinearSolverCommand->count() > 0)
        cmbExternalLinearSolverCommand->setCurrentIndex(0);
}

void FieldWidget::load()
{
    // analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_fieldInfo->analysisType()));
    if (cmbAnalysisType->currentIndex() == -1)
        cmbAnalysisType->setCurrentIndex(0);
    // adaptivity
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_fieldInfo->adaptivityType()));
    txtAdaptivitySteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance->setChecked(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble() > 0);
    txtAdaptivityTolerance->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());
    txtAdaptivityFineFraction->setValue(m_fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt());
    txtAdaptivityCoarseFraction->setValue(m_fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt());
    cmbAdaptivityEstimator->setCurrentIndex(cmbAdaptivityEstimator->findData((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt()));
    cmbAdaptivityStrategy->setCurrentIndex(cmbAdaptivityStrategy->findData((AdaptivityStrategy) m_fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt()));
    cmbAdaptivityStrategyHP->setCurrentIndex(cmbAdaptivityStrategyHP->findData((AdaptivityStrategyHP) m_fieldInfo->value(FieldInfo::AdaptivityStrategyHP).toInt()));
    txtAdaptivityBackSteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt());
    txtAdaptivityRedoneEach->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt());
    // matrix solver
    cmbLinearSolver->setCurrentIndex(cmbLinearSolver->findData(m_fieldInfo->matrixSolver()));
    //mesh
    txtNumberOfRefinements->setValue(m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    txtPolynomialOrder->setValue(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    // transient
    txtTransientInitialCondition->setValue(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble());
    txtTransientTimeSkip->setValue(m_fieldInfo->value(FieldInfo::TransientTimeSkip).toDouble());
    // linearity
    cmbLinearityType->setCurrentIndex(cmbLinearityType->findData(m_fieldInfo->linearityType()));
    txtNonlinearResidual->setValue(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble());
    chkNonlinearResidual->setChecked(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() > 0);
    chkNonlinearRelativeChangeOfSolutions->setChecked(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() > 0);
    txtNonlinearRelativeChangeOfSolutions->setValue(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    cmbNonlinearDampingType->setCurrentIndex(cmbNonlinearDampingType->findData((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt()));
    txtNonlinearDampingCoeff->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNewtonSufficientImprovementFactorForJacobianReuse->setValue(m_fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble());
    txtNonlinearDampingRatioForFactorDecrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble());
    txtNewtonMaximumStepsWithReusedJacobian->setValue(m_fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt());
    txtNonlinearDampingStepsForFactorIncrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt());
    chkNewtonReuseJacobian->setChecked((m_fieldInfo->value(FieldInfo::NewtonReuseJacobian)).toBool());
    chkPicardAndersonAcceleration->setChecked(m_fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool());
    txtPicardAndersonBeta->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble());
    txtPicardAndersonNumberOfLastVectors->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt());
    // linear solver
    txtIterLinearSolverToleranceAbsolute->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
    txtIterLinearSolverIters->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());
    cmbIterLinearSolverDealIIMethod->setCurrentIndex((IterSolverDealII) cmbIterLinearSolverDealIIMethod->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()));
    cmbIterLinearSolverDealIIPreconditioner->setCurrentIndex((PreconditionerDealII) cmbIterLinearSolverDealIIPreconditioner->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt()));
    // external solver
    cmbExternalLinearSolverCommand->setCurrentIndex(cmbExternalLinearSolverCommand->findData(m_fieldInfo->value(FieldInfo::LinearSolverExternalName).toString()));
    txtExternalLinearSolverCommandEnvironment->setText(m_fieldInfo->value(FieldInfo::LinearSolverExternalCommandEnvironment).toString());
    txtExternalLinearSolverCommandParameters->setText(m_fieldInfo->value(FieldInfo::LinearSolverExternalCommandParameters).toString());

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

bool FieldWidget::save()
{
    // analysis type
    m_fieldInfo->setAnalysisType((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt());
    // adaptivity
    m_fieldInfo->setAdaptivityType((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivitySteps, txtAdaptivitySteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTolerance, chkAdaptivityTolerance->isChecked() ? txtAdaptivityTolerance->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::AdaptivityFinePercentage, txtAdaptivityFineFraction->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityCoarsePercentage, txtAdaptivityCoarseFraction->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityEstimator, (AdaptivityEstimator) cmbAdaptivityEstimator->itemData(cmbAdaptivityEstimator->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityStrategy, (AdaptivityStrategy) cmbAdaptivityStrategy->itemData(cmbAdaptivityStrategy->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityStrategyHP, (AdaptivityStrategyHP) cmbAdaptivityStrategyHP->itemData(cmbAdaptivityStrategyHP->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientBackSteps, txtAdaptivityBackSteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientRedoneEach, txtAdaptivityRedoneEach->value());
    // matrix solver
    m_fieldInfo->setMatrixSolver((MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt());
    //mesh
    m_fieldInfo->setValue(FieldInfo::SpaceNumberOfRefinements, txtNumberOfRefinements->value());
    m_fieldInfo->setValue(FieldInfo::SpacePolynomialOrder, txtPolynomialOrder->value());
    // transient
    m_fieldInfo->setValue(FieldInfo::TransientInitialCondition, txtTransientInitialCondition->value());
    m_fieldInfo->setValue(FieldInfo::TransientTimeSkip, txtTransientTimeSkip->value());
    // linearity
    m_fieldInfo->setLinearityType((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NonlinearResidualNorm, chkNonlinearResidual->isChecked() ? txtNonlinearResidual->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearRelativeChangeOfSolutions, chkNonlinearRelativeChangeOfSolutions->isChecked() ? txtNonlinearRelativeChangeOfSolutions->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingCoeff, txtNonlinearDampingCoeff->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingType, (DampingType) cmbNonlinearDampingType->itemData(cmbNonlinearDampingType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NewtonReuseJacobian, chkNewtonReuseJacobian->isChecked());
    m_fieldInfo->setValue(FieldInfo::NewtonJacobianReuseRatio, txtNewtonSufficientImprovementFactorForJacobianReuse->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingFactorDecreaseRatio, txtNonlinearDampingRatioForFactorDecrease->value());
    m_fieldInfo->setValue(FieldInfo::NewtonMaxStepsReuseJacobian, txtNewtonMaximumStepsWithReusedJacobian->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearStepsToIncreaseDampingFactor, txtNonlinearDampingStepsForFactorIncrease->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonAcceleration, chkPicardAndersonAcceleration->isChecked());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonBeta, txtPicardAndersonBeta->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonNumberOfLastVectors, txtPicardAndersonNumberOfLastVectors->value());
    // linear solver
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterToleranceAbsolute, txtIterLinearSolverToleranceAbsolute->value());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterIters, txtIterLinearSolverIters->value());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIMethod, cmbIterLinearSolverDealIIMethod->itemData(cmbIterLinearSolverDealIIMethod->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIPreconditioner, cmbIterLinearSolverDealIIPreconditioner->itemData(cmbIterLinearSolverDealIIPreconditioner->currentIndex()).toInt());
    // external solver
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalName, cmbExternalLinearSolverCommand->itemData(cmbExternalLinearSolverCommand->currentIndex()).toString());
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalCommandEnvironment, txtExternalLinearSolverCommandEnvironment->text());
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalCommandParameters, txtExternalLinearSolverCommandParameters->text());

    return true;
}

void FieldWidget::refresh()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

FieldInfo *FieldWidget::fieldInfo()
{
    return m_fieldInfo;
}

void FieldWidget::doAnalysisTypeClicked()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void FieldWidget::doAnalysisTypeChanged(int index)
{
    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();

    // initial condition
    txtTransientInitialCondition->setEnabled(analysisType == AnalysisType_Transient);

    // time steps skip
    bool otherFieldIsTransient = false;
    foreach (FieldInfo* otherFieldInfo, Agros2D::preprocessor()->fieldInfos())
        if (otherFieldInfo->analysisType() == AnalysisType_Transient && otherFieldInfo->fieldId() != m_fieldInfo->fieldId())
            otherFieldIsTransient = true;

    txtTransientTimeSkip->setEnabled(!(m_fieldInfo->analysisType() == AnalysisType_Transient) && otherFieldIsTransient);

    LinearityType previousLinearityType = (LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt();
    cmbLinearityType->clear();

    int idx = 0, nextIndex = 0;
    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt()))
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);

        if (linearityType == previousLinearityType)
            nextIndex = idx;
        idx++;
    }
    cmbLinearityType->setCurrentIndex(nextIndex);

    doShowEquation();
    doAdaptivityChanged(cmbAdaptivityType->currentIndex());
}

void FieldWidget::doShowEquation()
{
    // equationLaTeX->setLatex(m_fieldInfo->equation());
    QPixmap pixmap(QString("%1/resources/images/equations/%2_equation_%3.png").
                   arg(datadir()).
                   arg(m_fieldInfo->fieldId()).
                   arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));

    equationImage->setPixmap(pixmap);
    equationImage->setMask(pixmap.mask());
}

void FieldWidget::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    chkAdaptivityTolerance->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    txtAdaptivityTolerance->setEnabled(((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None) && chkAdaptivityTolerance->isChecked());
    cmbAdaptivityEstimator->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    cmbAdaptivityStrategy->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    cmbAdaptivityStrategyHP->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() == AdaptivityMethod_HP);

    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    txtAdaptivityBackSteps->setEnabled(Agros2D::preprocessor()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    txtAdaptivityRedoneEach->setEnabled(Agros2D::preprocessor()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);

    doAdaptivityStrategyChanged(cmbAdaptivityStrategy->currentIndex());
}

void FieldWidget::doAdaptivityStrategyChanged(int index)
{
    bool enabled = (((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt() != AdaptivityMethod_None) &&
                    (((AdaptivityStrategy) cmbAdaptivityStrategy->itemData(index).toInt() == AdaptivityStrategy_FixedFractionOfCells) ||
                     ((AdaptivityStrategy) cmbAdaptivityStrategy->itemData(index).toInt() == AdaptivityStrategy_FixedFractionOfTotalError)));

    txtAdaptivityFineFraction->setEnabled(enabled);
    txtAdaptivityCoarseFraction->setEnabled(enabled);
}

void FieldWidget::doAdaptivityStrategyHPChanged(int index)
{
}

void FieldWidget::doLinearityTypeChanged(int index)
{
    chkNonlinearResidual->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
    chkNonlinearRelativeChangeOfSolutions->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());

    cmbNonlinearDampingType->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    doNonlinearDampingChanged(cmbNonlinearDampingType->currentIndex());

    txtNewtonMaximumStepsWithReusedJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    chkNewtonReuseJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    doNewtonReuseJacobian(chkNewtonReuseJacobian->isChecked());

    chkPicardAndersonAcceleration->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    doPicardAndersonChanged(chkPicardAndersonAcceleration->checkState());
}

void FieldWidget::doLinearSolverChanged(int index)
{
    MatrixSolverType solverType = (MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt();
    bool isIterative = isMatrixSolverIterative(solverType);

    txtIterLinearSolverToleranceAbsolute->setEnabled(isIterative);
    txtIterLinearSolverIters->setEnabled(isIterative);
    cmbIterLinearSolverDealIIMethod->setEnabled(solverType == SOLVER_DEALII);
    cmbIterLinearSolverDealIIPreconditioner->setEnabled(solverType == SOLVER_DEALII);
    cmbExternalLinearSolverCommand->setEnabled(solverType == SOLVER_EXTERNAL);
    txtExternalLinearSolverCommandEnvironment->setEnabled(solverType == SOLVER_EXTERNAL);
    txtExternalLinearSolverCommandParameters->setEnabled(solverType == SOLVER_EXTERNAL);
    // lblExternalLinearSolverHint->clear();
}

void FieldWidget::doNonlinearDampingChanged(int index)
{
    txtNonlinearDampingCoeff->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                         ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() != DampingType_Off));
    txtNonlinearDampingStepsForFactorIncrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));
    txtNonlinearDampingRatioForFactorDecrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));

}

void FieldWidget::doNewtonReuseJacobian(bool checked)
{
    txtNewtonMaximumStepsWithReusedJacobian->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                        (chkNewtonReuseJacobian->isChecked()));
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                                     (chkNewtonReuseJacobian->isChecked()));
}

void FieldWidget::doPicardAndersonChanged(int index)
{
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
}

void FieldWidget::doAdaptivityTolerance(int state)
{
    txtAdaptivityTolerance->setEnabled(((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt() != AdaptivityMethod_None) && chkAdaptivityTolerance->isChecked());
}

void FieldWidget::doNonlinearResidual(int state)
{
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
}

void FieldWidget::doNonlinearRelativeChangeOfSolutions(int state)
{
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());
}

void FieldWidget::doExternalSolverChanged(int index)
{
    if (cmbExternalLinearSolverCommand->currentIndex() != -1)
    {
        QString fileName = QFileInfo(cmbExternalLinearSolverCommand->itemData(cmbExternalLinearSolverCommand->currentIndex()).toString()).fileName();

        // read command parameters
        QFile f(QString("%1/libs/%2").arg(datadir()).arg(fileName));
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&f);

            QStringList commandContent = in.readAll().split("\n");

            // hint at third line
            lblExternalLinearSolverHint->setText(commandContent.at(2));
        }
    }
}

// ********************************************************************************************

FieldDialog::FieldDialog(FieldInfo *fieldInfo, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(fieldInfo->name());

    fieldWidget = new FieldWidget(fieldInfo, this);

    // dialog buttons
    QPushButton *btnDeleteField = new QPushButton(tr("Delete field"));
    btnDeleteField->setDefault(false);
    btnDeleteField->setEnabled(Agros2D::preprocessor()->hasField(fieldInfo->fieldId()));
    connect(btnDeleteField, SIGNAL(clicked()), this, SLOT(deleteField()));

#ifdef QT_DEBUG
    QPushButton *btnModuleEditor = new QPushButton(tr("Module editor"));
    btnModuleEditor->setDefault(false);
    btnModuleEditor->setVisible(false);
    connect(btnModuleEditor, SIGNAL(clicked()), this, SLOT(moduleEditor()));
#endif

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnDeleteField, QDialogButtonBox::ActionRole);
#ifdef QT_DEBUG
    // buttonBox->addButton(btnModuleEditor, QDialogButtonBox::ActionRole);
#endif
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(fieldWidget);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

FieldDialog::~FieldDialog()
{
}

void FieldDialog::doAccept()
{
    fieldWidget->save();
    accept();
}

void FieldDialog::deleteField()
{
    if (QMessageBox::question(this, tr("Delete"), tr("Physical field '%1' will be pernamently deleted. Are you sure?").
                              arg(fieldWidget->fieldInfo()->name()), tr("&Yes"), tr("&No")) == 0)
    {
        Agros2D::preprocessor()->removeField(fieldWidget->fieldInfo());
        accept();
    }
}

void FieldDialog::moduleEditor()
{
    // ModuleDialog moduleDialog(fieldWidget->fieldInfo()->fieldId(), this);
    // moduleDialog.exec();
}

// ********************************************************************************************

FieldsToobar::FieldsToobar(QWidget *parent) : QWidget(parent)
{
    createControls();

    connect(Agros2D::preprocessor(), SIGNAL(fieldsChanged()), this, SLOT(refresh()));
    connect(Agros2D::preprocessor()->scene(), SIGNAL(invalidated()), this, SLOT(refresh()));

    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));

    refresh();
}

void FieldsToobar::createControls()
{
    QButtonGroup *buttonBar = new QButtonGroup(this);
    connect(buttonBar, SIGNAL(buttonClicked(int)), this, SLOT(fieldDialog(int)));

    QGridLayout *layoutFields = new QGridLayout();

    // buttons and labels
    int numberOfFields = Module::availableModules().count();

    int row = 0;
    for (int i = 0; i < numberOfFields; i++)
    {
        QLabel *label = new QLabel(this);
        label->setVisible(false);
        label->setStyleSheet("QLabel { font-size: 8.5pt; }");

        QToolButton *button = new QToolButton(this);
        button->setVisible(false);
        button->setMinimumWidth(columnMinimumWidth());
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setStyleSheet("QToolButton { font-size: 8pt; }");
        button->setIconSize(QSize(36, 36));

        // add to button bar
        buttonBar->addButton(button, row);

        // add to layout
        layoutFields->addWidget(button, row, 0);
        layoutFields->addWidget(label, row, 1);

        // add to lists
        m_buttons.append(button);
        m_labels.append(label);

        row++;
    }

    // dialog buttons
    QPushButton *btnAddField = new QPushButton(tr("Add field")); // icon("tabadd")
    connect(btnAddField, SIGNAL(clicked()), SLOT(addField()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnAddField);

    QVBoxLayout *layoutToolBar = new QVBoxLayout();
    layoutToolBar->setContentsMargins(0, 1, 0, 1);
    // layoutToolBar->addWidget(tlbFields);
    layoutToolBar->addLayout(layoutFields);
    layoutToolBar->addLayout(layoutButtons);
    layoutToolBar->addStretch();

    setLayout(layoutToolBar);
}

void FieldsToobar::refresh()
{
    // disable during script running
    if (currentPythonEngineAgros()->isScriptRunning())
        return;

    setUpdatesEnabled(false);

    // fields
    int row = 0;
    foreach (FieldInfo *fieldInfo, Agros2D::preprocessor()->fieldInfos())
    {
        QString hint = tr("<table>"
                          "<tr><td><b>Analysis:</b></td><td>%1</td></tr>"
                          "<tr><td><b>Solver:</b></td><td>%3</td></tr>"
                          "<tr><td><b>Adaptivity:</b></td><td>%2</td></tr>"
                          "<tr><td><b>Number of ref. / order:</b></td><td>%4 / %5</td></tr>"
                          "</table>")
                .arg(analysisTypeString(fieldInfo->analysisType()))
                .arg(adaptivityTypeString(fieldInfo->adaptivityType()))
                .arg(linearityTypeString(fieldInfo->linearityType()))
                .arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt())
                .arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());

        m_labels[row]->setText(hint);
        m_labels[row]->setVisible(true);

        m_buttons[row]->setText(fieldInfo->name());
        m_buttons[row]->setIcon(icon("fields/" + fieldInfo->fieldId()));
        m_buttons[row]->setVisible(true);

        row++;
    }
    for (int i = row; i < m_buttons.count(); i++)
    {
        m_labels[i]->setVisible(false);
        m_buttons[i]->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void FieldsToobar::fieldDialog(int index)
{
    FieldInfo *fieldInfo = Agros2D::preprocessor()->fieldInfos().values().at(index);
    if (fieldInfo)
    {
        FieldDialog fieldDialog(fieldInfo, this);
        if (fieldDialog.exec() == QDialog::Accepted)
        {
            refresh();
            emit changed();
        }
    }
}

void FieldsToobar::addField()
{
    // select field dialog
    FieldSelectDialog dialog(Agros2D::preprocessor()->fieldInfos().keys(), this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        // add field
        FieldInfo *fieldInfo = new FieldInfo(dialog.selectedFieldId());

        FieldDialog fieldDialog(fieldInfo, this);
        if (fieldDialog.exec() == QDialog::Accepted)
        {
            Agros2D::preprocessor()->addField(fieldInfo);

            refresh();
            emit changed();
        }
        else
        {
            delete fieldInfo;
        }
    }
}

// ********************************************************************************************

CouplingsWidget::CouplingsWidget(QWidget *parent) : QWidget(parent)
{
    Agros2D::preprocessor()->synchronizeCouplings();

    createContent();

    connect(Agros2D::preprocessor(), SIGNAL(fieldsChanged()), this, SLOT(refresh()));
}

void CouplingsWidget::createContent()
{
    QGridLayout *layoutTable;layoutTable = new QGridLayout();
    layoutTable->setContentsMargins(0, 0, 0, 0);
    layoutTable->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTable->setColumnStretch(1, 1);

    for (int i = 0; i < couplingList()->availableCouplings().count(); i++)
    {
        QLabel *label = new QLabel(this);
        label->setVisible(false);

        QComboBox *combo = new QComboBox(this);
        combo->setVisible(false);

        foreach (QString cp, couplingTypeStringKeys())
            combo->addItem(couplingTypeString(couplingTypeFromStringKey(cp)), couplingTypeFromStringKey(cp));

        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(itemChanged(int)));

        layoutTable->addWidget(label, i, 0);
        layoutTable->addWidget(combo, i, 1);

        m_labels.append(label);
        m_comboBoxes.append(combo);
    }

    setLayout(layoutTable);
}

void CouplingsWidget::save()
{
    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::preprocessor()->couplingInfos())
    {
        couplingInfo->setCouplingType((CouplingType) m_comboBoxes[row]->itemData(m_comboBoxes[row]->currentIndex()).toInt());

        row++;
    }
}

void CouplingsWidget::refresh()
{
    Agros2D::preprocessor()->synchronizeCouplings();

    setUpdatesEnabled(false);

    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::preprocessor()->couplingInfos())
    {
        m_comboBoxes[row]->blockSignals(true);
        m_comboBoxes[row]->setUpdatesEnabled(false);
        m_comboBoxes[row]->setCurrentIndex(m_comboBoxes[row]->findData(couplingInfo->couplingType()));
        m_comboBoxes[row]->setUpdatesEnabled(true);
        m_comboBoxes[row]->setVisible(true);
        m_comboBoxes[row]->blockSignals(false);

        m_labels[row]->setText(couplingInfo->name());
        m_labels[row]->setVisible(true);

        row++;
    }
    for (int i = row; i < m_comboBoxes.count(); i++)
    {
        m_comboBoxes[i]->setVisible(false);
        m_labels[i]->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void CouplingsWidget::itemChanged(int index)
{
    // qDebug() << "void CouplingsWidget::itemChanged(int index)";
    emit changed();
}

// ********************************************************************************************

ProblemWidget::ProblemWidget(QWidget *parent) : QWidget(parent)
{
    createActions();
    createControls();

    updateControls();

    // global signals
    connect(Agros2D::preprocessor()->scene(), SIGNAL(invalidated()), this, SLOT(updateControls()));
    connect(Agros2D::preprocessor(), SIGNAL(fieldsChanged()), this, SLOT(updateControls()));
    connect(fieldsToolbar, SIGNAL(changed()), this, SLOT(updateControls()));

    // resend signal
    connect(fieldsToolbar, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(couplingsWidget, SIGNAL(changed()), this, SIGNAL(changed()));

    setMinimumSize(sizeHint());
}

void ProblemWidget::createActions()
{
    actProperties = new QAction(icon("document-properties"), tr("Properties"), this);
    actProperties->setShortcut(tr("Ctrl+1"));
    actProperties->setCheckable(true);
}

void ProblemWidget::createControls()
{
    // main toolbar
    toolBar = new QToolBar();

    // fields toolbar
    fieldsToolbar = new FieldsToobar();
    QVBoxLayout *layoutFields = new QVBoxLayout();
    layoutFields->addWidget(fieldsToolbar);
    layoutFields->addStretch();

    QGroupBox *grpFieldsToolbar = new QGroupBox(tr("Physical fields"));
    grpFieldsToolbar->setLayout(layoutFields);

    // problem
    cmbCoordinateType = new QComboBox();
    // mesh type
    cmbMeshType = new QComboBox();

    // general
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Coordinate type:")), 0, 0);
    layoutGeneral->addWidget(cmbCoordinateType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Mesh type:")), 1, 0);
    layoutGeneral->addWidget(cmbMeshType, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // harmonic
    txtFrequency = new ValueLineEdit();
    txtFrequency->setMinimum(0.0);

    // harmonic analysis
    QGridLayout *layoutHarmonicAnalysis = new QGridLayout();
    layoutHarmonicAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutHarmonicAnalysis->addWidget(new QLabel(tr("Frequency (Hz):")), 0, 0);
    layoutHarmonicAnalysis->addWidget(txtFrequency, 0, 1);

    grpHarmonicAnalysis = new QGroupBox(tr("Harmonic analysis"));
    grpHarmonicAnalysis->setLayout(layoutHarmonicAnalysis);

    // transient
    cmbTransientMethod = new QComboBox();
    txtTransientOrder = new QSpinBox();
    txtTransientOrder->setMinimum(1);
    txtTransientOrder->setMaximum(3);
    txtTransientTimeTotal = new LineEditDouble(1.0);
    txtTransientTimeTotal->setBottom(0.0);
    lblTransientTimeTotal = new QLabel("Total time");
    txtTransientTolerance = new LineEditDouble(0.1);
    txtTransientTolerance->setBottom(0.0);
    chkTransientInitialStepSize = new QCheckBox(this);
    txtTransientInitialStepSize = new LineEditDouble(0.01);
    txtTransientInitialStepSize->setBottom(0);
    txtTransientSteps = new QSpinBox();
    txtTransientSteps->setMinimum(1);
    txtTransientSteps->setMaximum(10000);
    lblTransientTimeStep = new QLabel("0.0");
    lblTransientSteps = new QLabel(tr("Number of constant steps:"));

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(2, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Method:")), 0, 0, 1, 2);
    layoutTransientAnalysis->addWidget(cmbTransientMethod, 0, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Order:")), 1, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientOrder, 1, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Tolerance (%):")), 2, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTolerance, 2, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeTotal, 3, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTimeTotal, 3, 2);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 4, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientSteps, 4, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial time step:")), 5, 0);
    layoutTransientAnalysis->addWidget(chkTransientInitialStepSize, 5, 1, 1, 1, Qt::AlignRight);
    layoutTransientAnalysis->addWidget(txtTransientInitialStepSize, 5, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Constant time step:")), 6, 0, 1, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeStep, 6, 2);

    grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // fill combobox
    fillComboBox();

    // couplings
    couplingsWidget = new CouplingsWidget(this);
    connect(Agros2D::preprocessor(), SIGNAL(couplingsChanged()), couplingsWidget, SLOT(refresh()));
    connect(couplingsWidget, SIGNAL(changed()), couplingsWidget, SLOT(save()));

    QVBoxLayout *layoutCouplings = new QVBoxLayout();
    layoutCouplings->addWidget(couplingsWidget);

    grpCouplings = new QGroupBox(tr("Couplings"));
    grpCouplings->setLayout(layoutCouplings);

    // parameters
    trvParameters = new QTreeWidget(this);
    trvParameters->setMouseTracking(true);
    trvParameters->setColumnCount(2);
    trvParameters->setIndentation(15);
    trvParameters->setMinimumWidth(220);

    QStringList headers;
    headers << tr("Name") << tr("Value");
    trvParameters->setHeaderLabels(headers);

    connect(trvParameters, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(parameterChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    lblParametersError = new QLabel();

    txtParameterName = new QLineEdit();
    connect(txtParameterName, SIGNAL(textChanged(QString)), this, SLOT(parameterNameTextChanged(QString)));
    txtParameterValue = new LineEditDouble();

    QGridLayout *layoutParametersEdit = new QGridLayout();
    layoutParametersEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutParametersEdit->addWidget(txtParameterName, 0, 1);
    layoutParametersEdit->addWidget(new QLabel(tr("Value")), 1, 0);
    layoutParametersEdit->addWidget(txtParameterValue, 1, 1);

    QPalette palette = lblParametersError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblParametersError->setPalette(palette);
    lblParametersError->setVisible(false);

    btnParametersApply = new QPushButton(tr("Apply"));
    connect(btnParametersApply, SIGNAL(clicked()), this, SLOT(parametersApply()));
    btnParametersAdd = new QPushButton(tr("Add"));
    connect(btnParametersAdd, SIGNAL(clicked()), this, SLOT(parameterAdded()));
    btnParametersRemove = new QPushButton(tr("Remove"));
    connect(btnParametersRemove, SIGNAL(clicked()), this, SLOT(parameterRemoved()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addWidget(btnParametersAdd);
    layoutParametersButton->addWidget(btnParametersRemove);
    layoutParametersButton->addStretch(1);
    layoutParametersButton->addWidget(btnParametersApply);

    QVBoxLayout *layoutParametersWidget = new QVBoxLayout();
    layoutParametersWidget->addWidget(trvParameters);
    layoutParametersWidget->addWidget(lblParametersError);
    layoutParametersWidget->addLayout(layoutParametersEdit);
    layoutParametersWidget->addLayout(layoutParametersButton);

    QGroupBox *grpParameters = new QGroupBox(tr("Parameters"));
    grpParameters->setLayout(layoutParametersWidget);

    // problem widget
    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->setContentsMargins(0, 0, 0, 0);
    layoutArea->addWidget(grpGeneral);
    layoutArea->addWidget(grpFieldsToolbar);
    layoutArea->addWidget(grpCouplings);
    layoutArea->addWidget(grpHarmonicAnalysis);
    layoutArea->addWidget(grpTransientAnalysis);
    layoutArea->addWidget(grpParameters, 1);
    // layoutArea->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutArea);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layout= new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(toolBar);
    layout->addWidget(widgetArea);

    setLayout(layout);
}

void ProblemWidget::fillComboBox()
{
    cmbCoordinateType->clear();
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Planar), CoordinateType_Planar);
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Axisymmetric), CoordinateType_Axisymmetric);

    cmbMeshType->clear();
    foreach (QString meshType, meshTypeStringKeys())
        cmbMeshType->addItem(meshTypeString(meshTypeFromStringKey(meshType)), meshTypeFromStringKey(meshType));

    cmbTransientMethod->clear();
    foreach (QString timeStepType, timeStepMethodStringKeys())
        cmbTransientMethod->addItem(timeStepMethodString(timeStepMethodFromStringKey(timeStepType)), timeStepMethodFromStringKey(timeStepType));
}

void ProblemWidget::updateControls()
{
    // disconnect signals
    cmbCoordinateType->disconnect();
    cmbMeshType->disconnect();

    txtFrequency->disconnect();

    cmbTransientMethod->disconnect();
    txtTransientOrder->disconnect();
    txtTransientTimeTotal->disconnect();
    txtTransientTolerance->disconnect();
    chkTransientInitialStepSize->disconnect();
    txtTransientInitialStepSize->disconnect();
    txtTransientSteps->disconnect();

    // main
    cmbCoordinateType->setCurrentIndex(cmbCoordinateType->findData(Agros2D::preprocessor()->config()->coordinateType()));
    if (cmbCoordinateType->currentIndex() == -1)
        cmbCoordinateType->setCurrentIndex(0);

    // mesh type
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(Agros2D::preprocessor()->config()->meshType()));

    // harmonic magnetic
    grpHarmonicAnalysis->setVisible(Agros2D::preprocessor()->isHarmonic());
    txtFrequency->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::Frequency).value<Value>());

    // transient
    grpTransientAnalysis->setVisible(Agros2D::preprocessor()->isTransient());
    txtTransientSteps->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
    txtTransientTimeTotal->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeTotal).toDouble());
    txtTransientTolerance->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
    chkTransientInitialStepSize->setChecked(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0);
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    txtTransientInitialStepSize->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
    txtTransientOrder->setValue(Agros2D::preprocessor()->config()->value(ProblemConfig::TimeOrder).toInt());
    cmbTransientMethod->setCurrentIndex(cmbTransientMethod->findData((dealii::TimeStepping::runge_kutta_method) Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethod).toInt()));
    if (cmbTransientMethod->currentIndex() == -1)
        cmbTransientMethod->setCurrentIndex(0);

    lblTransientTimeTotal->setText(QString("Total time (s)"));

    // couplings
    fieldsToolbar->refresh();
    couplingsWidget->refresh();

    grpCouplings->setVisible(Agros2D::preprocessor()->couplingInfos().count() > 0);

    transientChanged();

    // reload parameters
    reloadParameters();

    // connect signals
    connect(cmbCoordinateType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
    connect(cmbMeshType, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    connect(txtFrequency, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    // transient
    connect(cmbTransientMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
    connect(txtTransientSteps, SIGNAL(valueChanged(int)), this, SLOT(saveConfig()));
    connect(txtTransientTimeTotal, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
    connect(txtTransientOrder, SIGNAL(valueChanged(int)), this, SLOT(saveConfig()));
    connect(txtTransientTolerance, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));
    connect(chkTransientInitialStepSize, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));
    connect(txtTransientInitialStepSize, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    connect(cmbTransientMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientSteps, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(textChanged(QString)), this, SLOT(transientChanged()));
    connect(txtTransientOrder, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));
}

void ProblemWidget::saveConfig()
{
    // save properties
    Agros2D::preprocessor()->config()->blockSignals(true);

    Agros2D::preprocessor()->config()->setCoordinateType((CoordinateType) cmbCoordinateType->itemData(cmbCoordinateType->currentIndex()).toInt());
    Agros2D::preprocessor()->config()->setMeshType((MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt());

    Agros2D::preprocessor()->config()->setValue(ProblemConfig::Frequency, txtFrequency->value());
    Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeMethod, (TimeStepMethod) cmbTransientMethod->itemData(cmbTransientMethod->currentIndex()).toInt());
    Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeOrder, txtTransientOrder->value());
    Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeMethodTolerance, txtTransientTolerance->value());
    Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeConstantTimeSteps, txtTransientSteps->value());
    Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeTotal, txtTransientTimeTotal->value());
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    if (chkTransientInitialStepSize->isChecked())
    {
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeInitialStepSize, txtTransientInitialStepSize->value());
    }
    else
    {
        txtTransientInitialStepSize->setValue(0.0);
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeInitialStepSize, 0.0);
    }

    // save couplings
    couplingsWidget->save();

    Agros2D::preprocessor()->config()->blockSignals(false);
    Agros2D::preprocessor()->config()->refresh();

    emit changed();
}

void ProblemWidget::transientChanged()
{
    lblTransientTimeStep->setText(QString("%1 s").arg(txtTransientTimeTotal->value() / txtTransientSteps->value()));
    lblTransientSteps->setText(tr("Approx. number of steps:"));

    switch ((TimeStepMethod) Agros2D::preprocessor()->config()->value(ProblemConfig::TimeMethod).toInt())
    {
    case TimeStepMethod_Fixed:
    {
        chkTransientInitialStepSize->setEnabled(false);
        txtTransientInitialStepSize->setEnabled(false);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);

        lblTransientSteps->setText(tr("Number of steps:"));
    }
        break;
    case TimeStepMethod_BDFTolerance:
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(true);
        txtTransientSteps->setEnabled(false);
    }
        break;
    case TimeStepMethod_BDFNumSteps:
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);
    }
        break;
    default:
        assert(0);
    }
}

void ProblemWidget::parameterSelect(const QString &key)
{
    txtParameterName->clear();
    txtParameterName->setEnabled(trvParameters->currentItem());
    txtParameterValue->clear();
    txtParameterValue->setEnabled(trvParameters->currentItem());
    btnParametersRemove->setEnabled(trvParameters->currentItem());
    btnParametersApply->setEnabled(trvParameters->currentItem());

    if (key.isEmpty())
        return;

    // select current item
    for (int i = 0; i < trvParameters->topLevelItemCount(); i++)
    {
        if (trvParameters->topLevelItem(i)->data(0, Qt::UserRole).toString() == key)
        {
            trvParameters->setCurrentItem(trvParameters->topLevelItem(i));
            parameterChanged(trvParameters->topLevelItem(i), nullptr);
            break;
        }
    }
}

void ProblemWidget::reloadParameters()
{
    trvParameters->blockSignals(true);

    // parameters
    QString selectedItem = "";
    if (trvParameters->currentItem())
        selectedItem = trvParameters->currentItem()->data(0, Qt::UserRole).toString();

    trvParameters->clear();
    ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    foreach (QString key, parameters.keys())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvParameters);
        item->setText(0, key);
        item->setText(1, QString::number(parameters[key]));
        item->setData(0, Qt::UserRole, key);

        trvParameters->addTopLevelItem(item);
    }

    trvParameters->blockSignals(false);

    // select current parameter
    parameterSelect(selectedItem);

    // if not selected -> select first
    if (trvParameters->topLevelItemCount() > 0 && !trvParameters->currentItem())
        trvParameters->setCurrentItem(trvParameters->topLevelItem(0));
}

void ProblemWidget::parametersApply()
{
    lblParametersError->clear();
    lblParametersError->setVisible(false);

    if (trvParameters->currentItem() && !txtParameterName->text().isEmpty())
    {
        // check parameter name
        try
        {
            Agros2D::preprocessor()->config()->checkParameterName(txtParameterName->text());
        }
        catch (AgrosException &e)
        {
            lblParametersError->setText(e.toString());
            lblParametersError->setVisible(true);

            btnParametersApply->setEnabled(false);
            return;
        }

        ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
        parameters[txtParameterName->text()] = txtParameterValue->value();
        // remove old parameter
        if (txtParameterName->text() != trvParameters->currentItem()->data(0, Qt::UserRole).toString())
            parameters.remove(trvParameters->currentItem()->data(0, Qt::UserRole).toString());

        if (Agros2D::preprocessor()->checkAndApplyParameters(parameters))
        {
            reloadParameters();
            parameterSelect(txtParameterName->text());

            Agros2D::preprocessor()->scene()->invalidate();
        }
    }
}

void ProblemWidget::parameterAdded()
{
    ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();

    QString key = "untitled";
    if (parameters.contains(key))
    {
        for (int i = 0; i < 100; i++)
        {
            if (!parameters.contains(key + QString::number(i)))
            {
                key += QString::number(i);
                break;
            }
        }
    }

    parameters[key] = 0.0;

    try
    {
        Agros2D::preprocessor()->config()->checkParameterName(key);
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::Parameters, parameters);

        reloadParameters();
        parameterSelect(key);

        Agros2D::preprocessor()->scene()->invalidate();
    }
    catch (AgrosException &e)
    {
        lblParametersError->setText(e.toString());
        lblParametersError->setVisible(true);

        btnParametersApply->setEnabled(false);
        return;
    }
}

void ProblemWidget::parameterRemoved()
{
    if (trvParameters->currentItem())
    {
        ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
        QString key = trvParameters->currentItem()->data(0, Qt::UserRole).toString();
        parameters.remove(key);

        if (Agros2D::preprocessor()->checkAndApplyParameters(parameters))
        {
            reloadParameters();

            Agros2D::preprocessor()->scene()->invalidate();
        }
    }
}

void ProblemWidget::parameterChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    txtParameterName->clear();
    txtParameterName->setEnabled(trvParameters->currentItem());
    txtParameterValue->clear();
    txtParameterValue->setEnabled(trvParameters->currentItem());
    btnParametersRemove->setEnabled(trvParameters->currentItem());
    btnParametersApply->setEnabled(trvParameters->currentItem());

    if (trvParameters->currentItem())
    {
        ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();

        QString key = trvParameters->currentItem()->data(0, Qt::UserRole).toString();

        txtParameterName->setText(key);
        txtParameterValue->setValue(parameters[key]);

        parameterNameTextChanged(key);
    }
}

void ProblemWidget::parameterNameTextChanged(const QString &str)
{
    lblParametersError->setVisible(false);
    btnParametersApply->setEnabled(true);

    if (trvParameters->currentItem())
    {
        ParametersType parameters = Agros2D::preprocessor()->config()->value(ProblemConfig::Parameters).value<ParametersType>();

        if (str.isEmpty())
        {
            btnParametersApply->setEnabled(false);
            return;
        }

        if ((str != trvParameters->currentItem()->data(0, Qt::UserRole).toString()) && parameters.contains(str))
        {
            lblParametersError->setText(QObject::tr("Key already exists."));
            lblParametersError->setVisible(true);

            btnParametersApply->setEnabled(false);
            return;
        }

        try
        {
            Agros2D::preprocessor()->config()->checkParameterName(str);
        }
        catch (AgrosException &e)
        {
            lblParametersError->setText(e.toString());
            lblParametersError->setVisible(true);

            btnParametersApply->setEnabled(false);
            return;
        }
    }
}
