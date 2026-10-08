cmake_minimum_required(VERSION 3.20)
project(poker-holdem-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

# Inclusion du sous-module évaluateur
add_subdirectory(extern/poker-hand-evaluator)

# Définition de l'exécutable du moteur
add_executable(poker-holdem-engine
    src/main.cpp
)

# Liaison avec la bibliothèque de l'évaluateur
target_link_libraries(poker-holdem-engine PRIVATE poker-hand-evaluator)