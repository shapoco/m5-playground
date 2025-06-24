#include <sixel/sixel.h>
#include <stdio.h>

#define DEBUG_STAMP(msg)                              \
  do {                                                \
    printf("%s [%d]: %s\n", __FILE__, __LINE__, msg); \
    fflush(stdout);                                   \
  } while (false)

#include "shapobrot/border_tracer.hpp"
#include "shapobrot/shapobrot.hpp"

const int displayW = 320, displayH = 240;

shapobrot::Config cfg;
shapobrot::Palette palette;
shapobrot::MandelbrotFunc mandelbrot;
shapobrot::Plane **outPlanes;
shapobrot::bordertracing::BorderTracer renderer;

uint16_t outBuff[displayW * displayH];

void delay(int ms) { /* dummy */ }

static int sixel_write(char *data, int size, void *priv) {
  return fwrite(data, 1, size, (FILE *)priv);
}

int main(int argc, char **argv) {
  for (int y = 0; y < displayH; y++) {
    for (int x = 0; x < displayW; x++) {
      outBuff[y * displayW + x] = 0x1ff8;
    }
  }

  // 計算実行
  cfg.width = displayW;
  cfg.height = displayH;
  cfg.function = &mandelbrot;

  // 出力バッファの初期化
  DEBUG_STAMP("Creating output planes");
  outPlanes = mandelbrot.createPlanes(cfg.width, cfg.height);
  for (int i = 0; i < mandelbrot.numOutPlanes(); i++) {
    auto plane = outPlanes[i];
    printf("Output plane[%d]: size=%dx%dpx, stride=%ld, sizeInBytes=%ld\n", i,
           plane->width, plane->height, plane->stride, plane->sizeInBytes());
  }

  DEBUG_STAMP("Initializing renderer");
  renderer.init(cfg, outPlanes);
  {
    auto plane = &renderer.traceMap;
    printf("TraceMap plane: size=%dx%dpx, stride=%ld, sizeInBytes=%ld\n",
           plane->width, plane->height, plane->stride, plane->sizeInBytes());
  }

  renderer.service();

  const int aaw = 80;
  const int aah = 24;
  for (int iy = 0; iy < aah; iy++) {
    for (int ix = 0; ix < aaw; ix++) {
      auto iter = outPlanes[0]->pixelAt<shapobrot::Types::iter_count_t>(
          ix * displayW / aaw, iy * displayH / aah);
      printf("%1d", iter % 10);
    }
    printf("\n");
  }

  // 計算結果を画像に変換
  DEBUG_STAMP("Converting output to image");
  auto *wrPtr = outBuff;
  uint16_t col = 0;
  for (int y = 0; y < displayH; y++) {
    auto rdPtr = outPlanes[0]->lineAt<shapobrot::Types::iter_count_t>(y);
    for (int x = 0; x < displayW; x++) {
      auto iter = *(rdPtr++);
      if (iter > cfg.maxIter) {
        col = 0;
      } else {
        col = palette.colors[iter % shapobrot::Palette::PALETTE_SIZE];
      }
      *(wrPtr++) = col;
    }
  }

  DEBUG_STAMP("Dumping as SIXEL output");
  {
    SIXELSTATUS sxl_status;
    sixel_output_t *sixel_out = nullptr;
    sxl_status = sixel_output_new(&sixel_out, sixel_write, stdout, nullptr);
    if (SIXEL_FAILED(sxl_status)) {
      printf("sixel_output_new() failed: %d\n", sxl_status);
      fflush(stdout);
      goto cleanup;
    }

    sixel_dither_t *dither = sixel_dither_get(SIXEL_BUILTIN_XTERM256);
    sixel_dither_set_pixelformat(dither, SIXEL_PIXELFORMAT_RGB565);

    sxl_status = sixel_encode((unsigned char *)outBuff, displayW, displayH, 0,
                              dither, sixel_out);
    if (SIXEL_FAILED(sxl_status)) {
      printf("sixel_encode() failed: %d\n", sxl_status);
      fflush(stdout);
      goto cleanup;
    }

    printf("\n");
  }

cleanup:

  DEBUG_STAMP("Cleaning up...");
  for (int i = 0; i < mandelbrot.numOutPlanes(); i++) {
    delete outPlanes[i];
  }

  return 0;
}
