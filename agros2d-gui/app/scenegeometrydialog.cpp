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

#include "scenegeometrydialog.h"

#include "util/global.h"

#include "gui/common.h"
#include "gui/valuelineedit.h"
#include "app/scenemarkerdialog.h"

#include "scene.h"
#include "solver/problem.h"

SceneBasicDialog::SceneBasicDialog(QWidget *parent, bool isNew) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->m_isNew = isNew;
    layout = new QVBoxLayout();
}

void SceneBasicDialog::createControls()
{
    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addLayout(createContent());
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void SceneBasicDialog::doAccept()
{
    if (save())
        accept();
}

void SceneBasicDialog::doReject()
{
    reject();
}

void SceneBasicDialog::evaluated(bool isError)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}

// node *************************************************************************************************************************************

SceneNodeDialog::SceneNodeDialog(SceneNode *node, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = node;

    setWindowTitle(tr("Node"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

SceneNodeDialog::~SceneNodeDialog()
{
    delete txtPointX;
    delete txtPointY;
}

QLayout* SceneNodeDialog::createContent()
{
    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointY, SIGNAL(editingFinished()), this, SLOT(doEditingFinished()));
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    lblDistance = new QLabel();
    lblAngle = new QLabel();

    // coordinates must be greater then or equal to 0 (axisymmetric case)
    if (m_object->scene()->parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    QFormLayout *layout = new QFormLayout();
    layout->addRow(m_object->scene()->parentProblem()->config()->labelX() + " (m):", txtPointX);
    layout->addRow(m_object->scene()->parentProblem()->config()->labelY() + " (m):", txtPointY);
    layout->addRow(tr("Distance:"), lblDistance);
    layout->addRow(tr("Angle:"), lblAngle);

    return layout;
}

bool SceneNodeDialog::load()
{
    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    txtPointX->setValue(sceneNode->pointValue().x());
    txtPointY->setValue(sceneNode->pointValue().y());

    doEditingFinished();

    return true;
}

bool SceneNodeDialog::save()
{
    if (!txtPointX->evaluate(false)) return false;
    if (!txtPointY->evaluate(false)) return false;

    SceneNode *sceneNode = dynamic_cast<SceneNode *>(m_object);

    PointValue point(txtPointX->value(), txtPointY->value());

    // check if node doesn't exists
    if (m_object->scene()->getNode(point.point()) && ((sceneNode->point() != point.point()) || m_isNew))
    {
        QMessageBox::warning(this, tr("Node"), tr("Node already exists."));
        return false;
    }

    if (!m_isNew)
    {
        if (sceneNode->point() != point.point())
        {
            undoStack()->push(new SceneNodeCommandEdit(sceneNode->pointValue(), point));
        }
    }

    sceneNode->setPointValue(point);

    return true;
}

void SceneNodeDialog::doEditingFinished()
{
    lblDistance->setText(QString("%1 m").arg(sqrt(txtPointX->number()*txtPointX->number() + txtPointY->number()*txtPointY->number())));
    lblAngle->setText(QString("%1 deg.").arg(
                          (sqrt(txtPointX->number()*txtPointX->number() + txtPointY->number()*txtPointY->number()) > EPS_ZERO)
                          ? atan2(txtPointY->number(), txtPointX->number()) / M_PI * 180.0 : 0.0));
}

// undo framework *******************************************************************************************************************

SceneNodeCommandAdd::SceneNodeCommandAdd(const PointValue &point, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point)
{
}

void SceneNodeCommandAdd::undo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        Agros2D::problem()->scene()->nodes->remove(node);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneNodeCommandAdd::redo()
{
    Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), m_point));
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandRemove::SceneNodeCommandRemove(const PointValue &point, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point)
{
}

void SceneNodeCommandRemove::undo()
{
    Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), m_point));
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandRemove::redo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        Agros2D::problem()->scene()->nodes->remove(node);
        Agros2D::problem()->scene()->invalidate();
    }
}

SceneNodeCommandEdit::SceneNodeCommandEdit(const PointValue &point, const PointValue &pointNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point), m_pointNew(pointNew)
{
}

void SceneNodeCommandEdit::undo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_pointNew.point());
    if (node)
    {
        node->setPointValue(m_point);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneNodeCommandEdit::redo()
{
    SceneNode *node = Agros2D::problem()->scene()->getNode(m_point.point());
    if (node)
    {
        node->setPointValue(m_pointNew);
        Agros2D::problem()->scene()->invalidate();
    }
}

SceneNodeCommandMoveMulti::SceneNodeCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points), m_pointsNew(pointsNew)
{
}

void SceneNodeCommandMoveMulti::moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo)
{
    assert(moveFrom.size() == moveTo.size());
    QList<SceneNode*> nodes;
    for (int i = 0; i < moveFrom.size(); i++)
    {
        Point point = moveFrom[i].point();
        SceneNode *node = Agros2D::problem()->scene()->getNode(point);
        nodes.push_back(node);
    }

    for (int i = 0; i < moveFrom.size(); i++)
    {
        ProblemBase *problem = moveTo[i].x().problem();
        Point point = moveTo[i].point();
        SceneNode *node = nodes[i];
        if (node)
        {
            node->setPointValue(PointValue(problem, point));
        }
    }
}

void SceneNodeCommandMoveMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    moveAll(m_pointsNew, m_points);

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandMoveMulti::redo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    moveAll(m_points, m_pointsNew);

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandAddMulti::SceneNodeCommandAddMulti(QList<PointValue> points, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points)
{
}

void SceneNodeCommandAddMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);
    foreach(PointValue point, m_points)
    {
        SceneNode *node = Agros2D::problem()->scene()->getNode(point.point());
        if (node)
        {
            Agros2D::problem()->scene()->nodes->remove(node);
        }
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandAddMulti::redo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    foreach(PointValue point, m_points)
    {
        Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), point));
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneNodeCommandRemoveMulti::SceneNodeCommandRemoveMulti(QList<PointValue> points, QUndoCommand *parent)
    : QUndoCommand(parent), m_nodePoints(points)
{
}

void SceneNodeCommandRemoveMulti::undo()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    // new nodes
    foreach(PointValue point, m_nodePoints)
    {
        Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), point));
    }

    // new edges
    for (int i = 0; i < m_edgePointStart.count(); i++)
    {
        SceneNode *nodeStart = Agros2D::problem()->scene()->getNode(m_edgePointStart[i]);
        SceneNode *nodeEnd = Agros2D::problem()->scene()->getNode(m_edgePointEnd[i]);
        assert(nodeStart && nodeEnd);
        SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_edgeAngle[i]);

        edge->addMarkersFromStrings(m_edgeMarkers[i]);

        // add edge to the list
        Agros2D::problem()->scene()->addFace(edge);
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneNodeCommandRemoveMulti::redo()
{
    m_edgePointStart.clear();
    m_edgePointEnd.clear();
    m_edgeAngle.clear();
    m_edgeMarkers.clear();

    Agros2D::problem()->scene()->stopInvalidating(true);

    foreach (PointValue point, m_nodePoints)
    {
        SceneNode *node = Agros2D::problem()->scene()->getNode(point.point());
        if (node)
        {
            QList<SceneFace *> connectedEdges = node->connectedEdges();
            foreach (SceneFace *edge, connectedEdges)
            {
                m_edgePointStart.append(edge->nodeStart()->point());
                m_edgePointEnd.append(edge->nodeEnd()->point());
                m_edgeMarkers.append(edge->markersKeys());
                m_edgeAngle.append(edge->angleValue());

                Agros2D::problem()->scene()->faces->remove(edge);
            }

            Agros2D::problem()->scene()->nodes->remove(node);
        }
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

// face *************************************************************************************************************************************

SceneFaceMarker::SceneFaceMarker(SceneFace *edge, FieldInfo *fieldInfo, QWidget *parent)
    : QGroupBox(parent), m_fieldInfo(fieldInfo), m_face(edge)

{
    setTitle(fieldInfo->name());

    cmbBoundary = new QComboBox();
    connect(cmbBoundary, SIGNAL(currentIndexChanged(int)), this, SLOT(doBoundaryChanged(int)));

    btnBoundary = new QPushButton(iconAwesome(fa::caretup), "");
    btnBoundary->setMaximumSize(btnBoundary->sizeHint());
    connect(btnBoundary, SIGNAL(clicked()), this, SLOT(doBoundaryClicked()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbBoundary, 1);
    layoutBoundary->addWidget(btnBoundary);

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Boundary condition:"), layoutBoundary);

    setLayout(layoutBoundaries);
}

void SceneFaceMarker::load()
{
    if (m_face->hasMarker(m_fieldInfo))
        cmbBoundary->setCurrentIndex(cmbBoundary->findData(m_face->marker(m_fieldInfo)->variant()));
}

bool SceneFaceMarker::save()
{
    m_face->addMarker(cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>());

    return true;
}

void SceneFaceMarker::fillComboBox()
{
    cmbBoundary->clear();

    // none marker
    cmbBoundary->addItem(m_face->scene()->boundaries->getNone(m_fieldInfo)->name(),
                         m_face->scene()->boundaries->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneBoundary *boundary, m_face->scene()->boundaries->filter(m_fieldInfo).items())
    {
        cmbBoundary->addItem(boundary->name(),
                             boundary->variant());
    }
}

void SceneFaceMarker::doBoundaryChanged(int index)
{
    btnBoundary->setEnabled(cmbBoundary->currentIndex() > 0);
}

void SceneFaceMarker::doBoundaryClicked()
{
    SceneBoundary *boundary = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    SceneBoundaryDialog *dialog = new SceneBoundaryDialog(boundary, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        cmbBoundary->setItemText(cmbBoundary->currentIndex(), boundary->name());
        m_face->scene()->invalidate();
    }
}

SceneFaceDialog::SceneFaceDialog(SceneFace *edge, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = edge;

    setWindowTitle(tr("Edge"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

QLayout* SceneFaceDialog::createContent()
{
    // layout
    QFormLayout *layout = new QFormLayout();

    foreach (FieldInfo *fieldInfo, m_object->scene()->parentProblem()->fieldInfos())
    {
        SceneFaceMarker *sceneEdge = new SceneFaceMarker(dynamic_cast<SceneFace *>(m_object), fieldInfo, this);
        layout->addRow(sceneEdge);

        m_faceMarkers.append(sceneEdge);
    }

    txtAngle = new ValueLineEdit();
    txtAngle->setMinimum(0.0);
    txtAngle->setMaximum(90.0);
    txtSegments = new QSpinBox();
    txtSegments->setMinimum(4);
    txtSegments->setMaximum(20);
    chkIsCurvilinear = new QCheckBox(tr("Curvilinear"));
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(angleChanged()));

    lblLength = new QLabel();

    cmbNodeStart = new QComboBox();
    cmbNodeEnd = new QComboBox();
    connect(cmbNodeStart, SIGNAL(currentIndexChanged(int)), this, SLOT(nodeChanged()));
    connect(cmbNodeEnd, SIGNAL(currentIndexChanged(int)), this, SLOT(nodeChanged()));

    // coordinates
    QFormLayout *layoutEdgeParameters = new QFormLayout();
    layoutEdgeParameters->addRow(tr("Angle (deg.):"), txtAngle);
    layoutEdgeParameters->addRow(tr("Segments (-):"), txtSegments);
    layoutEdgeParameters->addRow(tr(""), chkIsCurvilinear);
    layoutEdgeParameters->addRow(tr("Start node:"), cmbNodeStart);
    layoutEdgeParameters->addRow(tr("End node:"), cmbNodeEnd);
    layoutEdgeParameters->addRow(tr("Length:"), lblLength);

    QGroupBox *grpEdgeParameters = new QGroupBox(tr("Edge parameters"));
    grpEdgeParameters->setLayout(layoutEdgeParameters);

    layout->addRow(grpEdgeParameters);

    fillComboBox();

    QPushButton *btnSwap = new QPushButton();
    btnSwap->setText(tr("Swap direction"));
    connect(btnSwap, SIGNAL(clicked()), this, SLOT(swap()));
    buttonBox->addButton(btnSwap, QDialogButtonBox::ActionRole);

    return layout;
}

void SceneFaceDialog::fillComboBox()
{
    // markers
    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->fillComboBox();

    // start and end nodes
    cmbNodeStart->blockSignals(true);
    cmbNodeEnd->blockSignals(true);
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i < m_object->scene()->nodes->count(); i++)
    {
        cmbNodeStart->addItem(QString("%1 - [%2; %3]").
                              arg(i).
                              arg(m_object->scene()->nodes->at(i)->point().x, 0, 'e', 2).
                              arg(m_object->scene()->nodes->at(i)->point().y, 0, 'e', 2),
                              m_object->scene()->nodes->at(i)->variant());
        cmbNodeEnd->addItem(QString("%1 - [%2; %3]").
                            arg(i).
                            arg(m_object->scene()->nodes->at(i)->point().x, 0, 'e', 2).
                            arg(m_object->scene()->nodes->at(i)->point().y, 0, 'e', 2),
                            m_object->scene()->nodes->at(i)->variant());
    }
    cmbNodeStart->blockSignals(false);
    cmbNodeEnd->blockSignals(false);
}

bool SceneFaceDialog::load()
{
    SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

    cmbNodeStart->blockSignals(true);
    cmbNodeEnd->blockSignals(true);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart()->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd()->variant()));
    txtAngle->setValue(sceneEdge->angleValue());
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(angleChanged()));
    txtSegments->setValue(sceneEdge->segments());
    chkIsCurvilinear->setChecked(sceneEdge->isCurvilinear());

    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->load();

    nodeChanged();

    cmbNodeStart->blockSignals(false);
    cmbNodeEnd->blockSignals(false);

    return true;
}

bool SceneFaceDialog::save()
{
    if (!txtAngle->evaluate(false)) return false;

    SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

    if (cmbNodeStart->currentIndex() == cmbNodeEnd->currentIndex())
    {
        QMessageBox::warning(this, tr("Edge"), tr("Start and end node are same."));
        return false;
    }

    // check if edge doesn't exists
    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    SceneFace *edgeCheck = m_object->scene()->getFace(nodeStart->point(), nodeEnd->point());
    if ((edgeCheck) && ((sceneEdge != edgeCheck) || m_isNew))
    {
        QMessageBox::warning(this, tr("Edge"), tr("Edge already exists."));
        return false;
    }

    if (!m_isNew)
    {
        undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart()->pointValue(), sceneEdge->nodeEnd()->pointValue(),
                                                   nodeStart->pointValue(), nodeEnd->pointValue(),
                                                   sceneEdge->angleValue(),
                                                   txtAngle->value(),
                                                   sceneEdge->segments(),
                                                   txtSegments->value(),
                                                   sceneEdge->isCurvilinear(),
                                                   chkIsCurvilinear->checkState() == Qt::Checked));
    }

    sceneEdge->setNodeStart(nodeStart);
    sceneEdge->setNodeEnd(nodeEnd);
    sceneEdge->setAngleValue(txtAngle->value());
    sceneEdge->setSegments(txtSegments->value());
    sceneEdge->setCurvilinear(chkIsCurvilinear->checkState() == Qt::Checked);

    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->save();

    m_object->scene()->invalidate();
    return true;
}

