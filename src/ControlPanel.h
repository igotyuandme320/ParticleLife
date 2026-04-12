#pragma once

#include <QWidget>

class QSlider;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class ParticleWidget;

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(ParticleWidget *pw, QWidget *parent = nullptr);

public slots:
    void refreshMatrix(); // re-read all matrix values from ParticleWidget

private slots:
    void onCountSlider(int v);
    void onRadiusSlider(int v);
    void onFrictionSlider(int v);
    void onMatrixCell(int i, int j, double v);
    void onRandomRules();
    void onResetParticles();
    void onTogglePause();

private:
    ParticleWidget *m_pw;

    QSlider *m_countSlider;
    QLabel  *m_countLabel;
    QSlider *m_radiusSlider;
    QLabel  *m_radiusLabel;
    QSlider *m_frictionSlider;
    QLabel  *m_frictionLabel;

    QDoubleSpinBox *m_cells[4][4];
    QPushButton    *m_pauseBtn;

    bool m_blockMatrixSignals = false; // prevent feedback loop during refreshMatrix()
};
