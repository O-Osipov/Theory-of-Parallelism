cmake_minimum_required(VERSION 3.10)
project(MatVecParallel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(matvec main.cpp)
target_link_libraries(matvec PRIVATE pthread) # Необходимо для std::thread