void SceneFaceDialog::nodeChanged()
{
    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    if (nodeStart && nodeEnd)
    {
        SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

        SceneFace *edgeCheck = m_object->scene()->getFace(nodeStart->point(), nodeEnd->point());
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!((nodeStart == nodeEnd) || ((edgeCheck) && ((sceneEdge != edgeCheck) || m_isNew))));

        SceneFace edge(m_object->scene(), nodeStart, nodeEnd, Value(nullptr, txtAngle->number()), txtSegments->value(), chkIsCurvilinear->checkState() == Qt::Checked);
        lblLength->setText(QString("%1 m").arg(edge.length()));
    }
}

void SceneFaceDialog::swap()
{
    // swap nodes
    int startIndex = cmbNodeStart->currentIndex();
    cmbNodeStart->setCurrentIndex(cmbNodeEnd->currentIndex());
    cmbNodeEnd->setCurrentIndex(startIndex);
}

void SceneFaceDialog::angleChanged()
{
    txtSegments->setEnabled(txtAngle->value().isEvaluated() && txtAngle->value().number() > 0.0);
    chkIsCurvilinear->setEnabled(txtAngle->value().isEvaluated() && txtAngle->value().number() > 0.0);
}

SceneEdgeSelectDialog::SceneEdgeSelectDialog(MarkedSceneBasicContainer<SceneBoundary, SceneFace> edges, QWidget *parent)
    : QDialog(parent), m_edges(edges)
{
    setWindowTitle(tr("Edges"));

    // markers
    QFormLayout *layoutBoundaries = new QFormLayout();

    QGroupBox *grpBoundaries = new QGroupBox(tr("Boundary conditions"));
    grpBoundaries->setLayout(layoutBoundaries);

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QComboBox *cmbBoundary = new QComboBox();
        cmbBoundaries[fieldInfo] = cmbBoundary;

        layoutBoundaries->addRow(fieldInfo->name(), cmbBoundary);
    }

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpBoundaries);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    load();

    setMinimumSize(sizeHint());
}

