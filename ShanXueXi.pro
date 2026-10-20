QT += core gui qml quick quickcontrols2

CONFIG += c++17

TARGET = ShanXueXi
TEMPLATE = app

INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/core \
    $$PWD/src/bridge

SOURCES += \
    src/main.cpp \
    src/core/EbbinghausEngine.cpp \
    src/core/QuizEngine.cpp \
    src/core/CheckinManager.cpp \
    src/core/AchievementEngine.cpp \
    src/core/StorageManager.cpp \
    src/bridge/AppController.cpp

HEADERS += \
    src/core/Models.h \
    src/core/JsonHelper.h \
    src/core/EbbinghausEngine.h \
    src/core/QuizEngine.h \
    src/core/CheckinManager.h \
    src/core/AchievementEngine.h \
    src/core/StorageManager.h \
    src/bridge/AppController.h

RESOURCES += \
    resources.qrc

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}
