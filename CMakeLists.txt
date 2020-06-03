cmake_minimum_required(VERSION 3.16)
project(dfatool)


set (SOURCES
    "src/main.cpp"
    "src/print.cpp"
)

add_executable(dfatool ${SOURCES})

target_compile_options(dfatool PRIVATE -march=native)
set_target_properties(dfatool PROPERTIES CXX_STANDARD 17)
