CPPC = g++
LD_FLAGS = `pkg-config --cflags --libs gtkmm-4.0`
COMMON_CPPC_FLAGS = -std=c++23 -Wall -Wextra $(LD_FLAGS)
CPPC_FLAGS = -s -O3 $(COMMON_CPPC_FLAGS) -DRELEASE
DEBUG_FLAGS = -ggdb $(COMMON_CPPC_FLAGS)
DEBUG_ASANITIZE = -fsanitize=address $(DEBUG_FLAGS) -fno-omit-frame-pointer

SRC_PATH := src
OBJ_PATH := build/obj
BIN_PATH := build/bin
DYNAMIC_SRC_PATH := build/src
BIN_NAME = pupes_releases_tracker

# Detect build type
ifeq ($(filter debug asan,$(MAKECMDGOALS)),)
  BUILD_TYPE = release
else
  BUILD_TYPE = debug
endif

SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))

# Conditionally include UI object for release builds
ifeq ($(BUILD_TYPE),release)
  OBJ_FILES += $(OBJ_PATH)/main_window_ui.o
endif

all: make-build-dir make-dynamic-src $(BIN_PATH)/$(BIN_NAME)

debug: CPPC_FLAGS = $(DEBUG_FLAGS)
debug: make-build-dir $(BIN_PATH)/$(BIN_NAME)

asan: CPPC_FLAGS = $(DEBUG_ASANITIZE)
asan: make-build-dir $(BIN_PATH)/$(BIN_NAME)

make-build-dir:
	mkdir -p $(OBJ_PATH)
	mkdir -p $(BIN_PATH)
	mkdir -p $(DYNAMIC_SRC_PATH)

# Conditional UI resource handling
ifeq ($(BUILD_TYPE),release)
make-dynamic-src: resources/ui/main_window.ui
	@echo "[RELEASE] Embedding UI resources"
	xxd -i resources/ui/main_window.ui | head -n -1 | tr '\n' ' ' | sed 's/ };/, 0x00 };/' > $(DYNAMIC_SRC_PATH)/main_window_ui.cpp
	$(CPPC) $(CPPC_FLAGS) $(DYNAMIC_SRC_PATH)/main_window_ui.cpp -c -o $(OBJ_PATH)/main_window_ui.o
else
make-dynamic-src:
	@echo "[DEBUG] Skipping UI resource embedding"
endif

$(BIN_PATH)/$(BIN_NAME): $(OBJ_FILES)
	$(CPPC) $(CPPC_FLAGS) $^ -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CPPC) $(CPPC_FLAGS) -c $< -o $@

install:
	@install -vpm 755 -o root -g root $(BIN_PATH)/$(BIN_NAME) /usr/bin/

clean:
	rm -fr build

.PHONY: all clean install debug asan make-build-dir make-dynamic-src
