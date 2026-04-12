#pragma once

struct Particle {
    float x, y;   // position in pixels
    float vx, vy; // velocity in pixels/tick
    int   color;  // 0=Red  1=Green  2=Blue  3=Yellow
};