void SceneEdgeSelectDialog::load()
{
    // markers
    foreach (FieldInfo *fieldInfo, cmbBoundaries.keys())
    {
        cmbBoundaries[fieldInfo]->clear();

        // none marker
        cmbBoundaries[fieldInfo]->addItem(Agros2D::problem()->scene()->boundaries->getNone(fieldInfo)->name(),
                                          Agros2D::problem()->scene()->boundaries->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneBoundary *boundary, Agros2D::problem()->scene()->boundaries->filter(fieldInfo).items())
            cmbBoundaries[fieldInfo]->addItem(boundary->name(),
                                              boundary->variant());
    }

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneBoundary* boundary = NULL;
        bool match = true;
        foreach (SceneFace *edge, m_edges.items())
        {
            if (boundary)
                match = match && (boundary == edge->marker(fieldInfo));
            else
                boundary = edge->marker(fieldInfo);
        }
        if (match)
            cmbBoundaries[fieldInfo]->setCurrentIndex(cmbBoundaries[fieldInfo]->findData(boundary->variant()));
        else
            cmbBoundaries[fieldInfo]->setCurrentIndex(-1);
    }
}

bool SceneEdgeSelectDialog::save()
{
    foreach (SceneFace* edge, m_edges.items())
    {
        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if (cmbBoundaries[fieldInfo]->currentIndex() != -1)
                edge->addMarker(cmbBoundaries[fieldInfo]->itemData(cmbBoundaries[fieldInfo]->currentIndex()).value<SceneBoundary *>());

        }
    }

    Agros2D::problem()->scene()->invalidate();
    return true;
}

void SceneEdgeSelectDialog::doAccept()
{
    if (save())
        accept();
}

void SceneEdgeSelectDialog::doReject()
{
    reject();
}

// **********************************************************************************************************************************

SceneFaceCommandAdd::SceneFaceCommandAdd(const PointValue &pointStart, const PointValue &pointEnd, const QMap<QString, QString> &markers,
                                         const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_pointStart(pointStart), m_pointEnd(pointEnd), m_markers(markers), m_angle(angle), m_segments(segments), m_isCurvilinear(isCurvilinear)
{
}

void SceneFaceCommandAdd::undo()
{
    Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStart.point(), m_pointEnd.point(), m_angle.number(), m_segments, m_isCurvilinear));
    Agros2D::problem()->scene()->invalidate();
}

void SceneFaceCommandAdd::redo()
{
    // new edge
    SceneNode *nodeStart = new SceneNode(Agros2D::problem()->scene(), m_pointStart);
    nodeStart = Agros2D::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros2D::problem()->scene(), m_pointEnd);
    nodeEnd = Agros2D::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angle, m_segments, m_isCurvilinear);

    edge->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addFace(edge);
    Agros2D::problem()->scene()->invalidate();
}

