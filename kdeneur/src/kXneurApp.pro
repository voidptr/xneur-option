QT += core gui dbus

TARGET = kXneur
TEMPLATE = app

include(i18n/i18n.pri)

SOURCES += \
    main.cpp \
    kxneurtray.cpp \
    frmabout.cpp \
    frmsettings.cpp \
    kxneur.cpp \
    xkb.c

INCLUDEPATH += /usr/include/xneur

LIBS += -lX11 -lkdeui -lkdecore -lxneur -lxnconfig

HEADERS += \
    kxneurtray.h \
    frmabout.h \
    frmsettings.h \
    tabbar.h \
    kxneur.h \
    xkb.h

FORMS += \
    frmabout.ui \
    frmsettings.ui

RESOURCES += \
    resursrc.qrc


#unix {
  #VARIABLES
 # isEmpty(PREFIX) {
  #  PREFIX = /usr
  #}

#BINDIR = $$PREFIX/bin
#DATADIR = $$PREFIX/share
#SHAREDIR = $$DATADIR/$${TARGET}
