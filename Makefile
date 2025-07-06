CPPC = g++
MAIN_CFLAGS = `pkg-config --cflags gtkmm-4.0`
MAIN_LIBS = `pkg-config --libs gtkmm-4.0`
SYSTRAY_LD_FLAGS = `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1`
COMMON_CPPC_FLAGS = -std=c++23 -Wall -Wextra -Wno-write-strings -Wno-int-to-pointer-cast
CPPC_FLAGS = -s -O3 $(COMMON_CPPC_FLAGS) -DRELEASE
DEBUG_FLAGS = -ggdb $(COMMON_CPPC_FLAGS)
DEBUG_ASANITIZE = -fsanitize=address $(DEBUG_FLAGS) -fno-omit-frame-pointer

SRC_PATH := src
SYSTRAY_SRC_PATH := systray_src
OBJ_PATH := build/obj
BIN_PATH := build/bin
DYNAMIC_SRC_PATH := build/src
BIN_NAME = pupes_releases_tracker
SYSTRAY_BIN_NAME = pupes_releases_tracker_systray

ifeq ($(filter debug asan,$(MAKECMDGOALS)),)
BUILD_TYPE = release
else
# i am too lazy to write code for debug conditional compilation
BUILD_TYPE = release
endif

SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))

RESOURCE_FILES := resources/ui/main_window.ui resources/ui/add_item_window.ui resources/ui/about.ui resources/img/icon.svg $(BIN_PATH)/$(SYSTRAY_BIN_NAME)

resource_to_cpp = $(DYNAMIC_SRC_PATH)/$(basename $(notdir $1))_resource.cpp
resource_to_obj = $(OBJ_PATH)/$(basename $(notdir $1))_resource.o

RESOURCE_CPP_FILES := $(foreach file,$(RESOURCE_FILES),$(call resource_to_cpp,$(file)))
RESOURCE_OBJ_FILES := $(foreach file,$(RESOURCE_FILES),$(call resource_to_obj,$(file)))

ifeq ($(BUILD_TYPE),release)
OBJ_FILES += $(RESOURCE_OBJ_FILES)
endif

all: make-dynamic-src $(BIN_PATH)/$(BIN_NAME) | make-build-dir

debug: CPPC_FLAGS = $(DEBUG_FLAGS)
debug: make-build-dir $(BIN_PATH)/$(BIN_NAME)

asan: CPPC_FLAGS = $(DEBUG_ASANITIZE)
asan: make-build-dir $(BIN_PATH)/$(BIN_NAME)

make-build-dir:
	mkdir -p $(OBJ_PATH)
	mkdir -p $(BIN_PATH)
	mkdir -p $(DYNAMIC_SRC_PATH)

ifeq ($(BUILD_TYPE),release)

make-dynamic-src: $(RESOURCE_CPP_FILES) | make-build-dir $(BIN_PATH)/$(SYSTRAY_BIN_NAME)

# Generate explicit rules for each resource file
define make-resource-rule
$(call resource_to_cpp,$1): $1 | make-build-dir
	xxd -i $$< | tr '\n' ' ' | sed 's/ };/, 0x00 };/' > $$@

$(call resource_to_obj,$1): $(call resource_to_cpp,$1) | make-build-dir
	$$(CPPC) $$(CPPC_FLAGS) $$(MAIN_CFLAGS) -c $$< -o $$@

endef

$(foreach res,$(RESOURCE_FILES),$(eval $(call make-resource-rule,$(res))))

else

make-dynamic-src:
	@echo "Skipped resource embedding"

endif

$(BIN_PATH)/$(BIN_NAME): $(OBJ_FILES) | make-build-dir
	$(CPPC) $(CPPC_FLAGS) $^ $(MAIN_LIBS) -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp | make-build-dir
	$(CPPC) $(CPPC_FLAGS) $(MAIN_CFLAGS) -c $< -o $@

$(BIN_PATH)/$(SYSTRAY_BIN_NAME): $(SYSTRAY_SRC_PATH)/systray.cpp
	$(CPPC) $(CPPC_FLAGS) $< $(SYSTRAY_LD_FLAGS) -o $@

install:
	@install -vpm 755 -o root -g root $(BIN_PATH)/$(BIN_NAME) /usr/bin/

clean:
	rm -fr build

.PHONY: all clean install debug asan
