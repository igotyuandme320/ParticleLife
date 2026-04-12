#include "ControlPanel.h"
#include "ParticleWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QFrame>

// ── colour meta ───────────────────────────────────────────────────────────────
static const char *COLOR_NAMES[4]  = { "R", "G", "B", "Y" };
static const char *COLOR_STYLE[4]  = {
    "background:#FF3C3C; color:white;",
    "background:#3CDC3C; color:#111;",
    "background:#3C78FF; color:white;",
    "background:#FFDC28; color:#111;",
};

// ── helpers ───────────────────────────────────────────────────────────────────
static QLabel *makeColorLabel(int c)
{
    QLabel *lbl = new QLabel(COLOR_NAMES[c]);
    lbl->setStyleSheet(QString(COLOR_STYLE[c]) +
                       "font-size:10px; font-weight:bold; border-radius:3px;");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setFixedSize(24, 20);
    return lbl;
}

static QLabel *makeSectionLabel(const QString &text)
{
    QLabel *lbl = new QLabel(text);
    lbl->setStyleSheet("color:#aaaacc; font-size:11px; font-weight:bold;");
    return lbl;
}

// ── constructor ───────────────────────────────────────────────────────────────
ControlPanel::ControlPanel(ParticleWidget *pw, QWidget *parent)
    : QWidget(parent), m_pw(pw)
{
    setFixedWidth(270);
    setStyleSheet("background-color:#12121e; color:#ddddee;");

    // ── scroll wrapper so content never clips ─────────────────────────────
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setStyleSheet("background:transparent;");

    QWidget *inner = new QWidget();
    inner->setStyleSheet("background:transparent;");
    QVBoxLayout *vbox = new QVBoxLayout(inner);
    vbox->setContentsMargins(12, 14, 12, 14);
    vbox->setSpacing(10);

    // ── Title ─────────────────────────────────────────────────────────────
    QLabel *title = new QLabel("Particle Life");
    title->setStyleSheet("color:#ffffff; font-size:16px; font-weight:bold;");
    title->setAlignment(Qt::AlignCenter);
    vbox->addWidget(title);

    QLabel *version = new QLabel("v2.0");
    version->setStyleSheet("color:#6666aa; font-size:11px;");
    version->setAlignment(Qt::AlignCenter);
    vbox->addWidget(version);

    // ── Separator ─────────────────────────────────────────────────────────
    QFrame *sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#333355;");
    vbox->addWidget(sep);

    // ── Simulation sliders ────────────────────────────────────────────────
    {
        QGroupBox *grp = new QGroupBox("Simulation");
        grp->setStyleSheet(
            "QGroupBox { color:#aaaacc; border:1px solid #333355; border-radius:5px; "
            "margin-top:8px; padding-top:6px; font-size:11px; }"
            "QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 4px; }");
        QVBoxLayout *gl = new QVBoxLayout(grp);
        gl->setSpacing(6);

        // Helper lambda: slider row
        auto addSlider = [&](const QString &prefix, QSlider *&slider, QLabel *&valLbl,
                             int lo, int hi, int def) {
            QHBoxLayout *row = new QHBoxLayout();
            QLabel *lbl = new QLabel(prefix);
            lbl->setStyleSheet("font-size:11px; color:#ccccdd;");
            lbl->setFixedWidth(62);

            slider = new QSlider(Qt::Horizontal);
            slider->setRange(lo, hi);
            slider->setValue(def);
            slider->setStyleSheet(
                "QSlider::groove:horizontal { height:4px; background:#333355; border-radius:2px; }"
                "QSlider::handle:horizontal { width:12px; height:12px; margin:-4px 0; "
                "background:#6666cc; border-radius:6px; }"
                "QSlider::sub-page:horizontal { background:#5555aa; border-radius:2px; }");

            valLbl = new QLabel();
            valLbl->setStyleSheet("font-size:11px; color:#aaaacc;");
            valLbl->setFixedWidth(38);
            valLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            row->addWidget(lbl);
            row->addWidget(slider, 1);
            row->addWidget(valLbl);
            gl->addLayout(row);
        };

        addSlider("Particles:", m_countSlider,    m_countLabel,    100, 2000, pw->particleCount());
        addSlider("Radius:",    m_radiusSlider,   m_radiusLabel,    20,  200, static_cast<int>(pw->rMax()));
        addSlider("Friction:",  m_frictionSlider, m_frictionLabel,   1,   99, static_cast<int>(pw->friction() * 100));

        // Initialise labels
        m_countLabel->setText(QString::number(pw->particleCount()));
        m_radiusLabel->setText(QString::number(static_cast<int>(pw->rMax())) + "px");
        m_frictionLabel->setText(QString::number(pw->friction(), 'f', 2));

        vbox->addWidget(grp);

        connect(m_countSlider,    &QSlider::valueChanged, this, &ControlPanel::onCountSlider);
        connect(m_radiusSlider,   &QSlider::valueChanged, this, &ControlPanel::onRadiusSlider);
        connect(m_frictionSlider, &QSlider::valueChanged, this, &ControlPanel::onFrictionSlider);
    }

    // ── Attraction Matrix ─────────────────────────────────────────────────
    {
        QGroupBox *grp = new QGroupBox("Attraction Matrix");
        grp->setStyleSheet(
            "QGroupBox { color:#aaaacc; border:1px solid #333355; border-radius:5px; "
            "margin-top:8px; padding-top:6px; font-size:11px; }"
            "QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 4px; }");

        QGridLayout *grid = new QGridLayout(grp);
        grid->setSpacing(3);
        grid->setContentsMargins(6, 8, 6, 6);

        // Corner spacer
        grid->addWidget(new QLabel(""), 0, 0);

        // Column headers (target color)
        for (int j = 0; j < 4; ++j)
            grid->addWidget(makeColorLabel(j), 0, j + 1, Qt::AlignCenter);

        // Row headers + spin boxes
        for (int i = 0; i < 4; ++i) {
            grid->addWidget(makeColorLabel(i), i + 1, 0, Qt::AlignCenter);

            for (int j = 0; j < 4; ++j) {
                QDoubleSpinBox *sb = new QDoubleSpinBox();
                sb->setRange(-1.0, 1.0);
                sb->setSingleStep(0.05);
                sb->setDecimals(2);
                sb->setValue(static_cast<double>(pw->matrixValue(i, j)));
                sb->setFixedWidth(55);
                sb->setStyleSheet(
                    "QDoubleSpinBox { background:#1e1e36; color:#ddddee; "
                    "border:1px solid #444466; border-radius:3px; font-size:10px; padding:1px; }"
                    "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width:14px; }");

                connect(sb, &QDoubleSpinBox::valueChanged,
                        this, [this, i, j](double v){ onMatrixCell(i, j, v); });

                m_cells[i][j] = sb;
                grid->addWidget(sb, i + 1, j + 1);
            }
        }

        // Row legend hint
        QLabel *hint = new QLabel("row → me,  col → them");
        hint->setStyleSheet("font-size:9px; color:#666688;");
        hint->setAlignment(Qt::AlignCenter);
        QVBoxLayout *wrapVBox = new QVBoxLayout();
        wrapVBox->addWidget(grp);
        wrapVBox->addWidget(hint);
        vbox->addLayout(wrapVBox);
    }

    // ── Buttons ───────────────────────────────────────────────────────────
    {
        const QString btnStyle =
            "QPushButton { background:#2a2a4a; color:#ccccee; border:1px solid #444466; "
            "border-radius:5px; padding:6px 4px; font-size:11px; }"
            "QPushButton:hover  { background:#3a3a6a; }"
            "QPushButton:pressed{ background:#1a1a3a; }";

        auto makeBtn = [&](const QString &text) {
            QPushButton *b = new QPushButton(text);
            b->setStyleSheet(btnStyle);
            b->setCursor(Qt::PointingHandCursor);
            return b;
        };

        QPushButton *randomBtn = makeBtn("🎲  Random Rules");
        QPushButton *resetBtn  = makeBtn("↺  Reset Particles");
        m_pauseBtn             = makeBtn("⏸  Pause");

        vbox->addWidget(randomBtn);
        vbox->addWidget(resetBtn);
        vbox->addWidget(m_pauseBtn);

        connect(randomBtn,  &QPushButton::clicked, this, &ControlPanel::onRandomRules);
        connect(resetBtn,   &QPushButton::clicked, this, &ControlPanel::onResetParticles);
        connect(m_pauseBtn, &QPushButton::clicked, this, &ControlPanel::onTogglePause);
    }

    vbox->addStretch();

    scroll->setWidget(inner);
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);

    // When the simulation randomises the matrix, refresh our spinboxes
    connect(m_pw, &ParticleWidget::matrixRandomized, this, &ControlPanel::refreshMatrix);
}

