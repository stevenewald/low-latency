cmake_minimum_required(VERSION 3.10)
project(engine_project LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Boost paths
set(BOOST_INCLUDE "/nix/store/hby0y11ppws9znf05x07k9apf84ac6bl-boost-1.81.0-dev/include")
set(BOOST_LIB "/nix/store/5273afwl2bjhy47kmhhv9vnx1bv76l5a-boost-1.81.0/lib")

include_directories(${BOOST_INCLUDE})
link_directories(${BOOST_LIB})

find_package(absl REQUIRED)

# Common compiler flags
add_compile_options(
  -Wall -Wextra -O3 -march=native
  -flto -mtune=native -funroll-loops -ffast-math -fomit-frame-pointer
  -fprefetch-loop-arrays -falign-functions=32 -falign-loops=32
  -fno-stack-protector -fno-math-errno -fstrict-aliasing
  -fno-semantic-interposition -finline-functions -finline-limit=1000
)

# Create tests executable from tests.cpp and engine.cpp
add_executable(tests tests.cpp engine.cpp)
target_link_libraries(tests boost_system boost_filesystem)
target_link_libraries(tests absl::base absl::throw_delegate absl::flat_hash_map)

# Custom target to run tests after building
add_custom_target(run_tests
  COMMAND tests
  DEPENDS tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Build a shared library from engine.cpp for submission
add_library(engine_shared SHARED engine.cpp)
target_link_libraries(engine_shared boost_system boost_filesystem)
target_link_libraries(engine_shared absl::base absl::throw_delegate absl::flat_hash_map)
set_target_properties(engine_shared PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Custom target for submission: copy shared library to engine.so and run lll-bench
add_custom_target(submit
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine_shared> ${CMAKE_CURRENT_SOURCE_DIR}/engine.so
  COMMAND lll-bench ${CMAKE_CURRENT_SOURCE_DIR}/build/libengine_shared.so -d 1
)

