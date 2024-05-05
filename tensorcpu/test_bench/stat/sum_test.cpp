//
// Created by tejag on 2024-04-26.
//

#include <algorithm>
#include <chrono>
#include <cmath>
#include <execution>
#include <future>
#include <iostream>
#include <limits>
#include <stdfloat>

#include "reducer.hpp"
#include "stat.hpp"
#include "tensorcpu.hpp"
#include "typed_array.hpp"

namespace stdx = std::experimental;

namespace chrono = std::chrono;
using std::chrono::steady_clock;

template <typename I>
const char *sum_stdalgo(I *out, const I *inp, uint64_t nel) {
  *out = std::reduce(
      std::execution::par_unseq, inp, inp + nel, I(0),
      [](I a, I b) { return a + b; }
  );
  return nullptr;
}

template <typename O, typename I>
void check(O out, const I *inp, uint64_t nel) {
  O res;
  for (uint64_t i = 0; i < nel; i++) {
    res += inp[i];
  }
  O diff = std::abs(res - out);
  if (diff > res * 1e-3) {
    std::cout << "Mismatch => " << res << " != " << out << "; " << diff
              << std::endl;
  }
}

int main() {
  using I = float;
  using O = float;
  const uint64_t size = 2048 * 1000;
  I *inp = new (std::align_val_t(128)) I[size];
  O out;

  for (uint64_t i = 0; i < size; i++) {
    if constexpr (std::is_floating_point<I>::value) {
      // inp[i] = drand48();
      inp[i] = I(i);
    } else {
      inp[i] = static_cast<I>(i);
    }
  }

  /*for (uint64_t i = 0; i < 3; i++) {
    sum_parsimd<O, I>(&out, inp, size);
  }*/

  steady_clock::time_point begin, end;
  Mean<double, int64_t> averageNaive, averageAutoVec, averageOptim;
  const int64_t iterations = 100;
  for(uint64_t i = 0; i < iterations; i++) {
    out = 0;
    begin = steady_clock::now();
    sum_parsimd<O, I>(&out, inp, size);
    end = steady_clock::now();
    auto timeC =
        chrono::duration_cast<chrono::microseconds>(end - begin).count();
    // std::cout << "Iteration: " << i << " Optim: " << timeC << "us sum: " << out << std::endl;
    averageOptim.consume(timeC);
    // check(out, inp, size);
    pool.printInfo();
    // std::cout << "---------" << std::endl;
  }
  std::cout << "====================" << std::endl;
  std::cout << "Average time: " << averageOptim.mean << "us" << std::endl;

  averageNaive = Mean<double, int64_t>();
  averageAutoVec = Mean<double, int64_t>();
  averageOptim = Mean<double, int64_t>();
  for (uint64_t i = 0; i < iterations; i++) {
    // std::cout << "Iteration " << i + 1 << std::endl;

    out = 0;
    begin = steady_clock::now();
    sum_stdalgo<I>(&out, inp, size);
    end = steady_clock::now();
    auto timeB =
        chrono::duration_cast<chrono::microseconds>(end - begin).count();
    averageAutoVec.consume(timeB);
    // std::cout << "stdalgo: " << timeB << "us" << " sum: " << out << std::endl;
    // averageNaive.consume(timeB);
    // check(out, inp, size);

    out = 0;
    begin = steady_clock::now();
    sum_1thread<O, I>(&out, inp, size);
    end = steady_clock::now();
    auto timeA =
        chrono::duration_cast<chrono::microseconds>(end - begin).count();
    averageNaive.consume(timeA);
    // std::cout << "1thread:   " << timeA << "us" << std::endl;
    check(out, inp, size);

    out = 0;
    begin = steady_clock::now();
    sum_parsimd<O, I>(&out, inp, size);
    end = steady_clock::now();
    auto timeC =
        chrono::duration_cast<chrono::microseconds>(end - begin).count();
    // std::cout << "Optim: " << timeC << "us sum: " << out << std::endl;
    averageOptim.consume(timeC);
    check(out, inp, size);
    pool.printInfo();
    // std::cout << "---------" << std::endl;
    // timeSum += timeA - timeB;
  }
  // std::cout << "Time diff: " << timeSum / iterations << "us" << std::endl;
  std::cout << "Average time: " << averageNaive.mean << "us" << std::endl;
  std::cout << "Average time: " << averageAutoVec.mean << "us" << std::endl;
  std::cout << "Average time: " << averageOptim.mean << "us" << std::endl;

  delete[] inp;

  pool.kill();

  return 0;
}