#pragma once

#include "shapobrot/shapobrot_common.hpp"

namespace shapobrot {

static constexpr Types::color_t color32to16(uint32_t color) {
  uint16_t c = ((color & 0xF80000) >> 8) | ((color & 0x00FC00) >> 5) |
               ((color & 0x0000F8) >> 3);
  return ((c >> 8) & 0xff) | ((c << 8) & 0xff00);
}

class Palette {
 public:
  SHAPOBROT_IMPORT_TEMPLATE_TYPES()

  static constexpr color_t colors[] = {
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
  static constexpr int PALETTE_SIZE = sizeof(colors) / sizeof(colors[0]);
};

};  // namespace shapobrot
