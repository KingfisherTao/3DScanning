QT += core gui serialport sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS

TARGET              = 3DScanning
TEMPLATE            = app
MOC_DIR             = temp/moc
RCC_DIR             = temp/rcc
UI_DIR              = temp/ui
OBJECTS_DIR         = temp/obj
DESTDIR             = bin
win32:RC_FILE       = main.rc
PRECOMPILED_HEADER  = myhelper.h

include($$PWD/qextserialport/qextserialport.pri)

HEADERS += $$files(./*.h)
SOURCES += $$files(./*.cpp)
FORMS += $$files(./*.ui)

RESOURCES += \
    main.qrc
