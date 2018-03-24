#include <stdint.h>

#include <iostream>
#include <vector>

#include "kozet_fixed_point/kfp.h"

std::vector<std::pair<int32_t, int32_t>> testValues = {
  {35, 35},
  {1'000'000, 10'000},
  {90000, 90000},
  {-80000, -80000},
};
std::vector<std::pair<int64_t, int64_t>> testValues64 = {
  {35, 35},
  {1'000'000, 10'000},
  {90000, 90000},
  {-80000, -80000},
  {5'000'000'000L, 5'000'000'000L},
  {-10'000'000'000L, -10'000'000'000L},
};

void testMulOverflow() {
  for (const auto& p : testValues) {
    auto q = kfp::mulOverflow(p.first, p.second);
    std::cout << p.first << ", " << p.second << " -> ";
    std::cout << q.first << ", " << q.second;
    std::cout << " (" << (((uint64_t) q.first << 32) + q.second) << ")\n";
  }
  for (const auto& p : testValues) {
    auto q = kfp::mulOverflow((uint32_t) p.first, (uint32_t) p.second);
    std::cout << (uint32_t) p.first << ", " << (uint32_t) p.second << " -> ";
    std::cout << q.first << ", " << q.second;
    std::cout << " (" << (((uint64_t) q.first << 32) + q.second) << ")\n";
  }
  for (const auto& p : testValues64) {
    auto q = kfp::mulOverflow(p.first, p.second);
    std::cout << p.first << ", " << p.second << " -> ";
    std::cout << q.first << ", " << q.second;
    std::cout << " (too big to count)\n";
  }
  for (const auto& p : testValues64) {
    auto q = kfp::mulOverflow((uint64_t) p.first, (uint64_t) p.second);
    std::cout << (uint64_t) p.first << ", " << (uint64_t) p.second << " -> ";
    std::cout << q.first << ", " << q.second;
    std::cout << " (too big to count)\n";
  }
}

int main() {
  testMulOverflow();
  kfp::Fixed<int32_t, 16> k = 3;
  auto m = kfp::Fixed<int32_t, 16>::raw(5835);
  std::cout << "3 = " << k << "\n";
  std::cout << "5835/65536 = " << m << "\n";
  std::cout << k << " + " << m << " = " << (k + m) << "\n";
  std::cout << k << " - " << m << " = " << (k - m) << "\n";
  std::cout << k << " * " << m << " = " << (k * m) << "\n";
  std::cout << k << " / " << m << " = " << (k / m) << "\n";
  return 0;
}