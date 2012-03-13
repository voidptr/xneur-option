#ifndef KXNEURTRAY_H
#define KXNEURTRAY_H
//Qt header files
#include <QSystemTrayIcon>
#include <QObject>
#include <QAction>
#include <QMenu>

namespace kXneurApp
{
  class kXneurTray : public QObject
  {
    Q_OBJECT
  public:
    explicit kXneurTray(QObject *parent=0);
    void  setTrayToolTips(QString);
  public slots:
    void setTrayIconFlags(QString);
    void kXneurAbout();
    void keyboardProperties();
    void showJournal();
    void settingsApp();
    void startStopNeur();
    void trayClicked(QSystemTrayIcon::ActivationReason);
    void setStatusXneur(bool);
  private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QMenu *user_action_menu;
    QAction *start_stop_neur;
    QAction *user_action;
    QAction *show_journal;
    QAction *settings_app;
    QAction *settings_keyboard;
    QAction *about_app;
    QAction *exit_app;
    void createActions();
    void add_user_action_menu_from_file();
signals:
    void statusDaemon(bool);
    void statusDaemon();
    void restartNeur();
    void nextLang();
    void exitApp();
  };
}
#endif // KXNEURTRAY_H
