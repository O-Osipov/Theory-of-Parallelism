# установим минимальную требуемую версию
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# название проекта, версия и испльзуемый язык
project(test_proj VERSION 0.0.0 LANGUAGES CXX)

# установим значение переменной определяющей стандарт
set(CMAKE_CXX_STANDARD 17)
# установим обязательное соответствие стандарту
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# определим конечную цель - исполняемый файл
add_executable(pr pr.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(pr PUBLIC OpenMP::OpenMP_CXX)

# cmake -S . -B ./build -G "MinGW Makefiles"
# cmake --build build