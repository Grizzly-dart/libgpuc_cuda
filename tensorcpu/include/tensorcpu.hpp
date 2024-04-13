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

enum BinaryOp : uint8_t {
  Plus,
  Minus,
  Mul,
  Div,
  Pow,
};

enum UnaryOp : uint8_t { Log, Exp, Sin, Cos, Tan, Sinh, Cosh, Tanh };

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

template <typename O, typename I, BinaryOp op>
extern void tcBinaryArith(
    O *out, I *inp1, I *inp2, uint64_t nel, uint8_t flip, Dim2 i2broadcaster
);

template <typename O, typename I, BinaryOp op>
extern void tcPlusSlow(
    void *out, void *inp1, void *inp2, uint64_t nel, uint8_t flip,
    Dim2 i2broadcaster, uint8_t outTID, uint8_t i1TID, uint8_t i2TID
);

template <typename O, typename I, UnaryOp op>
extern void tcTrignometry(O *out, I *inp1, uint64_t nel);

#endif // TENSORC_H
