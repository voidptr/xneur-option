#include "frmsettings.h"
#include "ui_frmsettings.h"

//Qt header files
#include <QDebug>
#include <QFileDialog>


kXneurApp::frmSettings::frmSettings(QWidget *parent) :  QDialog(parent),  ui(new Ui::frmSettings)
{
  ui->setupUi(this);

  config = new KConfig;
  general = config->group("General");
  layouts = config->group("Layouts");
  hotkeys =config->group("Hotkeys");
  autocompletion = config->group("Autocompletion");
  applications = config->group("Applications");
  notifications = config->group("Notifications");
  Abbreviations = config->group("Abbreviations");
  log = config->group("Log");
  troubleshooting = config->group("Troubleshooting");
  advanced = config->group("Advanced");
  plugins = config->group("Plugins");
  properties = config->group("Properties");

  createConnect();
}

kXneurApp::frmSettings::~frmSettings()
{
  delete ui;
}

void kXneurApp::frmSettings::Clicked(QAbstractButton *button)
{
  if(ui->cmdBox->standardButton(button) == QDialogButtonBox::Ok)
  {
      done(QDialog::Accepted);
      config->sync();
      this->close();
  }
  else
  {
      done(QDialog::Rejected);
      this->close();
  }
}

void kXneurApp::frmSettings::createConnect()
{
  connect(ui->cmdBox, SIGNAL(clicked(QAbstractButton*)), SLOT(Clicked(QAbstractButton*)));
  connect(ui->tabProperties_cmdRecoverKeyCommand, SIGNAL(clicked()), SLOT(RecoverKeyboardCommand()));
  connect(ui->tabProperties_cmdEditKeyCommand, SIGNAL(clicked()),SLOT(EditKeyboardCommand()));
  connect(ui->tabProperties_cmbTypeIconTray, SIGNAL(activated(int)),SLOT(TypeIconTray(int)));
  connect(ui->tabProperties_cmdBrowseIconTray, SIGNAL(clicked()),SLOT(BrowseIconTray()));
  connect(ui->tabProperties_chkEnableAutostart, SIGNAL(clicked(bool)), SLOT(chekAutostart(bool)));

}

void kXneurApp::frmSettings::RecoverKeyboardCommand()
{
    //TODO Save in file setting
    ui->tabProperties_txtKeyboardCommad->clear();
    ui->tabProperties_txtKeyboardCommad->setText("/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard");
     properties.writeEntry("KeyboardCommand", "/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard");
}

void kXneurApp::frmSettings::EditKeyboardCommand()
{
    //TODO Save in file setting
    ui->tabProperties_txtKeyboardCommad->setText(QFileDialog::getOpenFileName(0,tr("Select execute file"), "/usr/bin"));
    if(!ui->tabProperties_txtKeyboardCommad->text().isEmpty())
    {
      properties.writeEntry("KeyboardCommand", ui->tabProperties_txtKeyboardCommad->text());
    }
}


void kXneurApp::frmSettings::TypeIconTray(int selectIndex)
{
    switch(selectIndex)
    {
    case 0:
        ui->tabProperties_grpFolderIcon->setEnabled(true);
        break;
    default:
        ui->tabProperties_grpFolderIcon->setEnabled(false);
        break;
    }
    properties.writeEntry("Typeicontray", selectIndex);
}

void kXneurApp::frmSettings::BrowseIconTray()
{
    ui->tabProperties_txtPathIconTray->setText(QFileDialog::getOpenFileName(0,tr("Select folder with image for icon tray kdeNeur"),"" ));
    if(! ui->tabProperties_txtPathIconTray->text().isEmpty())
    {
      properties.writeEntry("Iconpath",  ui->tabProperties_txtPathIconTray->text());
    }
}


void kXneurApp::frmSettings::chekAutostart(bool cheked)
{
    if(cheked)
    {
      //TODO copy file .desktop in ~/.config/autstart
    }
    else
    {

    }
    properties.writeEntry("Autostart",  cheked);
    ui->tabProperties_chkEnableAutostart->setEnabled(cheked);
}
