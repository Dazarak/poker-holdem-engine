cmake_minimum_required(VERSION 3.20)
project(poker-holdem-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

# Définition de l'exécutable avec tes sources + celles de l'évaluateur
add_executable(poker-holdem-engine
    src/main.cpp
    src/GameContext.cpp
    src/Holdem.cpp
    src/Player.cpp
    extern/poker-hand-evaluator/src/Card.cpp
    extern/poker-hand-evaluator/src/Engine.cpp
    extern/poker-hand-evaluator/src/HandResult.cpp
)

# Indiquer où trouver les fichiers d'en-tête (.h)
target_include_directories(poker-holdem-engine PRIVATE 
    include
    extern/poker-hand-evaluator/include
)