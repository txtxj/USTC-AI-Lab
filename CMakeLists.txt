cmake_minimum_required(VERSION 3.22)
project(AI_Lab_1_A_star)
project(AI_Lab_1_CSP)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-DDIJKSTRA)
#add_definitions(-DUNADMISSIBLE)

add_executable(AI_Lab_1_A_star "A star/src/main.cpp")
add_executable(AI_Lab_1_CSP "CSP/src/main.cpp")