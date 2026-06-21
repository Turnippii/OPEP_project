cmake_minimum_required(VERSION 3.16)
project(OPEP_Qt_GUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(OPEP_CORE_SOURCES
    src/core/record.cpp
    src/task/task.cpp
    src/task/task_manager.cpp
    src/money/transaction.cpp
    src/money/wallet.cpp
    src/money/budget.cpp
    src/data/encryption.cpp
    src/data/user.cpp
    src/data/auth.cpp
)

add_executable(opep_gui
    gui/main_gui.cpp
    gui/appservice.h
    gui/appservice.cpp
    gui/loginwindow.h
    gui/loginwindow.cpp
    gui/mainwindow.h
    gui/mainwindow.cpp
    ${OPEP_CORE_SOURCES}
)

target_include_directories(opep_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(opep_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if (WIN32)
    set_target_properties(opep_gui PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
