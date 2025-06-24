#pragma once

#include <cstdint>
#include <cstring>

#include "shapobrot/shapobrot_common.hpp"

namespace shapobrot {

struct PlaneDesc {
  int sizeOfElement = 0;

  PlaneDesc() = default;
  constexpr PlaneDesc(int size) : sizeOfElement(size) {}
};

class Plane {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  PlaneDesc desc;
  pos_t width = 0;
  pos_t height = 0;
  size_t stride = 0;
  uint64_t *data = nullptr;

  Plane(PlaneDesc desc)
      : desc(desc), width(0), height(0), stride(0), data(nullptr) {
    SHAPOBROT_ASSERT(this->desc.sizeOfElement > 0);
  }

  Plane(PlaneDesc desc, pos_t w, pos_t h, size_t stride = 0)
      : width(w),
        height(h),
        desc(desc),
        stride((stride > 0) ? stride : ((size_t)w * desc.sizeOfElement)),
        data(new uint64_t[(this->stride * h + 7) / 8]) {
    SHAPOBROT_ASSERT(this->desc.sizeOfElement > 0);
    SHAPOBROT_ASSERT(this->width > 0);
    SHAPOBROT_ASSERT(this->height > 0);
    SHAPOBROT_ASSERT(this->stride > 0);
  }

  ~Plane() {
    if (data != nullptr) delete[] data;
  }

  void clear() { memset(data, 0, sizeInBytes()); }

  SHAPOBROT_INLINE size_t sizeInBytes() const { return stride * height; }

  void setSize(pos_t w, pos_t h, size_t stride = 0) {
    SHAPOBROT_ASSERT(desc.sizeOfElement > 0);

    stride = (stride > 0) ? stride : ((size_t)w * desc.sizeOfElement);

    SHAPOBROT_ASSERT(w > 0);
    SHAPOBROT_ASSERT(h > 0);
    SHAPOBROT_ASSERT(stride > 0);

    size_t newSize = stride * h;
    if (newSize != sizeInBytes() || data == nullptr) {
      if (data != nullptr) delete[] data;
      data = new uint64_t[(newSize + 7) / 8];
    }

    this->width = w;
    this->height = h;
    this->stride = stride;
  }

  template <typename T>
  SHAPOBROT_INLINE T *lineAt(pos_t y) {
    SHAPOBROT_ASSERT(0 <= y && y < height);
    auto ptr = reinterpret_cast<uint8_t *>(data) + (size_t)y * stride;
    // SHAPOBROT_ASSERT((uint8_t *)ptr - (uint8_t *)data == (size_t)y * stride);
    return reinterpret_cast<T *>(ptr);
  }

  template <typename T>
  SHAPOBROT_INLINE T &pixelAt(pos_t x, pos_t y) {
    SHAPOBROT_ASSERT(0 <= x && x < width);
    auto ptr = lineAt<uint8_t>(y) + (size_t)x * desc.sizeOfElement;
    // SHAPOBROT_ASSERT((uint8_t *)ptr - (uint8_t *)data == (size_t)y * stride +
    // (size_t)x * desc.sizeOfElement);
    return reinterpret_cast<T &>(*ptr);
  }
};

}  // namespace shapobrot
