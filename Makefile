# ===== COMPILER =====
CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude

# ===== DIRECTORIES =====
BUILD_DIR := build
SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/apps
TARGET := order_gen

# ===== SOURCES =====
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# ===== DEFAULT =====
all: $(APP_DIR)/$(TARGET)

# ===== COMPILE =====
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# ===== LINK =====
$(APP_DIR)/$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

# ===== DEPENDENCIES =====
-include $(DEPS)

# ===== MODES =====
debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

# ===== UTILS =====
info:
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"
	@echo "DEPS=$(DEPS)"

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all debug release clean info
