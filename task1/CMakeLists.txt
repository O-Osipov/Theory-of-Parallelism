cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

# Переменная типа с возможными значениями Float или Double
set(NUM_TYPE "Float" CACHE STRING "Numeric type: Float or Double")

# Ограничиваем возможные значения (чтобы не напортачили)
set_property(CACHE NUM_TYPE PROPERTY STRINGS Float Double)

# Проверка значения
if(NOT NUM_TYPE STREQUAL "Float" AND NOT NUM_TYPE STREQUAL "Double")
    message(FATAL_ERROR "NUM_TYPE must be Float or Double")
endif()

# Исполняемый файл из одного исходника
add_executable(my_app main.cpp)

# Передаём значение в код через define
target_compile_definitions(my_app PRIVATE NUM_TYPE=${NUM_TYPE})

# (опционально, но рекомендую)
target_compile_features(my_app PRIVATE cxx_std_17)
