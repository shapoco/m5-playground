#include "M5Unified.h"
#include "shapobrot/border_tracer.hpp"

#include <math.h>

M5Canvas *canvas;

int16_t displayW, displayH;
bool first = true;

shapobrot::Config cfg;
shapobrot::Palette palette;
shapobrot::MandelbrotFunc mandelbrot;
shapobrot::Plane **outPlanes;
shapobrot::bordertracing::BorderTracer renderer;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  // ディスプレイ初期化
  displayW = M5.Display.width();
  displayH = M5.Display.height();
  canvas = new M5Canvas(&M5.Display);
  canvas->createSprite(displayW, displayH);

  // 出力バッファの初期化
  outPlanes = mandelbrot.createPlanes(displayW, displayH);
}

void loop() {
  if (first) {
    first = false;

    // 計算実行
    cfg.width = displayW;
    cfg.height = displayH;
    cfg.function = &mandelbrot;
    cfg.swapXY = true;
    renderer.init(cfg, outPlanes);
    renderer.service();

    // 計算結果を画像に変換
    uint16_t col;
    auto wrPtr = reinterpret_cast<uint16_t *>(canvas->getBuffer());
    for (int16_t y = 0; y < displayH; y++) {
      auto rdPtr = outPlanes[0]->lineAt<shapobrot::Types::iter_count_t>(y);
      for (int16_t x = 0; x < displayW; x++) {
        auto iter = *(rdPtr++);
        if (iter >= cfg.maxIter) {
          col = 0;
        } else {
          col = palette.colors[iter % shapobrot::Palette::PALETTE_SIZE];
        }
        *(wrPtr++) = col;
      }
    }

    // 描画
    canvas->pushSprite(0, 0);
  }

  delay(100);
}
