#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <queue>

#include "shapobrot/plane.hpp"
#include "shapobrot/shapobrot.hpp"
#include "shapobrot/shapobrot_common.hpp"

namespace shapobrot::bordertracing {

// ピクセル毎の計算の状態
enum class TraceState : uint8_t {
  UNTOUCHED = 0,
  HANDLED = (1 << 0),
  COMPLETED = (1 << 1),
};

static SHAPOBROT_INLINE TraceState operator|(TraceState a, TraceState b) {
  return static_cast<TraceState>(static_cast<uint8_t>(a) |
                                 static_cast<uint8_t>(b));
}
static SHAPOBROT_INLINE TraceState operator&(TraceState a, TraceState b) {
  return static_cast<TraceState>(static_cast<uint8_t>(a) &
                                 static_cast<uint8_t>(b));
}
static SHAPOBROT_INLINE bool operator==(TraceState a, TraceState b) {
  return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);
}
static SHAPOBROT_INLINE bool operator!=(TraceState a, TraceState b) {
  return static_cast<uint8_t>(a) != static_cast<uint8_t>(b);
}
static SHAPOBROT_INLINE bool operator!(TraceState a) {
  return static_cast<uint8_t>(a) == 0;
}

// 計算タスク
struct Task {
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  pos_t x;
  pos_t y;
  Task(pos_t x, pos_t y) : x(x), y(y) {}
};

// レンダラー
class BorderTracer : public Renderer {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  Plane traceMap;
  Plane **outPlanes = nullptr;

  uint8_t numOutPlanes = 0;
  void **funcOut;

  std::queue<Task> queue;

  BorderTracer() : traceMap(PlaneDesc(sizeof(TraceState))), funcOut(nullptr) {}

  ~BorderTracer() {
    if (funcOut != nullptr) {
      delete[] funcOut;
      funcOut = nullptr;
    }
  }

  Result init(Config &cfg, Plane **outputs) override {
    Result ret;

    this->cfg = cfg;
    this->outPlanes = outputs;

    ret = Renderer::init(cfg, outputs);
    if (ret != Result::SUCCESS) return ret;

    auto w = cfg.width;
    auto h = cfg.height;

    // 作業バッファの初期化
    traceMap.setSize(w, h);
    traceMap.clear();

    // 関数出力用のバッファの初期化
    uint8_t n = cfg.function->numOutPlanes();
    if (numOutPlanes != n && funcOut == nullptr) {
      if (funcOut != nullptr) {
        delete[] funcOut;
      }
      funcOut = new void *[n];
      numOutPlanes = n;
    }

    for (pos_t x = 0; x < w; x++) {
      enqueue(x, 0);
      enqueue(x, h - 1);
    }
    for (pos_t y = 1; y < h - 1; y++) {
      enqueue(0, y);
      enqueue(w - 1, y);
    }

    return Result::SUCCESS;
  }

  Result service() override {
    Result ret;

    ret = Renderer::service();
    if (ret != Result::SUCCESS) return ret;

    auto w = cfg.width;
    auto h = cfg.height;

    // border tracing アルゴリズムの適用
    while (!queue.empty()) {
      auto task = queue.front();
      queue.pop();
      auto x = task.x;
      auto y = task.y;

      // 出力バッファの確保
      for (size_t i = 0; i < numOutPlanes; i++) {
        funcOut[i] = &(outPlanes[i]->pixelAt<uint8_t>(x, y));
      }

      // 計算の実行
      real_t a, b;
      cfg.project(x, y, &a, &b);
      ret = cfg.function->compute(a, b, funcOut);

      traceMap.pixelAt<TraceState>(x, y) =
          TraceState::HANDLED | TraceState::COMPLETED;
      if (ret != Result::SUCCESS) return ret;

      // 境界線の追跡
      if (x > 0) trace(x, y, false, -1);
      if (x < w - 1) trace(x, y, false, 1);
      if (y > 0) trace(x, y, true, -1);
      if (y < h - 1) trace(x, y, true, 1);
    }

    // 計算しなかった部分を埋める
    for (size_t i = 0; i < numOutPlanes; i++) {
      int elemSize = outPlanes[i]->desc.sizeOfElement;
      for (pos_t y = 0; y < h; y++) {
        auto rdPtr = traceMap.lineAt<TraceState>(y);
        auto wrPtr = outPlanes[i]->lineAt<uint8_t>(y);
        uint8_t *last = wrPtr;
        for (pos_t x = 0; x < w; x++) {
          auto state = *(rdPtr++);
          if (!!(state & TraceState::COMPLETED)) {
            last = wrPtr;
          } else {
            memcpy(wrPtr, last, elemSize);
          }
          wrPtr += elemSize;
        }
      }
    }

    return Result::SUCCESS;
  }

  void trace(pos_t x, pos_t y, bool vertical, pos_t sign) {
    auto w = cfg.width;
    auto h = cfg.height;

    // 注目ピクセルの状態
    auto focusState = traceMap.pixelAt<TraceState>(x, y);
    auto focusOutput = outPlanes[0]->pixelAt<iter_count_t>(x, y);

    // 隣接ピクセルの状態
    pos_t nx = x + (vertical ? 0 : sign);
    pos_t ny = y + (vertical ? sign : 0);
    auto nextState = traceMap.pixelAt<TraceState>(nx, ny);
    auto nextOutput = outPlanes[0]->pixelAt<iter_count_t>(nx, ny);

    // 計算が完了していない場合は無視
    if (!(nextState & TraceState::COMPLETED)) return;

    if (focusOutput != nextOutput) {
      // 境界線の検出 --> 周辺のピクセルをキューに追加
      if (vertical) {
        enqueue(x - 1, y);
        enqueue(x + 1, y);
        enqueue(x - 1, y + sign);
        enqueue(x + 1, y + sign);
      } else {
        enqueue(x, y - 1);
        enqueue(x, y + 1);
        enqueue(x + sign, y - 1);
        enqueue(x + sign, y + 1);
      }
    }
  }

  void enqueue(pos_t x, pos_t y) {
    if (x < 0 || cfg.width <= x || y < 0 || cfg.height <= y) return;

    auto *ptr = &traceMap.pixelAt<TraceState>(x, y);
    if (*ptr == TraceState::UNTOUCHED) {
      *ptr = TraceState::HANDLED;
      queue.push(Task(x, y));
    }
  }
};

}  // namespace shapobrot::bordertracing
