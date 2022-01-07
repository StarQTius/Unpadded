SHELL = /bin/bash -O extglob

cpp_flags = \
	-g \
	-Wall \
	-Wextra \
	-Werror \
  -fmax-errors=50 \
	-Iinclude \
	-Ilib/unpadded/include \
	-Ilib/Unity/src \
	-Ilib/config/include \
	-Ilib/mp11/include \
	-Ilib/static_assert/include \
	-Ilib/type_traits/include \
	-Ilib/expected/include

c_flags = \
	-std=c99 \
	-Ilib/Unity/src \

srcs = $(wildcard test/*.cpp)
unity_srcs = lib/Unity/src/unity.c

lib_objs = $(unity_srcs:lib/Unity/src/%.c=obj/lib/%.o)
cpp11_objs = $(srcs:test/%.cpp=obj/cpp11/%.o) $(lib_objs)
cpp17_objs = $(srcs:test/%.cpp=obj/cpp17/%.o) $(lib_objs)

libs = \
	-lstdc++ \

.SILENT: clean

check11: $(cpp11_objs)
	gcc $(cpp_flags) $^ $(libs) -o run_ut11
	./run_ut11

check17: $(cpp17_objs)
	gcc $(cpp_flags) $^ $(libs) -o run_ut17
	./run_ut17

check: check11 check17

clean:
	rm -vf obj/cpp?(11|17)/*.o

mrproper: clean
	git submodule deinit -f --all

obj/cpp11/%.o: test/%.cpp
	gcc -std=c++11 $(cpp_flags) -DUT_ONLY -c $^ -o $@

obj/cpp17/%.o: test/%.cpp
	gcc -std=c++17 $(cpp_flags) -DUT_ONLY -c $^ -o $@

obj/lib/unity.o: lib/Unity/src/unity.c
	gcc $(c_flags) -c $^ -o $@
