cmake_minimum_required(VERSION 3.10)
project(engine_project LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ENABLE_PGO ON)
set(PGO_PROFILE ON)

if(ENABLE_PGO)
  if(PGO_PROFILE)
    message(STATUS "Building with PGO optimization (using collected profile data)")
    add_compile_options(-fprofile-use)
    add_link_options(-fprofile-use)
    add_link_options(-fprofile-generate)
  else()
    message(STATUS "Building with PGO instrumentation")
    add_compile_options(-fprofile-generate)
    add_link_options(-fprofile-generate)
  endif()
endif()

find_package(absl REQUIRED)
find_package(Boost REQUIRED)
find_package(ring-span-lite REQUIRED)
find_package(tsl-ordered-map REQUIRED)

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
target_link_libraries(tests boost::boost)
target_link_libraries(tests absl::base absl::throw_delegate absl::flat_hash_map nonstd::ring-span-lite)
target_link_libraries(tests tsl::ordered_map)

# Custom target to run tests after building
add_custom_target(run_tests
  COMMAND tests
  DEPENDS tests
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Build a shared library from engine.cpp for submission
add_library(engine_shared SHARED engine.cpp)
target_link_libraries(engine_shared boost::boost)
target_link_libraries(engine_shared absl::base absl::throw_delegate absl::flat_hash_map nonstd::ring-span-lite tsl::ordered_map)
set_target_properties(engine_shared PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Custom target for submission: copy shared library to engine.so and run lll-bench
add_custom_target(submit
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine_shared> ${CMAKE_CURRENT_SOURCE_DIR}/engine.so
  COMMAND lll-bench ${CMAKE_CURRENT_SOURCE_DIR}/libengine_shared.so -d 1
)

