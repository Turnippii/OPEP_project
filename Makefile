CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
# GCC < 9 tren Linux can them: LDFLAGS := -lstdc++fs
LDFLAGS  :=

# Phat hien OS de dat ten file dich va lenh xoa
ifeq ($(OS),Windows_NT)
    EXE   := .exe
    RMDIR := rmdir /S /Q
else
    EXE   :=
    RMDIR := rm -rf
endif

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

# --- Production sources (khong co main.cpp) cho test build ---
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

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[OK] Build thanh cong: $@"

$(BUILD):
	mkdir -p $@

run: all
	$(TARGET)

# Build va chay toan bo unit test
test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) $(PROD_SRCS) | $(TEST_BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[OK] Build test thanh cong: $@"

$(TEST_BUILD):
	mkdir -p $@

clean:
	$(RMDIR) $(BUILD)
	@echo "[OK] Da xoa $(BUILD)/"
