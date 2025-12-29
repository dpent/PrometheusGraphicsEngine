# CMakeList.txt : CMake project for Prometheus, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Prometheus")

set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Add source to this project's executable.
add_executable (Prometheus ${SOURCES})

target_include_directories(Prometheus PUBLIC
    "C:/Users/Dimitris/Documents/Libraries/VulkanSDK/1.4.335.0/Include"
    "C:/Users/Dimitris/Documents/Libraries/glfw-3.4/include"
    "C:/Users/Dimitris/Documents/Libraries/glm1.0.2"
)

target_link_libraries(Prometheus PUBLIC
    "C:/Users/Dimitris/Documents/Libraries/VulkanSDK/1.4.335.0/Lib/vulkan-1.lib"
    "C:/Users/Dimitris/Documents/Libraries/glfw-3.4/lib-vc2022/glfw3.lib"
)

target_compile_features(Prometheus PUBLIC cxx_std_23)
target_compile_definitions(Prometheus PRIVATE BIN_DIR="${BIN_DIR}")

# TODO: Add tests and install targets if needed.
