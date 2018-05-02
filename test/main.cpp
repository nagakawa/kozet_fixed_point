/*
   Copyright 2018 AGC.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>

#include "kozet_fixed_point/kfp.h"
#include "kozet_fixed_point/kfp_extra.h"

void testBasic() {
  using namespace kfp::literals;
  kfp::Fixed<int32_t, 16> k = 3;
  auto m = kfp::Fixed<int32_t, 16>::raw(5835);
  kfp::Fixed<int32_t, 16> x = 2457;
  kfp::Fixed<int64_t, 32> y = x;
  kfp::Fixed<int32_t, 16> z = "5.26"_s16_16;
  kfp::Fixed<int32_t, 16> w = "735"_s16_16;
  std::cout << "3 = " << k << "\n";
  std::cout << "5835/65536 = " << m << "\n";
  std::cout << k << " + " << m << " = " << (k + m) << "\n";
  std::cout << k << " - " << m << " = " << (k - m) << "\n";
  std::cout << k << " * " << m << " = " << (k * m) << "\n";
  std::cout << k << " / " << m << " = " << (k / m) << "\n";
  std::cout << k << " > 0 = " << (k > 0) << "\n";
  std::cout << k << " < 0 = " << (k < 0) << "\n";
  std::cout << "-" << k << " = " << -k << "\n";
  std::cout << "(-m) >> 4 = " << ((-m) >> 4) << "\n";
  std::cout << "-(m >> 4) = " << -(m >> 4) << "\n";
  std::cout << x << " / " << m << " = " << (x / m) << "\n";
  std::cout << x << " = " << y << "\n";
  std::cout << "w = " << w << "\n";
  std::cout << "z = " << z << "\n";
}

void testTrig() {
  std::cout << "Fixed-point function test: trigonometry\n";
	kfp::s2_30 c, s;
  kfp::s16_16 cf, sf, r;
  kfp::frac32 t, i;
	do {
		kfp::sincos(i, c, s);
    std::cout << i << " -> (" << c << ", " << s;
    // Maybe we should add explicit casting to smaller types?
		cf.underlying = c.underlying >> 14;
		sf.underlying = s.underlying >> 14;
		kfp::rectp(cf, sf, r, t);
    std::cout << ") -> " << r << " angle " << t << "\n";
		/*
			Setting the increment lower and disabling the printing functions,
			it is estimated that 0x1000000 sincos and rectp calls take 2.538
			seconds or 152 frames. As long as you don't spawn more than about
			10,000 bullets, you probably shouldn't have much problem with this,
			even when accounting for other things that are going on.
		*/
    i += kfp::frac32::raw(0x1000000);
  } while (i != kfp::frac32(0));
}

void testTrigPerformance() {
  std::cout << "Fixed-point function test: trigonometry performance\n";
	kfp::s2_30 c, s;
  kfp::s16_16 cf, sf, r;
  kfp::frac32 t, i;
  kfp::frac32 sink = 0;
  clock_t t1 = clock();
	do {
		kfp::sincos(i, c, s);
    cf = (kfp::s16_16) c;
    sf = (kfp::s16_16) s;
		kfp::rectp(cf, sf, r, t);
    sink += t;
    i += kfp::frac32::raw(0x100);
  } while (i != kfp::frac32(0));
  std::cout << "sink = " << sink << "\n";
  clock_t t2 = clock();
  clock_t elapsed = t2 - t1;
  double elapsedSec = ((double) elapsed) / CLOCKS_PER_SEC;
  std::cout << "0x1000000 operations take "
    << elapsedSec << "s\n";
  std::cout << "(" << (elapsedSec / 0x1000000 * 1e9) << "ns per operation)\n"; 
}

void testSqrtPerformance() {
  std::cout << "Testing sqrt performance\n";
  std::cout << "Using " << (sizeof(int) * CHAR_BIT) << "-bit ints\n";
  srand(time(nullptr));
  int sink = 0;
  clock_t t1 = clock();
  for (size_t i = 0; i < 1000000; ++i) {
    sink += kfp::sqrti(rand());
  }
  std::cout << "sink = " << sink << "\n";
  clock_t t2 = clock();
  clock_t elapsed = t2 - t1;
  double elapsedSec = ((double) elapsed) / CLOCKS_PER_SEC;
  std::cout << "sqrti: 1000000 operations take " << elapsedSec << "s\n";
  //
  t1 = clock();
  for (size_t i = 0; i < 1000000; ++i) {
    sink += kfp::sqrtiFast(rand());
  }
  std::cout << "sink = " << sink << "\n";
  t2 = clock();
  elapsed = t2 - t1;
  elapsedSec = ((double) elapsed) / CLOCKS_PER_SEC;
  std::cout << "sqrtiFast: 1000000 operations take " << elapsedSec << "s\n";
}

int main() {
  testBasic();
  testTrig();
  testTrigPerformance();
  testSqrtPerformance();
  return 0;
}