// ── slots ─────────────────────────────────────────────────────────────────────
void ControlPanel::onCountSlider(int v)
{
    m_countLabel->setText(QString::number(v));
    m_pw->setParticleCount(v);
}

void ControlPanel::onRadiusSlider(int v)
{
    m_radiusLabel->setText(QString::number(v) + "px");
    m_pw->setRMax(static_cast<float>(v));
}

void ControlPanel::onFrictionSlider(int v)
{
    float f = v / 100.0f;
    m_frictionLabel->setText(QString::number(f, 'f', 2));
    m_pw->setFriction(f);
}

void ControlPanel::onMatrixCell(int i, int j, double v)
{
    if (m_blockMatrixSignals) return;
    m_pw->setMatrixValue(i, j, static_cast<float>(v));
}

void ControlPanel::onRandomRules()
{
    m_pw->randomizeRules(); // emits matrixRandomized → refreshMatrix()
}

void ControlPanel::onResetParticles()
{
    m_pw->resetParticles();
}

void ControlPanel::onTogglePause()
{
    m_pw->togglePause();
    m_pauseBtn->setText(m_pw->isPaused() ? "▶  Resume" : "⏸  Pause");
}

void ControlPanel::refreshMatrix()
{
    m_blockMatrixSignals = true;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m_cells[i][j]->setValue(static_cast<double>(m_pw->matrixValue(i, j)));
    m_blockMatrixSignals = false;
}
