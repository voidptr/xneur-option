extern "C"
{
    #include <xneur/xneur.h>
    #include <xneur/xnconfig.h>
    #include <xneur/list_char.h>
}



#include "xneurconfig.h"

#define XNEUR_NEEDED_MAJOR_VERSION 15
#define XNEUR_BUILD_MINOR_VERSION 0

extern "C"
{
    #include "xkb.h"
}

Display *kXneurApp::xNeurConfig::dpy=NULL;

kXneurApp::xNeurConfig::xNeurConfig(QObject *parent) :    QObject(parent)
{
    xconfig = NULL;
    init_libxnconfig();
    xneur_pid = xconfig->get_pid(xconfig);
    dpy=XOpenDisplay(NULL);
    procxNeur = new QProcess();
    connect(procxNeur, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(procxNeurStop(int,QProcess::ExitStatus)));
    connect(procxNeur, SIGNAL(started()), SLOT(procxNeurStart()));

}

kXneurApp::xNeurConfig::~xNeurConfig()
{
    XCloseDisplay(dpy);
}

bool kXneurApp::xNeurConfig::init_libxnconfig()
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

bool kXneurApp::xNeurConfig::xneurStop()
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
//        if(running)
//        {
//           procxNeurStop(0,QProcess::NormalExit);
//        }
        return true;
    }
    qDebug()<< "MSG: xNeur isn't running";
    return false;

    //

}

bool kXneurApp::xNeurConfig::xneurStart()
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

int kXneurApp::xNeurConfig::getNeur_pid()
{
    return xconfig->get_pid(xconfig);
}

QString kXneurApp::xNeurConfig::getCurrentLang()
{
    return QString("%1").arg(xconfig->handle->languages[get_active_kbd_group(dpy)].dir);
}

void kXneurApp::xNeurConfig::setNextLang()
{
    set_next_kbd_group(dpy);
}

void kXneurApp::xNeurConfig::restartNeur()
{
   xconfig->reload(xconfig);
}

void kXneurApp::xNeurConfig::procxNeurStop(int exitcode, QProcess::ExitStatus exitstatus)
{
  emit setStatusXneur(false);
  qDebug()<<"MSG: xNeur stopped:" << " ExitCode " << exitcode << " ExitStatus " << exitstatus;
  if(exitstatus >0)
  {
      qDebug()<< tr("ERROR: Warning process xNeur crashed, please look log file and inform the author xNeur. Thank You!");
  }
}

void kXneurApp::xNeurConfig::procxNeurStart()
{
  qDebug()<<"MSG: xNeur started.";
  emit setStatusXneur(true);
}

