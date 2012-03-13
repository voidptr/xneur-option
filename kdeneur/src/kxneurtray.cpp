//app header files
#include "kxneurtray.h"
#include "frmsettings.h"
#include "frmabout.h"

//Kde header files
#include <ktoolinvocation.h>

//Qt header files
#include <QDebug>
#include <QStringList>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDesktopServices>

kXneurApp::kXneurTray::kXneurTray(QObject *parent): QObject(parent)
{
  createActions();
}


void kXneurApp::kXneurTray::createActions()
{
  trayIcon = new QSystemTrayIcon();
  trayIcon->setIcon(QIcon(":/by"));
  trayMenu = new QMenu();
  user_action_menu = new QMenu();

  exit_app = new QAction(tr("Exit"), this);
  connect(exit_app, SIGNAL(triggered()), SIGNAL(exitApp()));

  about_app = new QAction(tr("kXneur About..."), this);
  connect(about_app, SIGNAL(triggered()),SLOT(kXneurAbout()));

  settings_keyboard =  new QAction(tr("Keyboard Properties..."), this);
  connect(settings_keyboard,SIGNAL(triggered()), SLOT(keyboardProperties()));

  settings_app = new QAction(tr("Properties"), this);
  connect(settings_app,SIGNAL(triggered()), SLOT(settingsApp()));

  show_journal = new QAction(tr("View log..."), this);
  connect(show_journal,SIGNAL(triggered()), SLOT(showJournal()));

  //TODO add dynamic user action from file settings
  user_action = new QAction(tr("User Action"), this);
  add_user_action_menu_from_file();
  user_action->setMenu(user_action_menu);

  //TODO ?
  start_stop_neur = new QAction(tr("Start daemon"), this);
  start_stop_neur->setData(QVariant(false));
  connect(start_stop_neur, SIGNAL(triggered()), SLOT(startStopNeur()));

  trayMenu->addAction(start_stop_neur);
  trayMenu->addSeparator();
  trayMenu->addAction(user_action);
  trayMenu->addAction(show_journal);
  trayMenu->addSeparator();
  trayMenu->addAction(settings_app);
  trayMenu->addAction(settings_keyboard);
  trayMenu->addAction(about_app);
  trayMenu->addAction(exit_app);

  trayIcon->setContextMenu(trayMenu);
  trayIcon->setToolTip(tr("X Neural Switcher running"));
  connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
  trayIcon->show();
}

void kXneurApp::kXneurTray::setTrayIconFlags(QString lang)
{
    QString path;
    path=QString("%1%2.png").arg("/usr/share/gxneur/pixmaps/").arg(lang);
    if (QFile::exists(path))
    {
        trayIcon->setIcon(QIcon(path));
    }
    else
    {
        trayIcon->setIcon(QIcon(":/noLayout"));
    }


  qDebug()<<lang;
}

void kXneurApp::kXneurTray::kXneurAbout()
{
  kXneurApp::frmAbout *formAbout = new kXneurApp::frmAbout;
  formAbout->show();
}

void kXneurApp::kXneurTray::keyboardProperties()
{
  QStringList arg;
  arg<< "--args=--tab=layouts";
  arg << "kcm_keyboard";
  KToolInvocation::kdeinitExec("kcmshell4", arg);
}

void kXneurApp::kXneurTray::settingsApp()
{
  kXneurApp::frmSettings *formSettings = new kXneurApp::frmSettings;

  if(formSettings->exec() == QDialog::Accepted)
  {
      emit restartNeur();
  }
}

void kXneurApp::kXneurTray::showJournal()
{
    //TODO лог файл может иметь расширение отличное от html
    QString logFile = QString("%1/%2").arg(QDir::homePath()).arg(".xneur/xneurlog.html");
    if (QFile::exists(logFile))
    {
       QDesktopServices::openUrl(QUrl(logFile));
    }
    else
    {
        QMessageBox::warning(0,tr("Log file not found..."),tr("Log file xneurlog.html not found. Maybe you do not have the option of logging!"));
    }
}

void kXneurApp::kXneurTray::trayClicked(QSystemTrayIcon::ActivationReason click)
{
  if(click ==QSystemTrayIcon::MiddleClick)
  {
      startStopNeur();
  }
  else if(click ==QSystemTrayIcon::Trigger)
  {
    emit nextLang();
  }
}

void kXneurApp::kXneurTray::startStopNeur()
{
//    emit statusDaemon(start_stop_neur->data().toBool());
    emit statusDaemon();

}

void kXneurApp::kXneurTray::add_user_action_menu_from_file()
{
  qDebug()<< "read settings from file for add user action";
}


void kXneurApp::kXneurTray::setTrayToolTips(QString tooltip)
{
    trayIcon->setToolTip(tooltip);
}

void kXneurApp::kXneurTray::setStatusXneur(bool status)
{
    if(status)
    {
        start_stop_neur->setText(tr("Stop daemon"));
    }
    else
    {
        start_stop_neur->setText(tr("Start daemon"));

    }
    //TODO .....
    start_stop_neur->setData(QVariant(status));
}
