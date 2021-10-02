SHELL = /bin/bash -O extglob

cpp_flags = \
	-g \
	-Wall \
	-Wextra \
	-Werror \
  -fmax-errors=50 \
	-Iinclude \
	-Ilib/Unity/src \
	-Ilib/config/include \
	-Ilib/mp11/include \
	-Ilib/static_assert/include \
	-Ilib/type_traits/include \

c_flags = \
	-std=c99 \
	-Ilib/Unity/src \

libs = \
	-lstdc++ \

.SILENT: clean

check11: obj/cpp11/main.o obj/cpp11/tuple.o obj/cpp11/unaligned_data.o obj/lib/unity.o
	gcc $^ $(libs) -o run_ut11
	./run_ut11

check17: obj/cpp17/main.o obj/cpp17/tuple.o obj/cpp17/unaligned_data.o obj/lib/unity.o
	gcc $^ $(libs) -o run_ut17
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
