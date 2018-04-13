#include <stdint.h>
#include <time.h>

#include <iostream>
#include <vector>

#include "kozet_fixed_point/kfp.h"
#include "kozet_fixed_point/kfp_extra.h"

void testBasic() {
  kfp::Fixed<int32_t, 16> k = 3;
  auto m = kfp::Fixed<int32_t, 16>::raw(5835);
  kfp::Fixed<int32_t, 16> x = 2457;
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
  kfp::frac32 sink;
  clock_t t1 = clock();
	do {
		kfp::sincos(i, c, s);
    // Maybe we should add explicit casting to smaller types?
		cf.underlying = c.underlying >> 14;
		sf.underlying = s.underlying >> 14;
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

int main() {
  testBasic();
  testTrig();
  testTrigPerformance();
  return 0;
}