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

#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H

#include "util.h"

class PostDeal;
class LineEditDouble;
class SceneViewPostInterface;
class FieldInfo;
class Computation;

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    VideoDialog(SceneViewPostInterface *sceneViewInterface, Computation *computation, QWidget *parent = 0);
    ~VideoDialog();

    void showDialog();

private:
    SceneViewPostInterface *m_sceneViewInterface;
    Computation *m_computation;

    int m_timeSteps;
    int m_timeStepStore;
    QList<double> m_timeLevels;

    int m_adaptiveStepStore;
    int m_adaptiveSteps;
    bool m_showRulersStore;
    bool m_showGridStore;
    bool m_showAxesStore;

    QStackedWidget *tabType;
    QWidget *tabTransient;
    QWidget *tabAdaptivity;

    QTimer *timer;

    // file
    QPushButton *btnClose;
    QPushButton *btnGenerate;

    QCheckBox *chkSaveImages;
    QCheckBox *chkFigureShowGrid;
    QCheckBox *chkFigureShowRulers;
    QCheckBox *chkFigureShowAxes;

    // adaptivity
    QLabel *lblAdaptiveStep;
    QSlider *sliderAdaptiveAnimate;

    // transient
    QLabel *lblTransientStep;
    QLabel *lblTransientTime;
    QSlider *sliderTransientAnimate;

    void createControls();
    QWidget *createControlsViewportAdaptiveSteps();
    QWidget *createControlsViewportTimeSteps();

private slots:
    void adaptiveAnimate();
    void adaptiveAnimateNextStep();
    void setAdaptiveStep(int adaptiveStep);

    void transientAnimate();
    void transientAnimateNextStep();
    void setTransientStep(int transientStep);

    void doClose();
    void doVideo();
    void doOpenFolder();
};

// *********************************************************************************

class ImageSequenceDialog : public QDialog
{
    Q_OBJECT

public:
    ImageSequenceDialog(QWidget *parent = 0);
    ~ImageSequenceDialog();

    bool showDialog();

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    QStringList m_images;
    QPixmap m_currentImage;

    QTimer *timer;
    QLabel *lblImage;
    QLabel *lblStep;

    QPushButton *btnClose;
    QPushButton *btnAnimate;
    QSlider *sliderAnimateSequence;
    QComboBox *cmbSpeed;

private slots:
    void updateImage();
    void animate();
    void animateNextStep();
    void animateSequence(int index);

    void doClose();
};

#endif // VIDEODIALOG_H
