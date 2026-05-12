# установим минимальную требуемую версию
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# название проекта, версия и испльзуемый язык
project(test_proj VERSION 0.0.0 LANGUAGES CXX)

# установим значение переменной определяющей стандарт
set(CMAKE_CXX_STANDARD 17)
# установим обязательное соответствие стандарту
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# определим конечную цель - исполняемый файл 1
add_executable(main2 main2.cpp)
add_executable(main1 main1.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(main2 PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(main1 PUBLIC OpenMP::OpenMP_CXX)

# cmake -S . -B ./build -G "MinGW Makefiles"
# cmake --build build