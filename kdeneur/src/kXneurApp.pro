QT += core gui dbus

TARGET = kdeneur
TEMPLATE = app

include(i18n/i18n.pri)

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
    addrules.cpp

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
    addrules.h

FORMS += \
    frmabout.ui \
    frmsettings.ui \
    frmaddabbreviature.ui \
    getnameapp.ui \
    ruleschange.ui \
    addrules.ui

RESOURCES += \
    resursrc.qrc



#unix {
#XNEURPLUGINDIR = /usr/lib/xneur
  #VARIABLES
 # isEmpty(PREFIX) {
  #  PREFIX = /usr
#}

#BINDIR = $$PREFIX/bin
#DATADIR = $$PREFIX/share
#SHAREDIR = $$DATADIR/$${TARGET}
#}