SceneEdgeCommandAddOrRemoveMulti::SceneEdgeCommandAddOrRemoveMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                                                                   QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QList<bool> isCurvilinear, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_pointStarts(pointStarts), m_pointEnds(pointEnds),
      m_markers(markers), m_angles(angles), m_segments(segments), m_isCurvilinear(isCurvilinear)
{
}

void SceneEdgeCommandAddOrRemoveMulti::remove()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    for(int i = 0; i < m_pointStarts.size(); i++)
    {
        Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStarts[i].point(), m_pointEnds[i].point(), m_angles[i].number(), m_segments[i], m_isCurvilinear[i]));
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneEdgeCommandAddOrRemoveMulti::add()
{
    Agros2D::problem()->scene()->stopInvalidating(true);
    for(int i = 0; i < m_pointStarts.size(); i++)
    {
        SceneNode *nodeStart = Agros2D::problem()->scene()->getNode(m_pointStarts[i].point());
        SceneNode *nodeEnd = Agros2D::problem()->scene()->getNode(m_pointEnds[i].point());
        assert(nodeStart && nodeEnd);
        if(nodeStart && nodeEnd)
        {
            SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angles[i], m_segments[i], m_isCurvilinear[i]);

            // if markers are not empty, we were deleting or copying "withMarkers = True"
            if(!m_markers.empty())
            {
                edge->addMarkersFromStrings(m_markers[i]);
            }
            // add edge to the list
            Agros2D::problem()->scene()->addFace(edge);
        }
    }
    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneFaceCommandRemove::SceneFaceCommandRemove(const PointValue &pointStart, const PointValue &pointEnd, const QMap<QString, QString> &markers,
                                               const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_pointStart(pointStart), m_pointEnd(pointEnd), m_markers(markers), m_angle(angle), m_segments(segments), m_isCurvilinear(isCurvilinear)
{
}

void SceneFaceCommandRemove::undo()
{
    // new edge
    SceneNode *nodeStart = new SceneNode(Agros2D::problem()->scene(), m_pointStart);
    nodeStart = Agros2D::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros2D::problem()->scene(), m_pointEnd);
    nodeEnd = Agros2D::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angle, m_segments, m_isCurvilinear);

    edge->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addFace(edge);
    Agros2D::problem()->scene()->invalidate();
}

void SceneFaceCommandRemove::redo()
{
    Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStart.point(), m_pointEnd.point(), m_angle.number(), m_segments, m_isCurvilinear));
    Agros2D::problem()->scene()->invalidate();
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const PointValue &pointStart, const PointValue &pointEnd, const PointValue &pointStartNew, const PointValue &pointEndNew,
                                           const Value &angle, const Value &angleNew, int segments, int segmentsNew, bool isCurvilinear, bool isCurvilinearNew, QUndoCommand *parent)
    : QUndoCommand(parent),
      m_pointStart(pointStart), m_pointEnd(pointEnd), m_pointStartNew(pointStartNew), m_pointEndNew(pointEndNew),
      m_angle(angle), m_angleNew(angleNew), m_segments(segments), m_segmentsNew(segmentsNew),
      m_isCurvilinear(isCurvilinear), m_isCurvilinearNew(isCurvilinearNew)
{
}

