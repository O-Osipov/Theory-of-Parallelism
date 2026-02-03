cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

# Переменная типа с возможными значениями Float или Double
set(NUM_TYPE "Float" CACHE STRING "Numeric type: Float or Double")

# Ограничиваем возможные значения (чтобы не напортачили)
set_property(CACHE NUM_TYPE PROPERTY STRINGS Float Double)

# Исполняемый файл из одного исходника
add_executable(my_app main.cpp)

if(NUM_TYPE STREQUAL "Float")
    target_compile_definitions(my_app PRIVATE USE_FLOAT)
elseif(NUM_TYPE STREQUAL "Double")
    target_compile_definitions(my_app PRIVATE USE_DOUBLE)
endif()


# (опционально, но рекомендую)
target_compile_features(my_app PRIVATE cxx_std_17)
