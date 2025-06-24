#pragma once

#include <cmath>
#include <cstdint>

#include "shapobrot/palette.hpp"
#include "shapobrot/plane.hpp"
#include "shapobrot/shapobrot_common.hpp"

namespace shapobrot {

class Function {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  Plane **createPlanes(pos_t w, pos_t h) const {
    int n = numOutPlanes();

    SHAPOBROT_ASSERT(w > 0);
    SHAPOBROT_ASSERT(h > 0);
    SHAPOBROT_ASSERT(n > 0);
    Plane **planes = new Plane *[n];
    for (int i = 0; i < n; i++) {
      auto desc = getOutPlaneDesc(i);
      SHAPOBROT_ASSERT(desc.sizeOfElement > 0);
      planes[i] = new Plane(desc, w, h);
    }
    return planes;
  }

  int numOutPlanes() const { return onNumOutPlanes(); }
  const PlaneDesc &getOutPlaneDesc(int index) const {
    SHAPOBROT_ASSERT(0 <= index && index < numOutPlanes());
    return onGetOutPlaneDesc(index);
  };
  Result compute(real_t a, real_t b, void **output) {
    SHAPOBROT_ASSERT(output != nullptr);
    return onCompute(a, b, output);
  };

 protected:
  virtual int onNumOutPlanes() const = 0;
  virtual const PlaneDesc &onGetOutPlaneDesc(int index) const = 0;
  virtual Result onCompute(real_t a, real_t b, void **output) = 0;
};

class Config {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  pos_t width = 320;
  pos_t height = 240;
  real_t centerImag = -0.5;
  real_t centerReal = 0;
  float rangeLog2 = 2;
  iter_count_t maxIter = 100;
  bool swapXY = false;
  Function *function = nullptr;

  Config() = default;
  Config(Function *func) : function(func) {}

  void project(pos_t x, pos_t y, real_t *cx, real_t *cy) const {
    pos_t w = width;
    pos_t h = height;
    float dispRange = sqrt(w * w + h * h);
    real_t sceneRange = pow(2, rangeLog2);
    real_t sceneW = sceneRange * w / dispRange;
    real_t sceneH = sceneRange * h / dispRange;

    real_t offsetX = sceneW * ((real_t)x / w - 0.5f);
    real_t offsetY = sceneH * ((real_t)y / h - 0.5f);
    if (swapXY) {
      *cx = centerImag - offsetY;
      *cy = centerReal - offsetX;
    } else {
      *cx = centerImag + offsetX;
      *cy = centerReal + offsetY;
    }
  }
};

class Renderer {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  Config cfg;
  Renderer() = default;

  virtual Result init(Config &scene, Plane **outputs) {
    return Result::SUCCESS;
  }
  virtual Result service() { return Result::SUCCESS; };
};

class MandelbrotFunc : public Function {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

 protected:
  iter_count_t maxIter = 100;

  static constexpr PlaneDesc outPlaneDescs[] = {
      PlaneDesc(sizeof(iter_count_t))};

  Result onCompute(real_t a, real_t b, void **output) override {
    real_t r = 0, i = 0;
    real_t rr = 0, ii = 0;
    iter_count_t iter = 0;
    while (rr + ii < 4 && iter++ < maxIter) {
      real_t tmp = rr - ii + a;
      i = 2 * r * i + b;
      r = tmp;
      rr = r * r;
      ii = i * i;
    }

    *reinterpret_cast<iter_count_t *>(output[0]) = iter;

    return Result::SUCCESS;
  }

  int onNumOutPlanes() const override { return 1; }
  const PlaneDesc &onGetOutPlaneDesc(int index) const override {
    return outPlaneDescs[index];
  }
};

}  // namespace shapobrot
