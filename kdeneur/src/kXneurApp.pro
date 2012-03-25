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
    getnameapp.cpp

INCLUDEPATH += /usr/include/xneur

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
    getnameapp.h

FORMS += \
    frmabout.ui \
    frmsettings.ui \
    frmaddabbreviature.ui \
    getnameapp.ui

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
#}
