#include "frmsettings.h"
#include "ui_frmsettings.h"

kXneurApp::frmSettings::frmSettings(QWidget *parent) :  QDialog(parent),  ui(new Ui::frmSettings)
{
  ui->setupUi(this);
  connect(ui->cmdBox, SIGNAL(clicked(QAbstractButton*)), SLOT(Clicked(QAbstractButton*)));
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