void SceneEdgeCommandEdit::undo()
{
    SceneFace *edge = Agros2D::problem()->scene()->getFace(m_pointStartNew.point(), m_pointEndNew.point(), m_angleNew.number(), m_segmentsNew, m_isCurvilinearNew);
    if (edge)
    {
        edge->setNodeStart(Agros2D::problem()->scene()->getNode(m_pointStart.point()));
        edge->setNodeEnd(Agros2D::problem()->scene()->getNode(m_pointEnd.point()));
        edge->setAngleValue(m_angle);
        edge->setSegments(m_segments);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneEdgeCommandEdit::redo()
{
    SceneFace *edge = Agros2D::problem()->scene()->getFace(m_pointStart.point(), m_pointEnd.point(), m_angle.number(), m_segments, m_isCurvilinear);
    if (edge)
    {
        edge->setNodeStart(Agros2D::problem()->scene()->getNode(m_pointStartNew.point()));
        edge->setNodeEnd(Agros2D::problem()->scene()->getNode(m_pointEndNew.point()));
        edge->setAngleValue(m_angleNew);
        edge->setSegments(m_segmentsNew);
        Agros2D::problem()->scene()->invalidate();
    }
}

// label *************************************************************************************************************************************

SceneLabelMarker::SceneLabelMarker(SceneLabel *label, FieldInfo *fieldInfo, QWidget *parent)
    : QGroupBox(parent), m_fieldInfo(fieldInfo), m_label(label)

{
    setTitle(fieldInfo->name());

    cmbMaterial = new QComboBox();
    connect(cmbMaterial, SIGNAL(currentIndexChanged(int)), this, SLOT(doMaterialChanged(int)));

    btnMaterial = new QPushButton(iconAwesome(fa::caretup), "");
    btnMaterial->setMaximumSize(btnMaterial->sizeHint());
    connect(btnMaterial, SIGNAL(clicked()), this, SLOT(doMaterialClicked()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbMaterial, 1);
    layoutBoundary->addWidget(btnMaterial);

    txtAreaRefinement = new QSpinBox(this);
    txtAreaRefinement->setMinimum(0);
    txtAreaRefinement->setMaximum(10);

    chkAreaRefinement = new QCheckBox();
    connect(chkAreaRefinement, SIGNAL(stateChanged(int)), this, SLOT(doAreaRefinement(int)));

    QHBoxLayout *layoutAreaRefinement = new QHBoxLayout();
    layoutAreaRefinement->addWidget(chkAreaRefinement);
    layoutAreaRefinement->addWidget(txtAreaRefinement);
    layoutAreaRefinement->addWidget(new QLabel(tr("Global refinement is %1.").arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt())));
    layoutAreaRefinement->addStretch();

    // order
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(10);

    chkPolynomialOrder = new QCheckBox();
    connect(chkPolynomialOrder, SIGNAL(stateChanged(int)), this, SLOT(doPolynomialOrder(int)));

    QHBoxLayout *layoutPolynomialOrder = new QHBoxLayout();
    layoutPolynomialOrder->addWidget(chkPolynomialOrder);
    layoutPolynomialOrder->addWidget(txtPolynomialOrder);
    layoutPolynomialOrder->addWidget(new QLabel(tr("Global order is %1.").arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt())));
    layoutPolynomialOrder->addStretch();

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Material:"), layoutBoundary);
    layoutBoundaries->addRow(tr("Area refinement (-):"), layoutAreaRefinement);
    layoutBoundaries->addRow(tr("Polynomial order (-):"), layoutPolynomialOrder);

    setLayout(layoutBoundaries);
}

void SceneLabelMarker::load()
{
    if (m_label->hasMarker(m_fieldInfo))
        cmbMaterial->setCurrentIndex(cmbMaterial->findData(m_label->marker(m_fieldInfo)->variant()));

    // refine area
    int refinement = m_fieldInfo->labelRefinement(m_label);
    chkAreaRefinement->setChecked(refinement > 0);
    txtAreaRefinement->setEnabled(chkAreaRefinement->isChecked());
    txtAreaRefinement->setValue(refinement);

    // polynomial order
    int order = m_fieldInfo->labelPolynomialOrder(m_label);
    chkPolynomialOrder->setChecked(order != m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
    txtPolynomialOrder->setValue(order);
}

bool SceneLabelMarker::save()
{
    m_label->addMarker(cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>());

    // refine area
    if (chkAreaRefinement->isChecked())
        m_fieldInfo->setLabelRefinement(m_label, txtAreaRefinement->text().toInt());
    else
        m_fieldInfo->removeLabelRefinement(m_label);

    // polynomial order
    if (chkPolynomialOrder->isChecked())
        m_fieldInfo->setLabelPolynomialOrder(m_label, txtPolynomialOrder->text().toInt());
    else
        m_fieldInfo->removeLabelPolynomialOrder(m_label);

    return true;
}

void SceneLabelMarker::doAreaRefinement(int state)
{
    txtAreaRefinement->setEnabled(chkAreaRefinement->isChecked());
}

void SceneLabelMarker::doPolynomialOrder(int state)
{
    txtPolynomialOrder->setEnabled(chkPolynomialOrder->isChecked());
}

void SceneLabelMarker::fillComboBox()
{
    cmbMaterial->clear();

    // none marker
    cmbMaterial->addItem(m_label->scene()->materials->getNone(m_fieldInfo)->name(),
                         m_label->scene()->materials->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneMaterial *material, m_label->scene()->materials->filter(m_fieldInfo).items())
    {
        cmbMaterial->addItem(material->name(),
                             material->variant());
    }
}

void SceneLabelMarker::doMaterialChanged(int index)
{
    btnMaterial->setEnabled(cmbMaterial->currentIndex() > 0);
    chkAreaRefinement->setEnabled(cmbMaterial->currentIndex() > 0);
    chkPolynomialOrder->setEnabled(cmbMaterial->currentIndex() > 0);
}

void SceneLabelMarker::doMaterialClicked()
{
    SceneMaterial *material = cmbMaterial->itemData(cmbMaterial->currentIndex()).value<SceneMaterial *>();
    SceneMaterialDialog *dialog = new SceneMaterialDialog(material, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        cmbMaterial->setItemText(cmbMaterial->currentIndex(), material->name());
        m_label->scene()->invalidate();
    }
}

SceneLabelDialog::SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = label;

    setWindowTitle(tr("Label"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

QLayout* SceneLabelDialog::createContent()
{
    // markers
    QFormLayout *layout = new QFormLayout();

    foreach (FieldInfo *fieldInfo, m_object->scene()->parentProblem()->fieldInfos())
    {
        SceneLabelMarker *sceneLabel = new SceneLabelMarker(dynamic_cast<SceneLabel *>(m_object), fieldInfo, this);
        layout->addRow(sceneLabel);

        m_labelMarkers.append(sceneLabel);
    }

    txtPointX = new ValueLineEdit();
    txtPointY = new ValueLineEdit();
    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    txtArea = new ValueLineEdit();
    txtArea->setMinimum(0.0);
    connect(txtArea, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    // coordinates must be greater then or equal to 0 (axisymmetric case)
    if (m_object->scene()->parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric)
        txtPointX->setMinimum(0.0);

    // coordinates
    QFormLayout *layoutCoordinates = new QFormLayout();
    layoutCoordinates->addRow(m_object->scene()->parentProblem()->config()->labelX() + " (m):", txtPointX);
    layoutCoordinates->addRow(m_object->scene()->parentProblem()->config()->labelY() + " (m):", txtPointY);

    QGroupBox *grpCoordinates = new QGroupBox(tr("Coordinates"));
    grpCoordinates->setLayout(layoutCoordinates);

    // area
    chkArea = new QCheckBox();
    connect(chkArea, SIGNAL(stateChanged(int)), this, SLOT(doArea(int)));

    QHBoxLayout *layoutArea = new QHBoxLayout();
    layoutArea->addWidget(chkArea);
    layoutArea->addWidget(txtArea);

    // mesh
    QFormLayout *layoutMeshParameters = new QFormLayout();
    layoutMeshParameters->addRow(tr("Element area (Triangle) (m<sup>2</sup>):"), layoutArea);

    QGroupBox *grpMeshParameters = new QGroupBox(tr("Mesh parameters"));
    grpMeshParameters->setLayout(layoutMeshParameters);

    layout->addRow(grpCoordinates);
    layout->addRow(grpMeshParameters);

    fillComboBox();

    return layout;
}

void SceneLabelDialog::fillComboBox()
{
    // markers
    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->fillComboBox();
}

bool SceneLabelDialog::load()
{
    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    txtPointX->setValue(sceneLabel->pointValue().x());
    txtPointY->setValue(sceneLabel->pointValue().y());
    txtArea->setNumber(sceneLabel->area());
    chkArea->setChecked(sceneLabel->area() > 0.0);
    txtArea->setEnabled(chkArea->isChecked());

    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->load();

    return true;
}

bool SceneLabelDialog::save()
{
    if (!txtPointX->evaluate(false)) return false;
    if (!txtPointY->evaluate(false)) return false;
    if (!txtArea->evaluate(false)) return false;

    SceneLabel *sceneLabel = dynamic_cast<SceneLabel *>(m_object);

    PointValue point(txtPointX->value(), txtPointY->value());

    // check if label doesn't exists
    if (m_object->scene()->getLabel(point.point()) && ((sceneLabel->point() != point.point()) || m_isNew))
    {
        QMessageBox::warning(this, "Label", "Label already exists.");
        return false;
    }

    // area
    if (txtArea->value().number() < 0)
    {
        QMessageBox::warning(this, "Label", "Area must be positive or zero.");
        txtArea->setFocus();
        return false;
    }

    if (!m_isNew)
    {
        if (sceneLabel->point() != point.point())
        {
            undoStack()->push(new SceneLabelCommandEdit(sceneLabel->pointValue(), point));
        }
    }

    sceneLabel->setPointValue(point);
    sceneLabel->setArea(chkArea->isChecked() ? txtArea->number() : 0.0);

    foreach (SceneLabelMarker *labelMarker, m_labelMarkers)
        labelMarker->save();

    m_object->scene()->invalidate();
    return true;
}

void SceneLabelDialog::doArea(int state)
{
    txtArea->setEnabled(chkArea->isChecked());
}

SceneLabelSelectDialog::SceneLabelSelectDialog(MarkedSceneBasicContainer<SceneMaterial, SceneLabel> labels, QWidget *parent)
    : QDialog(parent), m_labels(labels)
{
    setWindowTitle(tr("Labels"));

    // markers
    QFormLayout *layoutMaterials = new QFormLayout();

    QGroupBox *grpMaterials = new QGroupBox(tr("Materials"));
    grpMaterials->setLayout(layoutMaterials);

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QComboBox *cmbMaterial = new QComboBox();
        cmbMaterials[fieldInfo] = cmbMaterial;

        layoutMaterials->addRow(fieldInfo->name(), cmbMaterial);
    }

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpMaterials);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    load();

    setMinimumSize(sizeHint());
}

void SceneLabelSelectDialog::load()
{
    // markers
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        cmbMaterials[fieldInfo]->clear();

        // none marker
        cmbMaterials[fieldInfo]->addItem(Agros2D::problem()->scene()->materials->getNone(fieldInfo)->name(),
                                         Agros2D::problem()->scene()->materials->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneMaterial *material, Agros2D::problem()->scene()->materials->filter(fieldInfo).items())
            cmbMaterials[fieldInfo]->addItem(material->name(),
                                             material->variant());
    }

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneMaterial* material = NULL;
        bool match = true;
        foreach(SceneLabel* label, m_labels.items())
        {
            if(material)
                match = match && (material == label->marker(fieldInfo));
            else
                material = label->marker(fieldInfo);
        }
        if(match)
            cmbMaterials[fieldInfo]->setCurrentIndex(cmbMaterials[fieldInfo]->findData(material->variant()));
        else
            cmbMaterials[fieldInfo]->setCurrentIndex(-1);
    }
}

bool SceneLabelSelectDialog::save()
{
    foreach (SceneLabel* label, m_labels.items())
    {
        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if (cmbMaterials[fieldInfo]->currentIndex() != -1)
                label->addMarker(cmbMaterials[fieldInfo]->itemData(cmbMaterials[fieldInfo]->currentIndex()).value<SceneMaterial *>());

        }
    }

    Agros2D::problem()->scene()->invalidate();
    return true;
}

