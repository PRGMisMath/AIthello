cmake_minimum_required(VERSION 4.0)
project(GameBoardMac)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -g")


include_directories(src)

include_directories(src/nl)
set(NL_SOURCES
        src/nl/NLBackprop.cpp
        src/nl/NLBackprop.h
        src/nl/NLDebug.cpp
        src/nl/NLDebug.h
        src/nl/NLLayer.cpp
        src/nl/NLLayer.h
        src/nl/NLNetwork.cpp
        src/nl/NLNetwork.h
)

include_directories(src/utils)
set(UTILS_SOURCES
        src/utils/Terminal.cpp
        src/utils/Terminal.hpp
        src/utils/Random.cpp
        src/utils/Random.hpp
        src/utils/StringView.hpp
        src/utils/StringView.cpp
)

include_directories(src/game)
set(GAME_SOURCES
        src/game/GameBoard.hpp
        src/game/GameBoardManager.cpp
        src/game/GameBoardManager.hpp
        src/game/GamePlayer.hpp
        src/game/Othello.cpp
        src/game/Othello.hpp
        src/game/OthelloSave.cpp
        src/game/OthelloSave.hpp
)

include_directories(src/tree)
set(TREE_SOURCES
        src/tree/PreferTree.cpp
        src/tree/PreferTree.hpp
        src/tree/MinMaxTree.cpp
        src/tree/MinMaxTree.hpp
        src/tree/GameTree.cpp
        src/tree/GameTree.hpp
        src/tree/GameTreeNode.cpp
        src/tree/GameTreeNode.hpp
        src/tree/MiniOthello.hpp
        src/tree/MiniOthello.cpp
)

include_directories(src/train)
set(TRAIN_SOURCES
        src/train/NLTrainManager.cpp
        src/train/NLTrainManager.hpp
        src/train/OthelloTrain.cpp
        src/train/OthelloTrain.hpp
        src/utils/Benchmark.cpp
        src/utils/Benchmark.hpp
)

include_directories(src/graphics)
set(GRAPH_SOURCES
        src/graphics/OthelloUI.hpp
        src/graphics/GameView.cpp
        src/graphics/GameView.hpp
)

set(SOURCES ${TRAIN_SOURCES} ${TREE_SOURCES} ${GAME_SOURCES} ${UTILS_SOURCES} ${NL_SOURCES})

set(IS_GRAPH 1)
set(IS_GEN 0)

if (${IS_GRAPH})
    set(FULL_SOURCES ${SOURCES} ${GRAPH_SOURCES} src/main/graph_main.cpp)
elseif (${IS_GEN})
    set(FULL_SOURCES ${SOURCES} src/main/gen_main.cpp)
else ()
    set(FULL_SOURCES ${SOURCES} src/main/main.cpp)
endif ()

add_executable(GameBoardApp ${FULL_SOURCES})

if (${IS_GRAPH})
    find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)
    target_link_libraries(GameBoardApp PRIVATE SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System)

    find_package(TGUI CONFIG REQUIRED)
    include_directories(${SFML_INCLUDE_DIRS})
    target_link_libraries(GameBoardApp PRIVATE TGUI::TGUI)

    target_compile_options(GameBoardApp PRIVATE -Wall -Wextra)
endif ()
