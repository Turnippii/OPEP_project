CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
# GCC < 9 tren Linux can them: LDFLAGS := -lstdc++fs
LDFLAGS  :=

# Phát hiện OS để đặt extension file đích (.exe tren Windows)
# Yêu cầu shell POSIX: Git Bash / MSYS2 tren Windows, hoac bash/sh tren Linux/macOS
ifeq ($(OS),Windows_NT)
    EXE := .exe
else
    EXE :=
endif

# rm -rf hoat dong tren Git Bash, MSYS2, Linux, macOS
# (rmdir /S /Q chi dung duoc trong CMD — khong dung o day)
RMDIR := rm -rf

BUILD  := build
TARGET := $(BUILD)/opep$(EXE)

SRCS := main.cpp \
        src/core/record.cpp \
        src/task/task.cpp \
        src/task/task_manager.cpp \
        src/money/transaction.cpp \
        src/money/wallet.cpp \
        src/money/budget.cpp \
        src/data/encryption.cpp \
        src/data/user.cpp \
        src/data/auth.cpp \
        src/ui/input_validator.cpp \
        src/ui/menu.cpp \
        src/ui/dashboard.cpp

# --- Production sources (không có main.cpp) cho test build ---
PROD_SRCS := src/core/record.cpp \
             src/task/task.cpp \
             src/task/task_manager.cpp \
             src/money/transaction.cpp \
             src/money/wallet.cpp \
             src/money/budget.cpp \
             src/data/encryption.cpp \
             src/data/user.cpp \
             src/data/auth.cpp \
             src/ui/input_validator.cpp \
             src/ui/menu.cpp \
             src/ui/dashboard.cpp

# --- Test sources ---
TEST_BUILD  := $(BUILD)/tests
TEST_TARGET := $(TEST_BUILD)/run_tests$(EXE)

TEST_SRCS := tests/run_all_tests.cpp \
             tests/test_record.cpp \
             tests/test_task.cpp \
             tests/test_task_manager.cpp \
             tests/test_transaction.cpp \
             tests/test_wallet.cpp \
             tests/test_budget.cpp \
             tests/test_encryption.cpp \
             tests/test_auth.cpp \
             tests/test_input_validator.cpp \
             tests/test_polymorphism.cpp

.PHONY: all clean run test gui run-gui

all: $(TARGET)

$(TARGET): $(SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[OK] Build thành công: $@"

$(BUILD):
	mkdir -p $@

run: all
	$(TARGET)

# Build và chạy toàn bộ unit test
test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) $(PROD_SRCS) | $(TEST_BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[OK] Build test thành công: $@"

$(TEST_BUILD):
	mkdir -p $@

# Build GUI Qt bằng CMake.
# Yêu cầu đã cài Qt Widgets và CMake trong MSYS2/Qt Creator.
gui:
	cmake -S . -B build-gui
	cmake --build build-gui

run-gui: gui
	./build-gui/opep_gui$(EXE)

clean:
	$(RMDIR) $(BUILD) build-gui
	@echo "[OK] Đã xóa $(BUILD)/ và build-gui/"
