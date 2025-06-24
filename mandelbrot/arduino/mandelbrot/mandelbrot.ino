#include "M5Unified.h"

#include <math.h>
#include <cstring>
#include <queue>

#include "border_tracing.hpp"

M5Canvas *canvas;

int16_t displayW, displayH;
bool first = true;

using color_t = uint16_t;

static constexpr color_t color32to16(uint32_t color) {
  uint16_t c = ((color & 0xF80000) >> 8) | ((color & 0x00FC00) >> 5) |
               ((color & 0x0000F8) >> 3);
  return ((c >> 8) & 0xff) | ((c << 8) & 0xff00);
}

color_t colors[] = {
    // clang-format off
  color32to16(0x000000),
  color32to16(0x000040),
  color32to16(0x000080),
  color32to16(0x0000c0),
  color32to16(0x0000ff),
  color32to16(0x0040ff),
  color32to16(0x0080ff),
  color32to16(0x00c0ff),
  color32to16(0x00ffff),
  color32to16(0x40ffff),
  color32to16(0x80ffff),
  color32to16(0xc0ffff),
  color32to16(0xffffff),
  color32to16(0xffffc0),
  color32to16(0xffff80),
  color32to16(0xffff40),
  color32to16(0xffff00),
  color32to16(0xffc000),
  color32to16(0xff8000),
  color32to16(0xff4000),
  color32to16(0xff0000),
  color32to16(0xc00000),
  color32to16(0x800000),
  color32to16(0x400000),
    // clang-format on
};
constexpr int PALETTE_SIZE = sizeof(colors) / sizeof(colors[0]);

static void render(uint16_t *buff, Scene &scene);
static PixelState mandelbrot(Scene &scene, Task &task);
static void trace(Scene &scene, int16_t x, int16_t y, bool hori, int sign);
static void pushTask(Scene &scene, int16_t x, int16_t y);

Scene scene;
std::queue<Task> queue;

PixelState *workBuff;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  displayW = M5.Display.width();
  displayH = M5.Display.height();

  canvas = new M5Canvas(&M5.Display);
  canvas->createSprite(displayW, displayH);

  workBuff = new PixelState[displayW * displayH];

  scene.width = displayW;
  scene.height = displayH;
}

void loop() {
  if (first) {
    first = false;

    render(scene);

    color_t col = 0;
    auto *rdPtr = workBuff;
    auto *wrPtr = (uint16_t *)canvas->getBuffer();
    for (int16_t y = 0; y < displayH; y++) {
      for (int16_t x = 0; x < displayW; x++) {
        auto state = *(rdPtr++);
        if (state.isFinished()) {
          if (state.isDiverged()) {
            col = colors[state.numIterations() % PALETTE_SIZE];
          } else {
            col = 0;
          }
        }
        *(wrPtr++) = col;
      }
    }

    canvas->pushSprite(0, 0);
  }

  delay(100);
}

static void render(Scene &scene) {
  auto w = scene.width;
  auto h = scene.height;

  memset(workBuff, 0, sizeof(PixelState) * w * h);
  for (int16_t x = 0; x < w; x++) {
    pushTask(scene, x, 0);
    pushTask(scene, x, h - 1);
  }
  for (int16_t y = 1; y < h - 1; y++) {
    pushTask(scene, 0, y);
    pushTask(scene, w - 1, y);
  }

  while (!queue.empty()) {
    auto task = queue.front();
    queue.pop();

    auto x = task.x;
    auto y = task.y;

    auto state = mandelbrot(scene, task);
    auto iter = state.numIterations();

    auto iPixel = (size_t)y * w + x;
    workBuff[iPixel] = state;
    if (x > 0) {
      trace(scene, x, y, false, -1);
    }
    if (x < w - 1) {
      trace(scene, x, y, false, 1);
    }
    if (y > 0) {
      trace(scene, x, y, true, -1);
    }
    if (y < h - 1) {
      trace(scene, x, y, true, 1);
    }
  }
}

static PixelState mandelbrot(Scene &scene, Task &task) {
  real_t a, b;
  scene.project(task.x, task.y, &a, &b);

  real_t r = 0, i = 0;
  real_t rr = 0, ii = 0;
  uint32_t iter = 0;
  while (rr + ii < 4 && iter++ < scene.maxIter) {
    real_t tmp = rr - ii + a;
    i = 2 * r * i + b;
    r = tmp;
    rr = r * r;
    ii = i * i;
  }

  return PixelState(true, iter, iter < scene.maxIter);
}

static void trace(Scene &scene, int16_t x, int16_t y, bool hori, int sign) {
  auto w = scene.width;
  auto h = scene.height;

  auto *work = workBuff + (size_t)y * w + x;
  auto *next = work + sign * (hori ? 1 : w);
  if (!next->isFinished()) return;

  if (work->numIterations() == next->numIterations()) return;

  if (hori) {
    pushTask(scene, x, y - 1);
    pushTask(scene, x, y + 1);
    pushTask(scene, x + sign, y - 1);
    pushTask(scene, x + sign, y + 1);
  } else {
    pushTask(scene, x - 1, y);
    pushTask(scene, x + 1, y);
    pushTask(scene, x - 1, y + sign);
    pushTask(scene, x + 1, y + sign);
  }
}

static void pushTask(Scene &scene, int16_t x, int16_t y) {
  if (x < 0 || x >= scene.width || y < 0 || y >= scene.height) return;

  auto *ptr = workBuff + (size_t)y * scene.width + x;
  if (ptr->isUntouched()) {
    *ptr = PixelState(false);
    queue.push(Task(x, y));
  }
}
