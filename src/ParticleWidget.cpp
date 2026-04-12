#include "ParticleWidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QColor>
#include <cmath>
#include <random>

// ── colour palette ────────────────────────────────────────────────────────────
static const QColor COLORS[4] = {
    QColor(255,  60,  60),   // Red
    QColor( 60, 220,  60),   // Green
    QColor( 60, 120, 255),   // Blue
    QColor(255, 220,  40),   // Yellow
};

// ── force function ────────────────────────────────────────────────────────────
// r_norm : normalised distance [0, 1]  (r / R_MAX)
// a      : attraction coefficient    [-1, 1]
// returns: signed force magnitude    [-1, 1]
//   r < BETA          → short-range repulsion (prevents overlap)
//   BETA ≤ r < 1      → triangle-shaped attractive/repulsive lobe
static float forceFunc(float r_norm, float a, float beta)
{
    if (r_norm < beta) {
        // Linearly ramps from -1 (at r=0) to 0 (at r=beta)
        return r_norm / beta - 1.0f;
    }
    if (r_norm < 1.0f) {
        // Triangle peak at midpoint between beta and 1
        return a * (1.0f - std::fabs(2.0f * r_norm - 1.0f - beta) / (1.0f - beta));
    }
    return 0.0f;
}

// ── constructor ───────────────────────────────────────────────────────────────
ParticleWidget::ParticleWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(800, 800);
    setStyleSheet("background-color: black;");

    // Use a fast random device once
    initMatrix();
    initParticles();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ParticleWidget::tick);
    m_timer->start(16); // ~60 fps
}

// ── initialisation ────────────────────────────────────────────────────────────
void ParticleWidget::initParticles()
{
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posX(0.0f, 800.0f);
    std::uniform_real_distribution<float> posY(0.0f, 800.0f);
    std::uniform_int_distribution<int>    col(0, NUM_COLORS - 1);

    m_particles.resize(NUM_PARTICLES);
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

// ── simulation tick ───────────────────────────────────────────────────────────
void ParticleWidget::tick()
{
    updatePhysics();
    update(); // trigger paintEvent
}

void ParticleWidget::updatePhysics()
{
    const int   n  = static_cast<int>(m_particles.size());
    const float W  = static_cast<float>(width());
    const float H  = static_cast<float>(height());
    const float halfW = W * 0.5f;
    const float halfH = H * 0.5f;
    const float r2max = R_MAX * R_MAX;

    // Accumulate forces into a temporary buffer so positions/velocities
    // are not modified mid-loop.
    std::vector<float> fx(n, 0.0f), fy(n, 0.0f);

    for (int i = 0; i < n; ++i) {
        const float xi = m_particles[i].x;
        const float yi = m_particles[i].y;
        const int   ci = m_particles[i].color;

        for (int j = 0; j < n; ++j) {
            if (i == j) continue;

            float dx = m_particles[j].x - xi;
            float dy = m_particles[j].y - yi;

            // Toroidal (wrap-around) shortest-path distance
            if (dx >  halfW) dx -= W;
            if (dx < -halfW) dx += W;
            if (dy >  halfH) dy -= H;
            if (dy < -halfH) dy += H;

            const float r2 = dx * dx + dy * dy;
            if (r2 <= 0.0f || r2 >= r2max) continue;

            const float r      = std::sqrt(r2);
            const float r_norm = r / R_MAX;
            const float a      = m_matrix[ci][m_particles[j].color];
            const float f      = forceFunc(r_norm, a, BETA);

            // Normalise direction and accumulate
            const float inv_r = 1.0f / r;
            fx[i] += dx * inv_r * f;
            fy[i] += dy * inv_r * f;
        }
    }

    // Integrate
    for (int i = 0; i < n; ++i) {
        // v = v * friction + force * scale  (friction gives velocity decay)
        m_particles[i].vx = m_particles[i].vx * FRICTION + fx[i] * FORCE_SCALE;
        m_particles[i].vy = m_particles[i].vy * FRICTION + fy[i] * FORCE_SCALE;

        // Update position
        m_particles[i].x += m_particles[i].vx;
        m_particles[i].y += m_particles[i].vy;

        // Toroidal boundary wrapping
        if (m_particles[i].x <  0.0f) m_particles[i].x += W;
        if (m_particles[i].x >= W)    m_particles[i].x -= W;
        if (m_particles[i].y <  0.0f) m_particles[i].y += H;
        if (m_particles[i].y >= H)    m_particles[i].y -= H;
    }
}

// ── rendering ─────────────────────────────────────────────────────────────────
void ParticleWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Black background
    painter.fillRect(rect(), Qt::black);

    // Draw each particle as a filled circle
    for (const auto &p : m_particles) {
        painter.setBrush(COLORS[p.color]);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(
            QPointF(p.x, p.y),
            static_cast<float>(PARTICLE_R),
            static_cast<float>(PARTICLE_R));
    }
}
