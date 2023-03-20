cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(branchbound "src/branchbound.cpp" "include/branchbound.h")
install (TARGETS branchbound DESTINATION "branchbound/out/build")