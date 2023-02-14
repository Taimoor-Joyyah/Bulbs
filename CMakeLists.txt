cmake_minimum_required(VERSION 3.24)
project(Bulb)

set(CMAKE_CXX_STANDARD 20)

set(raylib_dir "C:\\Users\\Muhammad Al Fahad\\CLionProjects\\libraries\\raylib-4.2.0")

include_directories(${raylib_dir}\\include)
link_directories(${raylib_dir}\\lib)

add_executable(Bulb main.cpp)
target_link_libraries(${PROJECT_NAME} raylib winmm gdi32 opengl32)
