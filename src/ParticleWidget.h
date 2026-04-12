#pragma once

#include <QWidget>
#include <QTimer>
#include <vector>
#include "Particle.h"

class ParticleWidget : public QWidget {
    Q_OBJECT

public:
    explicit ParticleWidget(QWidget *parent = nullptr);

    // Exposed so MainWindow can display them
    const float (&matrix() const)[4][4] { return m_matrix; }

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    void initParticles();
    void initMatrix();
    void updatePhysics();

    std::vector<Particle> m_particles;
    float m_matrix[4][4]; // m_matrix[i][j]: force color-i exerts on color-j
                          // positive = attract, negative = repel

    QTimer *m_timer;

    // ── Simulation constants ──────────────────────────────────────────────
    static constexpr int   NUM_PARTICLES = 500;
    static constexpr int   NUM_COLORS    = 4;
    static constexpr float R_MAX         = 80.0f;  // interaction radius (px)
    static constexpr float BETA          = 0.3f;   // inner repulsion zone (fraction of R_MAX)
    static constexpr float FRICTION      = 0.5f;   // velocity retention per tick
    static constexpr float FORCE_SCALE   = 0.4f;   // scales raw force → px/tick²
    static constexpr int   PARTICLE_R    = 3;       // draw radius (px)
};
