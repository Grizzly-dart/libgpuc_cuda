#ifndef TENSORC_H
#define TENSORC_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

struct Dim2 {
  uint32_t r;
  uint32_t c;

  [[nodiscard]] uint64_t nel() const { return r * c; }
};

struct Dim3 {
  uint32_t ch;
  uint32_t r;
  uint32_t c;

  [[nodiscard]] Dim2 toDim2() { return {r, c}; };

  [[nodiscard]] uint64_t nel() const { return ch * r * c; }
};

enum ArithMode : uint8_t {
  ewise,
  rwise,
  scalar,
};

typedef enum PadMode : uint8_t {
  CONSTANT,
  CIRCULAR,
  REFLECT,
  REPLICATION
} PadMode;

extern void tcFree(void *ptr);
extern void *tcRealloc(void *ptr, uint64_t size);
extern void tcMemcpy(void *dst, void *src, uint64_t size);

#ifdef __cplusplus
}
#endif

template <typename O, typename I1, typename I2>
extern void tcPlus(
    O *out, I1 *inp1, I2 *inp2, uint64_t nel, uint8_t flip, Dim2 i2broadcaster
);

#endif // TENSORC_H
