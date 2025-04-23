QT += core gui network qml quick positioning location testlib
CONFIG += c++17

TARGET = stacje_pomiarowe

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

RESOURCES += \
    qml.qrc

DISTFILES += \
    StationDialog.qml \
    main.qml \
    project.pro.user

# Konfiguracja dla testów jednostkowych
test {
    TARGET = tst_mainwindow
    SOURCES -= main.cpp
    SOURCES += tst_mainwindow.cpp
    HEADERS += mainwindow.h
    QT += testlib
    CONFIG += testcase
}
