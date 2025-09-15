CXX = g++
CFLAGS = -std=c++23 -O2 -g -Iheaders
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

CPP_DIR = cpps
BUILD_DIR = ../build

SRCS = $(wildcard $(CPP_DIR)/*.cpp) main.cpp
# Put all .o files directly in build/, no subdirectories
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SRCS)))

TARGET = $(BUILD_DIR)/Prometheus

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Generic rule to compile .cpp -> .o in build/
$(BUILD_DIR)/%.o: $(CPP_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@	

# Rule for main.cpp separately
$(BUILD_DIR)/main.o: main.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

# Build executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	# Delete object files after linking
	rm -f $(OBJS)

.PHONY: clean run

run: $(TARGET)
	$(TARGET)

clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)
