#pragma once

#include <cmath>
#include <cstdint>

using real_t = float;

struct Scene {
  int16_t width = 320;
  int16_t height = 240;
  real_t centerX = -0.5;
  real_t centerY = 0;
  float rangeLog2 = 2;
  uint32_t maxIter = 100;
  bool swapXY = true;

  void project(int16_t x, int16_t y, real_t *cx, real_t *cy) const {
    int16_t w = width;
    int16_t h = height;
    float dispRange = sqrt(w * w + h * h);
    real_t sceneRange = pow(2, rangeLog2);
    real_t sceneW = sceneRange * w / dispRange;
    real_t sceneH = sceneRange * h / dispRange;

    real_t offsetX = sceneW * ((real_t)x / w - 0.5f);
    real_t offsetY = sceneH * ((real_t)y / h - 0.5f);
    if (swapXY) {
      *cx = centerX - offsetY;
      *cy = centerY - offsetX;
    } else {
      *cx = centerX + offsetY;
      *cy = centerY + offsetX;
    }
  }
};

struct Task {
  int16_t x;
  int16_t y;
  Task(int16_t x, int16_t y) : x(x), y(y) {}
};

struct PixelState {
  enum Flags {
    HANDLED = 0x10000000u,
    COMPLETED = 0x20000000u,
    DIVERGED = 0x40000000u,
    NUM_ITER = 0x0fffffffu,
  };

  uint32_t raw;

  PixelState() : raw(0) {}

  PixelState(bool completed, uint32_t iter = 0, bool diverged = false)
      : raw(HANDLED | (completed ? COMPLETED : 0) | iter |
            (diverged ? DIVERGED : 0)) {}

  inline bool isUntouched() const { return raw == 0; }
  inline bool isHandled() const { return (raw & HANDLED) != 0; }
  inline bool isFinished() const { return (raw & COMPLETED) != 0; }
  inline bool isDiverged() const { return (raw & DIVERGED) != 0; }
  inline uint32_t numIterations() const { return raw & NUM_ITER; }
};
