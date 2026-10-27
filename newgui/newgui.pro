QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connectpage.cpp \
    main.cpp \
    mainwindow.cpp \
    ../utils.cpp \
    ../net.cpp \
    ../net_host.cpp \
    waitpage.cpp

HEADERS += \
    connectpage.h \
    mainwindow.h \
    ../card.h \
    ../net.h \
    waitpage.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    newgui_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

DEFINES += EMBEDDED_SERVER

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
