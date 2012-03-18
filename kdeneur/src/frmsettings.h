#ifndef FRMSETTINGS_H
#define FRMSETTINGS_H
//app headre files
#include "frmaddabbreviature.h"


//Qt header files
#include <QDialog>
#include <QAbstractButton>

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
    explicit frmSettings(QWidget *parent = 0);
    ~frmSettings();

  private slots:
    void Clicked(QAbstractButton *);


    //tab abbreviations
    void addAbbreviation();

    //tab properties
    void RecoverKeyboardCommand();
    void EditKeyboardCommand();
    void TypeIconTray(int);
    void TypeEngine(int);
    void BrowseIconTray();
    void chekAutostart(bool);
    void delayStartApp(int);


  private:
    Ui::frmSettings *ui;
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
    void readSettings();
  };
}
#endif // FRMSETTINGS_H
