cmake_minimum_required(VERSION 3.21)

project(StalzoneUtility VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Добавили Svg
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network Svg)

add_executable(StalzoneUtility
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/api/StalzoneAPI.cpp
    src/api/StalzoneAPI.h
    src/core/GameDatabase.cpp
    src/core/GameDatabase.h
    src/core/ThemeManager.cpp   # <--- НОВОЕ
    src/core/ThemeManager.h     # <--- НОВОЕ
    src/ui/pages/AuctionWindow.cpp
    src/ui/pages/AuctionWindow.h
    src/ui/pages/BarterWindow.cpp
    src/ui/pages/BarterWindow.h
    src/ui/pages/ItemsWindow.cpp
    src/ui/pages/ItemsWindow.h
    resources.qrc
)

target_include_directories(StalzoneUtility PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(StalzoneUtility PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    Qt6::Svg               # <--- НОВОЕ
)

if(MSVC)
    target_compile_options(StalzoneUtility PRIVATE /utf-8)
endif()
