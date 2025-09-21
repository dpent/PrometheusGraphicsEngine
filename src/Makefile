CXX = g++
CFLAGS = -std=c++23 -O2 -g -Iengine/headers -Iobjects/headers #-fsanitize=address
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

BUILD_DIR = ../build
TARGET = $(BUILD_DIR)/Prometheus

# Recursively find all .cpp files in the src directory and subdirectories
SRCS = $(shell find . -name "*.cpp" -type f)

# Generate object file names in build directory (flatten the structure)
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

# Create a mapping from source files to object files for proper dependencies
# This handles the case where files with same name exist in different directories
define cpp-to-obj
$(BUILD_DIR)/$(notdir $(1:.cpp=.o)): $(1) | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c $$< -o $$@
endef

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Generate rules for each source file
$(foreach src,$(SRCS),$(eval $(call cpp-to-obj,$(src))))

# Build executable and clean up object files
all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking..."
	@$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	@echo "Cleaning up object files..."
	@rm -f $(OBJS)
	@echo "==============================BUILD COMPLETE=============================="
	@echo "Executable: $(TARGET) is now running..."

.PHONY: clean run all

run: $(TARGET)
	@$(TARGET)

clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

# Debug target to show what files were found
debug:
	@echo "Source files found:"
	@echo $(SRCS)
	@echo "Object files to be created:"
	@echo $(OBJS)