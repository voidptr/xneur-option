//xneur header files
extern "C"
{
    #include <xneur/xneur.h>
    #include <xneur/xnconfig.h>
    #include <xneur/list_char.h>
}

#define XNEUR_NEEDED_MAJOR_VERSION 15
#define XNEUR_BUILD_MINOR_VERSION 0

//app header files
#include "kxneur.h"

//Kde header files
#include <ktoolinvocation.h>
#include <kstandarddirs.h>

//Qt header files
#include <QDBusConnection>
#include <QDebug>

extern "C"
{
#include "xkb.h"
}

Display *kXneurApp::kXneur::dpy=NULL;

kXneurApp::kXneur::kXneur(int& argc, char **argv): QApplication (argc, argv)
{
    xconfig = NULL;
    dpy=XOpenDisplay(NULL);
    settignsTray();
    init_libxnconfig();
    xneur_pid=xconfig->get_pid(xconfig);
    if(xneur_pid < 0)
    {
        xneurStart();
        running =false;
    }
    else
    {
        procxNeurStart();
        running = true;
        qDebug()<<"xneur is running";
    }
    emit changeIconTray(QString("%1").arg(xconfig->handle->languages[get_active_kbd_group(dpy)].dir));
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts","currentLayoutChanged", this, SLOT(layoutChanged(QString)));
    qDebug()<<"CURRENT KEYBOARD LAYOUT " << get_active_kbd_group(dpy) << "LANG NAME " << xconfig->handle->languages[get_active_kbd_group(dpy)].dir;


}

kXneurApp::kXneur::~kXneur()
{
    xneurStop();
    XCloseDisplay(dpy);
}

void kXneurApp::kXneur::settignsTray()
{
    trayApp = new kXneurTray(this);
    procxNeur = new QProcess();
    connect(procxNeur, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(procxNeurStop(int,QProcess::ExitStatus)));
    connect(procxNeur, SIGNAL(started()), SLOT(procxNeurStart()));
    connect(this, SIGNAL(changeIconTray(QString)), trayApp, SLOT(setTrayIconFlags(QString)));
    connect(trayApp, SIGNAL(exitApp()), SLOT(quit()));
    connect(trayApp, SIGNAL(nextLang()), SLOT(nextLang()));
    //connect(trayApp, SIGNAL(statusDaemon(bool)), SLOT(startStopNeur(bool)));
    connect(trayApp, SIGNAL(statusDaemon()), SLOT(startStopNeur()));
    connect(trayApp, SIGNAL(restartNeur()), SLOT(restartNeur()));
}

void kXneurApp::kXneur::layoutChanged(QString lang)
{
    emit changeIconTray(lang.left(2));
}

//void kXneurApp::kXneur::startStopNeur(bool status)
void kXneurApp::kXneur::startStopNeur()
{
    if(xconfig->get_pid(xconfig)>0)
    {
        xneurStop();
    }
    else
    {
       xneurStart();
    }

//    if(status)
//    {
//        xneurStop();
//    }
//    else
//    {
//        xneurStart();
//    }
}

bool kXneurApp::kXneur::xneurStart()
{
    if ( !init_libxnconfig())
    {
        qDebug("ERROR: start xNeur Fail [not init libxnconfig]");
        return false;
    }
    xneur_pid = xconfig->get_pid(xconfig);
    if ( xneur_pid > 0 )
    {
        if (!xneurStop())
        {
            return false;
        }
    }
    procxNeur->start("xneur",QIODevice::ReadWrite);
    xneur_pid = procxNeur->pid();
    if ( xneur_pid > 0 )
    {
        init_libxnconfig();
        return true;
    }
    qDebug() << tr("ERROR: start xNeur Fail") /*<< error*/;
    xneur_pid = 0;
    qDebug("start -- fail 3\n");
    return false;
}

bool kXneurApp::kXneur::xneurStop()
{
    if ( xneur_pid > 0 )
    {
        if ( xconfig->kill(xconfig) )
        {
            xconfig->set_pid(xconfig, 0);
        }
        else
        {
           qDebug()<< "ERROR: Fail xNeur stopped [don't kill xconfig']";
           return false;
        }
        xneur_pid = 0;
        //TODO
        if(running)
        {
           procxNeurStop(0,QProcess::NormalExit);
        }
        return true;
    }
    qDebug()<< "MSG: xNeur isn't running";
    return false;
}

void kXneurApp::kXneur::nextLang()
{
 //TODO
    set_next_kbd_group(dpy);
}

void kXneurApp::kXneur::restartNeur()
{
    xconfig->reload(xconfig);
}

bool kXneurApp::kXneur::init_libxnconfig()
{
    if (xconfig != NULL)
    {
        return true;
    }

    if((xconfig = xneur_config_init())==NULL)
    {
        qDebug()<<"ERROR: lib init fail";
        return false;
    }
    int major_version, minor_version;
    xconfig->get_library_version(&major_version, &minor_version);
    if ((major_version != XNEUR_NEEDED_MAJOR_VERSION) || (minor_version != XNEUR_BUILD_MINOR_VERSION))
    {
        qDebug()<<QString(tr("Wrong XNeur configuration library api version.\nPlease, install libxnconfig version 0.%1.%2")).arg(XNEUR_NEEDED_MAJOR_VERSION).arg(XNEUR_BUILD_MINOR_VERSION);
        xconfig->uninit(xconfig);
        exit(1);
    }
    qDebug()<<QString(tr("Using libxnconfig API version 0.%1.%2 (build with 0.%3.%4)")).arg(major_version).arg(minor_version).arg(XNEUR_NEEDED_MAJOR_VERSION).arg(XNEUR_BUILD_MINOR_VERSION);
    if (!xconfig->load(xconfig))
    {
        qDebug()<< tr("XNeur's config broken or was created with old version!\nPlease, remove ~/.xneur/. It should solve the problem!\nIf you don't want to loose your configuration, back it up\nand manually patch new configuration which will be created after first run.");
        xconfig->uninit(xconfig);
        exit(1);
    }
  return true;
}

void kXneurApp::kXneur::procxNeurStop(int exitcode,QProcess::ExitStatus exitstatus)
{
  trayApp->setStatusXneur(false);
  qDebug()<<"MSG: xNeur stopped:" << " ExitCode " << exitcode << " ExitStatus " << exitstatus;
  if(exitstatus >0)
  {
      qDebug()<< tr("ERROR: Warning process xNeur crashed, please look log file and inform the author xNeur. Thank You!");
  }
}

void kXneurApp::kXneur::procxNeurStart()
{
    qDebug()<<"MSG: xNeur started.";
  trayApp->setStatusXneur(true);
}












