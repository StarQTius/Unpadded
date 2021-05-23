cpp_flags = \
	-std=c++11 \
	-g \
	--coverage \
	-Wall \
	-Wextra \
	-Werror \
	-Iinclude \
	-Ilib/config/include \
	-Ilib/mp11/include \
	-Ilib/static_assert/include \
	-Ilib/type_traits/include \
	-Ilib/Unity/src \

c_flags = \
	-std=c99 \
	-Ilib/Unity/src \

libs = \
	-lstdc++

check: obj/main.o obj/lib/unity.o
	gcc --coverage $^ $(libs) -o run_ut
	./run_ut

gcov: check
	mkdir -p gcov
	cd gcov && gcov ../test/main.cpp -m -o ../obj

clean:
	rm obj/*.o obj/*.gcda obj/*.gcno

obj/main.o: test/main.cpp
	gcc $(cpp_flags) -DUT_ONLY -c $^ -o obj/main.o

obj/lib/unity.o:
	gcc $(c_flags) -c lib/Unity/src/unity.c -o obj/lib/unity.o
