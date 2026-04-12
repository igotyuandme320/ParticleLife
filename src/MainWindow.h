#pragma once

#include <QMainWindow>

class ParticleWidget;
class ControlPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    ParticleWidget *m_particleWidget;
    ControlPanel   *m_controlPanel;
};
