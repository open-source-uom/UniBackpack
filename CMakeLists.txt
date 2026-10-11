cmake_minimum_required(VERSION 3.16)
project(unibackpack VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build the unit tests" ON)
option(UNIBACKPACK_WERROR "Treat compiler warnings as errors" OFF)

# Applied to every target defined below; Qt's headers are system includes and stay quiet
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(UNIBACKPACK_WERROR)
        add_compile_options(-Werror)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

set(CMAKE_AUTOMOC ON) # Handles Signals/Slots
set(CMAKE_AUTORCC ON) # Handles .qrc resource files
set(CMAKE_AUTOUIC ON) # Handles .ui files

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

include(GNUInstallDirs)

# Everything except the widgets, so the tests can link against it
qt_add_library(unibackpack_core STATIC
        src/Downloader.cpp
        src/PackageManager.cpp
        src/utils/language.cpp
        src/utils/loadUniversities.cpp
        src/utils/packageList.cpp

        include/Downloader.hpp
        include/PackageManager.hpp
        include/utils/language.hpp
        include/utils/loadUniversities.hpp
        include/utils/packageList.hpp
)

target_include_directories(unibackpack_core PUBLIC include)
target_link_libraries(unibackpack_core PUBLIC Qt6::Core)

qt_add_executable(unibackpack
        src/main.cpp
        src/MainWindow.cpp
        include/MainWindow.hpp
        ui/MainWindow.ui
        resources/resources.qrc
)

target_link_libraries(unibackpack PRIVATE
        unibackpack_core
        Qt6::Gui
        Qt6::Widgets
)

# Every package list under resources/lists/ is embedded as :/lists/<university>/<department>/packages.json
file(GLOB_RECURSE PACKAGE_LISTS CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        resources/lists/*.json)
qt_add_resources(unibackpack package_lists
        PREFIX "/"
        BASE resources
        FILES ${PACKAGE_LISTS})

# Strings from the core library are translated too, so lupdate scans its sources as well
get_target_property(CORE_SOURCES unibackpack_core SOURCES)
qt_add_translations(unibackpack
        TS_FILES
        translations/unibackpack_en.ts
        translations/unibackpack_el.ts
        SOURCES
        ${CORE_SOURCES}
        src/main.cpp
        src/MainWindow.cpp
        include/MainWindow.hpp
        ui/MainWindow.ui
        INCLUDE_DIRECTORIES
        include
)

install(TARGETS unibackpack RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/unibackpack.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/icons/unibackpack.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps)

if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    qt_add_executable(tst_package_manager tests/tst_package_manager.cpp)
    target_link_libraries(tst_package_manager PRIVATE unibackpack_core Qt6::Test)
    # The tests also check the bundled data files, read from the source tree
    target_compile_definitions(tst_package_manager PRIVATE
            UNIBACKPACK_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME tst_package_manager COMMAND tst_package_manager)
endif()
