#include "MainWindow.h"
#include "ParticleWidget.h"

#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Particle Life v1.0");

    m_particleWidget = new ParticleWidget(this);
    setCentralWidget(m_particleWidget);

    // Size the window exactly around the fixed-size widget
    adjustSize();
    setFixedSize(sizeHint());
}
