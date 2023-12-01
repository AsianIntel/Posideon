cmake_minimum_required(VERSION 3.26)
project(Posideon)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp"  "src/*.h")

add_executable(Posideon ${SOURCES})
target_include_directories(Posideon PUBLIC src)
target_compile_definitions(Posideon PRIVATE POSIDEON_ASSERTS)