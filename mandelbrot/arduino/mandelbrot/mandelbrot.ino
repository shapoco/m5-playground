#include "M5Unified.h"
#include <math.h>

M5Canvas *canvas;

int displayW, displayH;
bool first = true;

static constexpr uint16_t color32to16(uint32_t color) {
  uint16_t c = ((color & 0xF80000) >> 8) | ((color & 0x00FC00) >> 5) |
               ((color & 0x0000F8) >> 3);
  return ((c >> 8) & 0xff) | ((c << 8) & 0xff00);
}

uint16_t colors[] = {
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
    // clang-format on
};
constexpr int PALETTE_SIZE = sizeof(colors) / sizeof(colors[0]);

struct Scene {
  float centerX = -0.5;
  float centerY = 0;
  float rangeLog2 = 2;
  int maxIter = 100;
  bool swapXY = true;
};

static void render(uint16_t *buff, int w, int h, Scene &scene);
static int mandelbrot(Scene &scene, float x, float y);

Scene scene;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  displayW = M5.Display.width();
  displayH = M5.Display.height();

  canvas = new M5Canvas(&M5.Display);
  canvas->createSprite(displayW, displayH);
}

void loop() {
  if (first) {
    first = false;
    auto ptr = (uint16_t *)canvas->getBuffer();
    render(ptr, displayW, displayH, scene);
    canvas->pushSprite(0, 0);
  }

  delay(100);
}

static void render(uint16_t *buff, int w, int h, Scene &scene) {
  float dispRange = sqrt(w * w + h * h);
  float sceneRange = pow(2, scene.rangeLog2);
  float sceneW = sceneRange * w / dispRange;
  float sceneH = sceneRange * h / dispRange;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      float offsetX = sceneW * ((float)x / displayW - 0.5f);
      float offsetY = sceneH * ((float)y / displayH - 0.5f);
      float cx = scene.centerX;
      float cy = scene.centerY;
      if (scene.swapXY) {
        cx -= offsetY;
        cy -= offsetX;
      }
      else {
        cx += offsetX;
        cy += offsetY;
      }
      int iter = mandelbrot(scene, cx, cy);
      buff[y * w + x] =
          (iter >= scene.maxIter) ? 0 : colors[iter % PALETTE_SIZE];
    }
  }
}

static int mandelbrot(Scene &scene, float x, float y) {
  float zx = 0.0f, zy = 0.0f;
  int iter = 0;
  while (zx * zx + zy * zy < 4.0f && iter < scene.maxIter) {
    float tmp = zx * zx - zy * zy + x;
    zy = 2.0f * zx * zy + y;
    zx = tmp;
    iter++;
  }
  return iter;
}
