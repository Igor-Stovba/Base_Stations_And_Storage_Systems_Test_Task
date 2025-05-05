cmake_minimum_required(VERSION 3.10)
project(club LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} src/main.cpp 
                               src/input_info.cpp)

add_subdirectory(tests)

# It ensures that club_tests can safely use binary club
add_dependencies(club_tests ${PROJECT_NAME})