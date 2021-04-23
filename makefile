cpp_flags = \
	-std=c++11 \
	-Iinclude \
	-Ilib/Unity/src \
	-Ilib/ct_magic/include

c_flags = \
	-std=c99 \
	-Ilib/Unity/src

check: obj/main.o obj/endianess.o obj/unity.o
	gcc $^ -o run_ut
	./run_ut

clean:
	rm obj/*

obj/main.o: test/main.cpp
	gcc $(cpp_flags) -DUT_ONLY -c $^ -o obj/main.o

obj/endianess.o: test/endianess.cpp
	gcc $(cpp_flags) -c $^ -o obj/endianess.o

obj/unity.o:
	gcc $(c_flags) -c lib/Unity/src/unity.c -o obj/unity.o
