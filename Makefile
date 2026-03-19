BUILD_DIR := build
BIN       := $(BUILD_DIR)/bin/circa-solem

.PHONY: all configure build run test clean distclean ephemeris help

all: build

$(BUILD_DIR)/CMakeCache.txt:
	cmake -B $(BUILD_DIR)

configure: $(BUILD_DIR)/CMakeCache.txt

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR)

run: build
	$(BIN)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	cmake --build $(BUILD_DIR) --target clean

distclean:
	rm -rf $(BUILD_DIR)

ephemeris:
	bash scripts/download-ephemeris.sh

help:
	@echo "Targets:"
	@echo "  build       Configure and compile (default)"
	@echo "  run         Build and launch the simulation"
	@echo "  test        Build and run the test suite"
	@echo "  clean       Remove compiled objects (keeps CMake config)"
	@echo "  distclean   Remove the entire build directory"
	@echo "  ephemeris   Download JPL DE440 ephemeris data (~83 MB)"
	@echo "  configure   Run CMake configuration only"
