# User inputs :
# - CC : compiler
# - CC_FLAGS : flags for compiler
# - LINKER_FLAGS : flags for the linker
# - DEVICE : used to make folders
# - OUTPUT : output file name

SHELL = /bin/bash -O extglob

CC = gcc

CPP_FLAGS = \
	-g \
	-Wall \
	-Wextra \
	-Werror \

C_FLAGS = \
	-std=c99 \

GCOV_FLAGS = \
	--coverage

INCS = \
	-Iinclude \
	-Ilib/config/include \
	-Ilib/mp11/include \
	-Ilib/static_assert/include \
	-Ilib/type_traits/include \
	-Ilib/unpadded/include \
	-Ilib/Unity/src \
	-Itest/mock \
	-Ilib/Unity/src \

XSRCS = $(wildcard xsrc/$(DEVICE)/*.cpp)
XOBJS = $(XSRCS:xsrc/%.cpp=obj/%.o)

CHECK_LIBS = -lstdc++

all: obj/$(DEVICE)/main.o obj/$(DEVICE)/unity.o $(XOBJS)
	$(CC) $(LINKER_FLAGS) $^ -o $(OUTPUT)

check11: obj/cpp11/main.o obj/lib/unity.o
	$(CC) --coverage $^ $(CHECK_LIBS) -o run_ut11
	./run_ut11

check14: obj/cpp14/main.o  obj/lib/unity.o
	$(CC) --coverage $^ $(CHECK_LIBS) -o run_ut14
	./run_ut14

check17: obj/cpp17/main.o obj/lib/unity.o
	$(CC) --coverage $^ $(CHECK_LIBS) -o run_ut17
	./run_ut17

check20: obj/cpp20/main.o obj/lib/unity.o
	$(CC) --coverage $^ $(CHECK_LIBS) -o run_ut20
	./run_ut20

check: check11 check14 check17 check20

gcov: check
	mkdir -p gcov
	cd gcov && gcov ../test/main.cpp -m -o ../obj

clean:
	rm -f obj/cpp?(11|14|17|20)/*.?(o|gcda|gcno)

obj/cpp11/main.o: test/main.cpp
	$(CC) -std=c++11 $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp11/main.o

obj/cpp14/main.o: test/main.cpp
	$(CC) -std=c++14 $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp14/main.o

obj/cpp17/main.o: test/main.cpp
	$(CC) -std=c++17 $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp17/main.o

obj/cpp20/main.o: test/main.cpp
	$(CC) -std=c++20 $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp20/main.o
	
obj/lib/unity.o: lib/Unity/src/unity.c
	$(CC) $(C_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/main.o: test/main.cpp
	$(CC) $(CC_FLAGS) $(CPP_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/unity.o: lib/Unity/src/unity.c
	$(CC) $(CC_FLAGS) $(C_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/%.o: xsrc/$(DEVICE)/%.cpp
	$(CC) $(CC_FLAGS) $(CPP_FLAGS) $(INCS) -c $^ -o $@
