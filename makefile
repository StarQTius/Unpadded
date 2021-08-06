# User inputs :
# - CC : compiler
# - CC_FLAGS : flags for compiler
# - CPPSTD : version of the standard (-std=CPPSTD)
# - LINKER_FLAGS : flags for the linker
# - DEVICE : used to make folders
# - OUTPUT : output file name

SHELL = /bin/bash -O extglob

CC = gcc

OPT_FLAGS = -O2

DEBUG_FLAGS = -g

CPP_FLAGS = \
	-Wall \
	-Wextra \
	-Werror \
	-Wno-array-bounds \
	-Wno-stringop-overread \

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
	$(CC) -std=c++11 $(DEBUG_FLAGS) $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp11/main.o

obj/cpp14/main.o: test/main.cpp
	$(CC) -std=c++14 $(DEBUG_FLAGS) $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp14/main.o

obj/cpp17/main.o: test/main.cpp
	$(CC) -std=c++17 $(DEBUG_FLAGS) $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp17/main.o

obj/cpp20/main.o: test/main.cpp
	$(CC) -std=c++20 $(DEBUG_FLAGS) $(CPP_FLAGS) $(INCS) -DUT_ONLY -c $^ -o obj/cpp20/main.o

obj/lib/unity.o: lib/Unity/src/unity.c
	$(CC) $$(DEBUG_FLAGS) $(C_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/main.o: test/main.cpp
	$(CC) -std=$(CPPSTD) $(OPT_FLAGS) $(CC_FLAGS) $(CPP_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/unity.o: lib/Unity/src/unity.c
	$(CC) $(OPT_FLAGS) $(CC_FLAGS) $(C_FLAGS) $(INCS) -c $^ -o $@

obj/$(DEVICE)/%.o: xsrc/$(DEVICE)/%.cpp
	$(CC) -std=$(CPPSTD) $(OPT_FLAGS) $(CC_FLAGS) $(CPP_FLAGS) $(INCS) -c $^ -o $@
