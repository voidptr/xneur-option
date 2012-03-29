#ifndef FRMSETTINGS_H
#define FRMSETTINGS_H
//app headre files
#include "frmaddabbreviature.h"
#include "xneurconfig.h"
#include "getnameapp.h"
#include "ruleschange.h"

//Qt header files
#include <QDialog>
#include <QAbstractButton>
#include <QListWidget>


//Kde header files
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

namespace Ui
{
  class frmSettings;
}

namespace kXneurApp
{
  class frmSettings : public QDialog
  {
    Q_OBJECT

  public:
    explicit frmSettings(QWidget *parent = 0,kXneurApp::xNeurConfig *cfg=0);
    ~frmSettings();

  private slots:
    void Clicked(QAbstractButton *);
    //tab layout
    void addApp_OneLayout();
    void removeApp_OneLayout();
    void rulesChange();

    //tab abbreviations
    void addAbbreviation();

    //tab properties
    void RecoverKeyboardCommand();
    void EditKeyboardCommand();
    void TypeIconTray(int);
    //void TypeEngine(int);
    void BrowseIconTray();
    void chekAutostart(bool);
    void delayStartApp(int);


  private:
    Ui::frmSettings *ui;
    void saveSettingsNeur();
    QStringList getListFromWidget(QListWidget *);

    xNeurConfig *cfgNeur;
    KConfig *config;
    KConfigGroup general;
    KConfigGroup layouts;
    KConfigGroup hotkeys;
    KConfigGroup autocompletion;
    KConfigGroup applications;
    KConfigGroup notifications;
    KConfigGroup Abbreviations;
    KConfigGroup log;
    KConfigGroup troubleshooting;
    KConfigGroup advanced;
    KConfigGroup plugins;
    KConfigGroup properties;
    void settintgGrid();
    void createConnect();
    void readSettingsKdeNeur();
    void readSettingsNeur();

    void tab_lay_get_list_lang(QStringList);
    void tab_lay_get_list_app(QStringList);
    QHash<QString, bool> tab_lay_save_list_lang();

    void hot_get_list_hotkeys(QMap<QString, QString>);
    void hot_get_list_user_actions(QMap<QString, QMap<QString, QString> >);

    void notif_get_list_action_sound(QMap<QString, QMultiMap<QString, QString> >);
    void notif_get_list_action_osd(QMap<QString, QMultiMap<QString, QString> >);
    void notif_get_list_action_popup(QMap<QString, QMultiMap<QString, QString> >);

    void abbr_get_list_abbreviations(QMap <QString, QString>);

    void plug_get_list_plugins(QMap<QString, QMultiMap<bool, QString> >);


    void auto_get_list_app_autocomp(QStringList);
  };
}
#endif // FRMSETTINGS_H
