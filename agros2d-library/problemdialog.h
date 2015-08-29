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

#ifndef PROBLEMDIALOG_H
#define PROBLEMDIALOG_H

#include "util.h"

class ProblemConfig;
class FieldInfo;
class CouplingInfo;

class LineEditDouble;
class ValueLineEdit;
class CollapsableGroupBoxButton;
class ScriptEditor;

class FieldInfo;

class ProblemWidget;

class FieldSelectDialog : public QDialog
{
    Q_OBJECT
public:
    FieldSelectDialog(QList<QString> fields, QWidget *parent);

    int showDialog();
    inline QString selectedFieldId() { return m_selectedFieldId; }

private slots:
    void doAccept();
    void doReject();

    void doItemSelected(QListWidgetItem *item);
    void doItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *lstFields;
    QString m_selectedFieldId;

    QDialogButtonBox *buttonBox;
};

class CouplingsWidget : public QWidget
{
    Q_OBJECT
public:
    CouplingsWidget(QWidget *parent);

    void createContent();

signals:
    void changed();

public slots:
    void save();
    void refresh();
    void itemChanged(int index);

private:
    void createComboBoxes();

    QList<QComboBox *> m_comboBoxes;
    QList<QLabel *> m_labels;
};

class FieldWidget : public QWidget
{
    Q_OBJECT
public:
    FieldWidget(FieldInfo *m_fieldInfo, QWidget *parent);

    void createContent();
    QWidget *createSolverWidget();
    QWidget *createAdaptivityWidget();
    QWidget *createTransientAnalysisWidget();
    QWidget *createLinearSolverWidget();

    void load();
    bool save();

    FieldInfo *fieldInfo();
    void refresh();

private:
    FieldInfo *m_fieldInfo;

    // main
    QComboBox *cmbAnalysisType;

    // adaptivity
    QComboBox *cmbAdaptivityType;
    QSpinBox *txtAdaptivitySteps;
    QCheckBox *chkAdaptivityTolerance;
    LineEditDouble *txtAdaptivityTolerance;
    QSpinBox *txtAdaptivityFineFraction;
    QSpinBox *txtAdaptivityCoarseFraction;
    QComboBox *cmbAdaptivityEstimator;
    QComboBox *cmbAdaptivityStrategy;
    QComboBox *cmbAdaptivityStrategyHP;
    QSpinBox *txtAdaptivityBackSteps;
    QSpinBox *txtAdaptivityRedoneEach;

    QComboBox *cmbLinearityType;
    QComboBox *cmbLinearSolver;

    // mesh
    QSpinBox *txtNumberOfRefinements;
    QSpinBox *txtPolynomialOrder;

    // linearity
    QCheckBox *chkNonlinearResidual;
    LineEditDouble *txtNonlinearResidual;
    QCheckBox *chkNonlinearRelativeChangeOfSolutions;
    LineEditDouble *txtNonlinearRelativeChangeOfSolutions;
    QComboBox *cmbNonlinearDampingType;
    LineEditDouble *txtNonlinearDampingCoeff;
    QSpinBox *txtNonlinearDampingStepsForFactorIncrease;
    LineEditDouble *txtNonlinearDampingRatioForFactorDecrease;

    // Newton
    QCheckBox *chkNewtonReuseJacobian;
    LineEditDouble *txtNewtonSufficientImprovementFactorForJacobianReuse;
    QSpinBox *txtNewtonMaximumStepsWithReusedJacobian;

    // Picard
    QCheckBox *chkPicardAndersonAcceleration;
    LineEditDouble *txtPicardAndersonBeta;
    QSpinBox *txtPicardAndersonNumberOfLastVectors;

    // transient
    LineEditDouble *txtTransientInitialCondition;
    LineEditDouble *txtTransientTimeSkip;

    // linear solver
    LineEditDouble *txtIterLinearSolverToleranceAbsolute;
    QSpinBox *txtIterLinearSolverIters;
    QComboBox *cmbIterLinearSolverDealIIMethod;
    QComboBox *cmbIterLinearSolverDealIIPreconditioner;
    QComboBox *cmbIterLinearSolverPARALUTIONMethod;
    QComboBox *cmbIterLinearSolverPARALUTIONPreconditioner;
    QCheckBox *chkIterLinearSolverPARALUTIONDoublePrecision;

    // equation
    // LaTeXViewer *equationLaTeX;
    QLabel *equationImage;

    void fillComboBox();

private slots:
    void doAnalysisTypeChanged(int index = -1);
    void doAnalysisTypeClicked();
    void doAdaptivityChanged(int index);
    void doAdaptivityStrategyChanged(int index);
    void doAdaptivityStrategyHPChanged(int index);
    void doLinearityTypeChanged(int index);
    void doLinearSolverChanged(int index);

    void doNonlinearDampingChanged(int index);
    void doNewtonReuseJacobian(bool checked);
    void doPicardAndersonChanged(int index);
    void doShowEquation();

    void doAdaptivityTolerance(int state);
    void doNonlinearResidual(int state);
    void doNonlinearRelativeChangeOfSolutions(int state);
};

class FieldDialog : public QDialog
{
    Q_OBJECT
public:
    FieldDialog(FieldInfo *fieldInfo, QWidget *parent = 0);
    ~FieldDialog();

private:
    FieldWidget *fieldWidget;

private slots:
    void doAccept();
    void deleteField();
    void moduleEditor();
};

class FieldsToobar: public QWidget
{
    Q_OBJECT

public:
    FieldsToobar(QWidget *parent = 0);

signals:
    void changed();

public slots:
    void refresh();

private:
    QList<QToolButton *> m_buttons;
    QList<QLabel *> m_labels;

    void createControls();

private slots:
    void fieldDialog(int index);
    void addField();
};

class ProblemWidget: public QWidget
{
    Q_OBJECT
public:
    ProblemWidget(QWidget *parent = 0);

    QAction *actProperties;
    QToolBar *toolBar;

signals:
    void changed();

public slots:
    void updateControls();

private:
    QDialogButtonBox *buttonBox;

    FieldsToobar *fieldsToolbar;
    CouplingsWidget *couplingsWidget;

    QComboBox *cmbCoordinateType;
    QComboBox *cmbMeshType;

    // harmonic
    QGroupBox *grpHarmonicAnalysis;
    ValueLineEdit *txtFrequency;

    // transient
    QGroupBox *grpTransientAnalysis;
    LineEditDouble *txtTransientTimeTotal;
    QLabel* lblTransientSteps;
    QSpinBox *txtTransientSteps;
    LineEditDouble *txtTransientTolerance;
    QCheckBox *chkTransientInitialStepSize;
    LineEditDouble *txtTransientInitialStepSize;
    QLabel *lblTransientTimeTotal;
    QSpinBox *txtTransientOrder;
    QComboBox *cmbTransientMethod;
    QLabel *lblTransientTimeStep;

    // couplings
    QGroupBox *grpCouplings;

    // startup script
    CollapsableGroupBoxButton *grpStartupScript;
    QWidget *widStartupScript;
    ScriptEditor *txtStartupScript;
    QLabel *lblStartupScriptError;

    void createActions();
    void createControls();

    void fillComboBox();

private slots:
    void transientChanged();
    void changedWithClear();
    void startupScriptCollapse(bool collapsed);
    void startupScriptChanged();
};

#endif // PROBLEMDIALOG_H
