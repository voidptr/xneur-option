#include "frmsettings.h"
#include "ui_frmsettings.h"


#include <QDebug>
#include <QFileDialog>
kXneurApp::frmSettings::frmSettings(QWidget *parent) :  QDialog(parent),  ui(new Ui::frmSettings)
{
  ui->setupUi(this);
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

}

void kXneurApp::frmSettings::RecoverKeyboardCommand()
{
    //TODO Save in file setting
    ui->tabProperties_txtKeyboardCommad->clear();
    ui->tabProperties_txtKeyboardCommad->setText("/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard");
    qDebug()<<"/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard";
}

void kXneurApp::frmSettings::EditKeyboardCommand()
{
    //TODO Save in file setting
    ui->tabProperties_txtKeyboardCommad->setText(QFileDialog::getOpenFileName(0,tr("Select execute file"), "/usr/bin"));
}


