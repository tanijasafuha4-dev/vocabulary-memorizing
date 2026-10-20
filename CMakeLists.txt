cmake_minimum_required(VERSION 3.16)

project(ShanXueXi VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 支持 Qt6 或 Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

set(PROJECT_SOURCES
    src/main.cpp
    src/core/Models.h
    src/core/JsonHelper.h
    src/core/EbbinghausEngine.h
    src/core/EbbinghausEngine.cpp
    src/core/QuizEngine.h
    src/core/QuizEngine.cpp
    src/core/CheckinManager.h
    src/core/CheckinManager.cpp
    src/core/AchievementEngine.h
    src/core/AchievementEngine.cpp
    src/core/StorageManager.h
    src/core/StorageManager.cpp
    src/bridge/AppController.h
    src/bridge/AppController.cpp
    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ShanXueXi
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
    set_property(TARGET ShanXueXi APPEND PROPERTY
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
else()
    if(ANDROID)
        add_library(ShanXueXi SHARED
            ${PROJECT_SOURCES}
        )
        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE INTERNAL "")
    else()
        add_executable(ShanXueXi
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(ShanXueXi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bridge
)

target_link_libraries(ShanXueXi PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
)

if(${QT_VERSION_MAJOR} EQUAL 6)
    qt_finalize_executable(ShanXueXi)
endif()

# 独立的纯 C++ 核心测试程序（无需 Qt 也可单独编译验证 C++ 算法）
option(BUILD_CORE_TESTS "Build pure C++ core console test runner" OFF)
if(BUILD_CORE_TESTS)
    add_executable(ShanXueXi_CoreTest
        tests/core_test.cpp
        src/core/EbbinghausEngine.cpp
        src/core/QuizEngine.cpp
        src/core/CheckinManager.cpp
        src/core/AchievementEngine.cpp
        src/core/StorageManager.cpp
    )
    target_include_directories(ShanXueXi_CoreTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    )
endif()
