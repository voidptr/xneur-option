QT += core gui dbus

TARGET = kdeneur
TEMPLATE = app

include(../po/localize.pri)

SOURCES += \
    main.cpp \
    kxneurtray.cpp \
    frmabout.cpp \
    frmsettings.cpp \
    kxneur.cpp \
    xkb.c \
    frmaddabbreviature.cpp \
    xneurconfig.cpp \
    getnameapp.cpp \
    ruleschange.cpp \
    addrules.cpp \
    edithotkey.cpp \
    adduseraction.cpp

INCLUDEPATH += /usr/include

LIBS += -lX11 -lkdeui -lkdecore -lxneur -lxnconfig

HEADERS += \
    kxneurtray.h \
    frmabout.h \
    frmsettings.h \
    tabbar.h \
    kxneur.h \
    xkb.h \
    frmaddabbreviature.h \
    xneurconfig.h \
    getnameapp.h \
    ruleschange.h \
    addrules.h \
    edithotkey.h \
    adduseraction.h

FORMS += \
    frmabout.ui \
    frmsettings.ui \
    frmaddabbreviature.ui \
    getnameapp.ui \
    ruleschange.ui \
    addrules.ui \
    edithotkey.ui \
    adduseraction.ui

RESOURCES += \
    resursrc.qrc



unix{
 contains(QMAKE_HOST.arch, x86_64) {
    DEFINES += XNEUR_PLUGIN_DIR=\\\"/usr/lib64/xneur\\\"
 }
 else {
    DEFINES += XNEUR_PLUGIN_DIR=\\\"/usr/lib/xneur\\\"
 }
  #VARIABLES
 isEmpty(PREFIX) {
   PREFIX = /usr
 }

BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share
LOCALEDIR = $$DATADIR/locale/
SHAREDIR = $$DATADIR/$${TARGET}

target.path    = $${BINDIR}

kdeneur_data.files  = ../pixmaps/*.png
kdeneur_data.path  = $${SHAREDIR}

INSTALLS = target kdeneur_data

}

