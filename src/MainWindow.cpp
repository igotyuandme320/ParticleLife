#include "MainWindow.h"
#include "ParticleWidget.h"
#include "ControlPanel.h"

#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Particle Life v2.0");
    setStyleSheet("background-color:#12121e;");

    m_particleWidget = new ParticleWidget(this);
    m_controlPanel   = new ControlPanel(m_particleWidget, this);

    QWidget *central = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_controlPanel);
    layout->addWidget(m_particleWidget, 1); // particle widget takes all extra space

    setCentralWidget(central);
    resize(1070, 800);
}
