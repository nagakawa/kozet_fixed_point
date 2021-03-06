CPP=c++ -Iinclude/ -I/usr/include/ --std=c++14
CFLAGS=-Wall -Werror -pedantic -Og -g
CFLAGS_RELEASE=-Wall -Werror -pedantic -O3 -march=native

all: build/test

build/test: test/main.cpp \
		include/kozet_fixed_point/kfp.h \
		include/kozet_fixed_point/kfp_extra.h \
		include/kozet_fixed_point/kfp_random.h
	@mkdir -p build
	@echo -e '\e[33mCompiling test program...\e[0m'
	@$(CPP) --std=c++14 test/main.cpp -o build/test $(CFLAGS_RELEASE)
	@echo -e '\e[32mDone!\e[0m'

clean:
	rm build/test
