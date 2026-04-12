#include "ParticleWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QColor>
#include <cmath>
#include <random>
#include <algorithm>

// ── colour palette ────────────────────────────────────────────────────────────
static const QColor COLORS[4] = {
    QColor(255,  60,  60),  // Red
    QColor( 60, 220,  60),  // Green
    QColor( 60, 120, 255),  // Blue
    QColor(255, 220,  40),  // Yellow
};

// ── force function ────────────────────────────────────────────────────────────
// r_norm : normalised distance [0, 1]  (r / rMax)
// a      : attraction coefficient    [-1, 1]
// returns: signed force magnitude    [-1, 1]
static float forceFunc(float r_norm, float a, float beta)
{
    if (r_norm < beta)
        return r_norm / beta - 1.0f;                                     // repulsion
    if (r_norm < 1.0f)
        return a * (1.0f - std::fabs(2.0f * r_norm - 1.0f - beta) / (1.0f - beta)); // lobe
    return 0.0f;
}

// ── constructor ───────────────────────────────────────────────────────────────
ParticleWidget::ParticleWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 600);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: black;");

    initMatrix();
    initParticles();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ParticleWidget::tick);
    m_timer->start(16);
}

// ── init ──────────────────────────────────────────────────────────────────────
void ParticleWidget::initParticles()
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posX(0.0f, static_cast<float>(width()  ? width()  : 800));
    std::uniform_real_distribution<float> posY(0.0f, static_cast<float>(height() ? height() : 800));
    std::uniform_int_distribution<int>    col(0, NUM_COLORS - 1);

    m_particles.resize(m_numParticles);
    for (auto &p : m_particles) {
        p.x     = posX(rng);
        p.y     = posY(rng);
        p.vx    = 0.0f;
        p.vy    = 0.0f;
        p.color = col(rng);
    }
}

void ParticleWidget::initMatrix()
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> coeff(-1.0f, 1.0f);
    for (int i = 0; i < NUM_COLORS; ++i)
        for (int j = 0; j < NUM_COLORS; ++j)
            m_matrix[i][j] = coeff(rng);
}

// ── public slots ──────────────────────────────────────────────────────────────
void ParticleWidget::setParticleCount(int n)
{
    m_numParticles = std::clamp(n, 10, 5000);
    resetParticles();
}

void ParticleWidget::setRMax(float r)
{
    m_rMax = std::clamp(r, 10.0f, 300.0f);
}

void ParticleWidget::setFriction(float f)
{
    m_friction = std::clamp(f, 0.01f, 0.99f);
}

void ParticleWidget::setMatrixValue(int i, int j, float v)
{
    if (i >= 0 && i < NUM_COLORS && j >= 0 && j < NUM_COLORS)
        m_matrix[i][j] = std::clamp(v, -1.0f, 1.0f);
}

void ParticleWidget::randomizeRules()
{
    initMatrix();
    emit matrixRandomized();
}

void ParticleWidget::resetParticles()
{
    initParticles();
}

void ParticleWidget::togglePause()
{
    if (m_timer->isActive())
        m_timer->stop();
    else
        m_timer->start(16);
}

// ── tick ──────────────────────────────────────────────────────────────────────
void ParticleWidget::tick()
{
    updatePhysics();
    update();
}

// ── physics ───────────────────────────────────────────────────────────────────
void ParticleWidget::updatePhysics()
{
    const int   n     = static_cast<int>(m_particles.size());
    const float W     = static_cast<float>(width());
    const float H     = static_cast<float>(height());
    const float halfW = W * 0.5f;
    const float halfH = H * 0.5f;
    const float r2max = m_rMax * m_rMax;

    std::vector<float> fx(n, 0.0f), fy(n, 0.0f);

    for (int i = 0; i < n; ++i) {
        const float xi = m_particles[i].x;
        const float yi = m_particles[i].y;
        const int   ci = m_particles[i].color;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            float dx = m_particles[j].x - xi;
            float dy = m_particles[j].y - yi;

            // Toroidal shortest-path
            if (dx >  halfW) dx -= W;
            if (dx < -halfW) dx += W;
            if (dy >  halfH) dy -= H;
            if (dy < -halfH) dy += H;

            const float r2 = dx * dx + dy * dy;
            if (r2 <= 0.0f || r2 >= r2max) continue;

            const float r      = std::sqrt(r2);
            const float r_norm = r / m_rMax;
            const float a      = m_matrix[ci][m_particles[j].color];
            const float f      = forceFunc(r_norm, a, BETA);

            const float inv_r = 1.0f / r;
            fx[i] += dx * inv_r * f;
            fy[i] += dy * inv_r * f;
        }
    }

    for (int i = 0; i < n; ++i) {
        m_particles[i].vx = m_particles[i].vx * m_friction + fx[i] * FORCE_SCALE;
        m_particles[i].vy = m_particles[i].vy * m_friction + fy[i] * FORCE_SCALE;

        m_particles[i].x += m_particles[i].vx;
        m_particles[i].y += m_particles[i].vy;

        if (m_particles[i].x <  0.0f) m_particles[i].x += W;
        if (m_particles[i].x >= W)    m_particles[i].x -= W;
        if (m_particles[i].y <  0.0f) m_particles[i].y += H;
        if (m_particles[i].y >= H)    m_particles[i].y -= H;
    }
}

// ── render ────────────────────────────────────────────────────────────────────
void ParticleWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), Qt::black);

    for (const auto &p : m_particles) {
        painter.setBrush(COLORS[p.color]);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(p.x, p.y),
                            static_cast<float>(PARTICLE_R),
                            static_cast<float>(PARTICLE_R));
    }
}
