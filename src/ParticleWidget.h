#pragma once

#include <QWidget>
#include <QTimer>
#include <vector>
#include "Particle.h"

class ParticleWidget : public QWidget {
    Q_OBJECT

public:
    explicit ParticleWidget(QWidget *parent = nullptr);

    // ── Getters (for ControlPanel to read initial state) ──────────────────
    float matrixValue(int i, int j) const { return m_matrix[i][j]; }
    bool  isPaused()      const { return !m_timer->isActive(); }
    int   particleCount() const { return m_numParticles; }
    float rMax()          const { return m_rMax; }
    float friction()      const { return m_friction; }

signals:
    void matrixRandomized(); // emitted after randomizeRules()

public slots:
    void setParticleCount(int n);
    void setRMax(float r);
    void setFriction(float f);
    void setMatrixValue(int i, int j, float v);
    void randomizeRules();   // new random matrix, then emits matrixRandomized()
    void resetParticles();   // scatter particles, keep current rules
    void togglePause();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void tick();

private:
    void initParticles();
    void initMatrix();
    void updatePhysics();

    std::vector<Particle> m_particles;
    float  m_matrix[4][4];
    QTimer *m_timer;

    // ── Runtime-settable simulation parameters ────────────────────────────
    int   m_numParticles = 500;
    float m_rMax         = 80.0f;
    float m_friction     = 0.5f;

    // ── Fixed constants ───────────────────────────────────────────────────
    static constexpr int   NUM_COLORS  = 4;
    static constexpr float BETA        = 0.3f;  // inner repulsion zone fraction
    static constexpr float FORCE_SCALE = 0.4f;  // raw force → px/tick²
    static constexpr int   PARTICLE_R  = 3;     // draw radius (px)
};
