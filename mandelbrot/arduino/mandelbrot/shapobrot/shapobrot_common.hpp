#pragma once

#include <cstdint>

#define SHAPOBROT_INLINE __attribute__((always_inline)) inline

#ifdef SHAPOBROT_ENABLE_ASSERT

#include <iostream>
#include <stacktrace>

#define SHAPOBROT_ASSERT(x)                                               \
  do {                                                                    \
    if (!(x)) {                                                           \
      printf("%s [%d]: ASSERTION FAILED (%s)\n", __FILE__, __LINE__, #x); \
      fflush(stdout);                                                     \
      std::cout << std::stacktrace::current() << std::endl;               \
    }                                                                     \
  } while (false)

#else

#define SHAPOBROT_ASSERT(x) \
  do {                      \
  } while (false)

#endif

namespace shapobrot {

class Types {
 public:
  using real_t = float;
  using pos_t = int16_t;
  using iter_count_t = uint32_t;
  using color_t = uint16_t;
};

enum class Result {
  SUCCESS = 0,
};

#define SHAPOBROT_IMPORT_TEMPLATE_TYPES()   \
  using real_t = Types::real_t;             \
  using pos_t = Types::pos_t;               \
  using iter_count_t = Types::iter_count_t; \
  using color_t = Types::color_t;

}  // namespace shapobrot
