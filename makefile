TARGET := acalsim

ACALSIM_VERBOSE := OFF
MT_DEBUG := OFF
NO_LOGS := OFF
BUILD_SHARED_LIBS := ON
CMAKE_FLAGS += -DBUILD_SHARED_LIBS=$(BUILD_SHARED_LIBS) -DACALSIM_VERBOSE=$(ACALSIM_VERBOSE) -DMT_DEBUG=$(MT_DEBUG) -DNO_LOGS=$(NO_LOGS)

BUILD_DIR := build
BUILD_DEBUG_DIR := $(BUILD_DIR)/debug
BUILD_RELEASE_DIR := $(BUILD_DIR)/release

MAKEFLAGS += --no-print-directory

# Compile libraries and executables
.PHONY: all debug release

all: release

debug: # Build targets for debugging
	@cmake -B $(BUILD_DEBUG_DIR) -DCMAKE_BUILD_TYPE=Debug $(CMAKE_FLAGS)
	@cmake --build $(BUILD_DEBUG_DIR) -j $(nproc) --target $(TARGET)

release: # Build targets for release
	@cmake -B $(BUILD_RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release $(CMAKE_FLAGS)
	@cmake --build $(BUILD_RELEASE_DIR) -j $(nproc)--target $(TARGET)

# Auxiliary tools
.PHONY: test regression pre-commit

test: regression

regression: # Run regression test
	@python scripts/regression.py

pre-commit: # Run pre-commit against the whole repository
	@pre-commit run --all

# Repository management
.PHONY: init clean

init: clean # Initialize the repository
	@rm -rf third-party/*/
	@git submodule update --init --recursive --depth 1
	@pre-commit install

clean: # Clean built files
	@rm -rf $(BUILD_DIR)
