cpp_flags = \
	-std=c++11 \
	-g \
	-Wall \
	-Wextra \
	-Werror \
	-Iinclude \
	-Ilib/Unity/src \
	-Ilib/ct_magic/include

c_flags = \
	-std=c99 \
	-Ilib/Unity/src

libs = \
	-lstdc++

check: obj/main.o obj/unity.o
	gcc $^ $(libs) -o run_ut
	./run_ut

clean:
	rm obj/*

obj/main.o: test/main.cpp
	gcc $(cpp_flags) -DUT_ONLY -c $^ -o obj/main.o

obj/unity.o:
	gcc $(c_flags) -c lib/Unity/src/unity.c -o obj/unity.o
