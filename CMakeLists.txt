cmake_minimum_required(VERSION 3.16)
project(UniBackpack VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_AUTOMOC ON) # Handles Signals/Slots
set(CMAKE_AUTORCC ON) # Handles .qrc resource files
set(CMAKE_AUTOUIC ON) # Handles .ui files
set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)
set(RESOURCE_FILES resources/icons/resources.qrc)

add_executable(UniBackpack
    # Source files
    src/main.cpp
    src/Downloader.cpp
    src/MainWindow.cpp

    # Header files
    include/MainWindow.hpp
    include/Downloader.hpp

    #UI
    ui/MainWindow.ui

    # Resources
    resources/resources.qrc
)

target_include_directories(UniBackpack PRIVATE include)

target_link_libraries(UniBackpack PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
