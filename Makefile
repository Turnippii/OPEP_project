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

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[OK] Build thanh cong: $@"

$(BUILD):
	mkdir -p $@

run: all
	$(TARGET)

clean:
	$(RMDIR) $(BUILD)
	@echo "[OK] Da xoa $(BUILD)/"
