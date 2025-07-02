CPPC = g++
LD_FLAGS = `pkg-config --cflags --libs gtkmm-4.0`
COMMON_CPPC_FLAGS = -std=c++23 -Wall -Wextra $(LD_FLAGS)
CPPC_FLAGS =  -s -O3 $(COMMON_CPPC_FLAGS)
DEBUG_FLAGS = -ggdb $(COMMON_CPPC_FLAGS)
DEBUG_ASANITIZE = -fsanitize=address -ggdb -fno-omit-frame-pointer $(COMMON_CPPC_FLAGS)


SRC_PATH := src
OBJ_PATH := build/obj
BIN_PATH := build/bin
BIN_NAME = pupes_releases_tracker


SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))


all: make-build-dir $(BIN_PATH)/$(BIN_NAME)


debug: CPPC_FLAGS = $(DEBUG_FLAGS)
debug: make-build-dir $(BIN_PATH)/$(BIN_NAME)

asan: CPPC_FLAGS = $(DEBUG_ASANITIZE)
asan: make-build-dir $(BIN_PATH)/$(BIN_NAME)


make-build-dir:
	    mkdir -p $(OBJ_PATH)
	    mkdir -p $(BIN_PATH)


$(BIN_PATH)/$(BIN_NAME): $(OBJ_FILES)
	    $(CPPC) $(CPPC_FLAGS) $^ -o $@


$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	    $(CPPC) $(CPPC_FLAGS) -c $< -o $@


install:
	    @install -vpm 755 -o root -g root $(BIN_PATH)/$(BIN_NAME) /usr/bin/

clean:
	    rm -fr build

.PHONY: all clean install debug asan