void SceneLabelSelectDialog::doAccept()
{
    if (save())
        accept();
}

void SceneLabelSelectDialog::doReject()
{
    reject();
}

// undo framework *******************************************************************************************************************

SceneLabelCommandAdd::SceneLabelCommandAdd(const PointValue &pointValue, const QMap<QString, QString> &markers, double area, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(pointValue), m_markers(markers), m_area(area)
{
}

void SceneLabelCommandAdd::undo()
{
    Agros2D::problem()->scene()->labels->remove(Agros2D::problem()->scene()->getLabel(m_point.point()));
    Agros2D::problem()->scene()->invalidate();
}

void SceneLabelCommandAdd::redo()
{
    // new edge
    SceneLabel *label = new SceneLabel(Agros2D::problem()->scene(), m_point, m_area);

    label->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addLabel(label);;
    Agros2D::problem()->scene()->invalidate();
}

SceneLabelCommandRemove::SceneLabelCommandRemove(const PointValue &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point), m_markers(markers), m_area(area)
{
}

void SceneLabelCommandRemove::undo()
{
    // new edge
    SceneLabel *label = new SceneLabel(Agros2D::problem()->scene(), m_point, m_area);

    label->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addLabel(label);
    Agros2D::problem()->scene()->invalidate();
}

void SceneLabelCommandRemove::redo()
{
    Agros2D::problem()->scene()->labels->remove(Agros2D::problem()->scene()->getLabel(m_point.point()));
    Agros2D::problem()->scene()->invalidate();
}

SceneLabelCommandEdit::SceneLabelCommandEdit(const PointValue &point, const PointValue &pointNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_point(point), m_pointNew(pointNew)
{
}

void SceneLabelCommandEdit::undo()
{
    SceneLabel *label = Agros2D::problem()->scene()->getLabel(m_pointNew.point());
    if (label)
    {
        label->setPointValue(m_point);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneLabelCommandEdit::redo()
{
    SceneLabel *label = Agros2D::problem()->scene()->getLabel(m_point.point());
    if (label)
    {
        label->setPointValue(m_pointNew);
        Agros2D::problem()->scene()->invalidate();
    }
}

SceneLabelCommandMoveMulti::SceneLabelCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points), m_pointsNew(pointsNew)
{
}

void SceneLabelCommandMoveMulti::moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo)
{
    assert(moveFrom.size() == moveTo.size());
    QList<SceneLabel*> labels;
    for (int i = 0; i < moveFrom.size(); i++)
    {
        PointValue point = moveFrom[i];
        SceneLabel *label = Agros2D::problem()->scene()->getLabel(point.point());
        labels.push_back(label);
    }

    for (int i = 0; i < moveFrom.size(); i++)
    {
        PointValue pointNew = moveTo[i];
        SceneLabel *label = labels[i];
        if (label)
            label->setPointValue(pointNew);
    }
}

void SceneLabelCommandMoveMulti::undo()
{
    moveAll(m_pointsNew, m_points);
    Agros2D::problem()->scene()->invalidate();
}

void SceneLabelCommandMoveMulti::redo()
{
    moveAll(m_points, m_pointsNew);
    Agros2D::problem()->scene()->invalidate();
}

SceneLabelCommandAddOrRemoveMulti::SceneLabelCommandAddOrRemoveMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent)
    : QUndoCommand(parent), m_points(points), m_areas(areas), m_markers(markers)
{
}

void SceneLabelCommandAddOrRemoveMulti::remove()
{
    Agros2D::problem()->scene()->stopInvalidating(true);
    foreach(PointValue point, m_points)
    {
        SceneLabel *label = Agros2D::problem()->scene()->getLabel(point.point());
        if (label)
            Agros2D::problem()->scene()->labels->remove(label);
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneLabelCommandAddOrRemoveMulti::add()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    for (int i = 0; i < m_points.size(); i++)
    {
        SceneLabel *label = new SceneLabel(Agros2D::problem()->scene(), m_points[i], m_areas[i]);

        // if markers are not empty, we were deleting or copying "withMarkers = True"
        if(!m_markers.empty())
        {
            label->addMarkersFromStrings(m_markers[i]);
        }

        Agros2D::problem()->scene()->addLabel(label);
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}
