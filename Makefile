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

ifeq ($(filter debug asan,$(MAKECMDGOALS)),)
BUILD_TYPE = release
else
BUILD_TYPE = debug
endif

SRC_FILES := $(shell find $(SRC_PATH) -name '*.cpp')
OBJ_FILES := $(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRC_FILES))

RESOURCE_FILES := resources/ui/main_window.ui resources/ui/add_item_window.ui resources/ui/about.ui resources/img/icon.svg

resource_to_cpp = $(DYNAMIC_SRC_PATH)/$(basename $(notdir $1))_resource.cpp
resource_to_obj = $(OBJ_PATH)/$(basename $(notdir $1))_resource.o

RESOURCE_CPP_FILES := $(foreach file,$(RESOURCE_FILES),$(call resource_to_cpp,$(file)))
RESOURCE_OBJ_FILES := $(foreach file,$(RESOURCE_FILES),$(call resource_to_obj,$(file)))

ifeq ($(BUILD_TYPE),release)
OBJ_FILES += $(RESOURCE_OBJ_FILES)
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

ifeq ($(BUILD_TYPE),release)

make-dynamic-src: $(RESOURCE_CPP_FILES) | make-build-dir

# Generate explicit rules for each resource file
define make-resource-rule
$(call resource_to_cpp,$1): $1
	xxd -i $$< | tr '\n' ' ' | sed 's/ };/, 0x00 };/' > $$@

$(call resource_to_obj,$1): $(call resource_to_cpp,$1)
	$$(CPPC) $$(CPPC_FLAGS) -c $$< -o $$@
endef

$(foreach res,$(RESOURCE_FILES),$(eval $(call make-resource-rule,$(res))))

else

make-dynamic-src:
	@echo "Skipped resource embedding"

endif

$(BIN_PATH)/$(BIN_NAME): $(OBJ_FILES) | make-build-dir
	$(CPPC) $(CPPC_FLAGS) $^ -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CPPC) $(CPPC_FLAGS) -c $< -o $@

install:
	@install -vpm 755 -o root -g root $(BIN_PATH)/$(BIN_NAME) /usr/bin/

clean:
	rm -fr build

.PHONY: all clean install debug asan make-build-dir make-dynamic-src
