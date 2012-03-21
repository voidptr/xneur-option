#include "frmsettings.h"
#include "ui_frmsettings.h"

//Qt header files
#include <QDebug>
#include <QFileDialog>

kXneurApp::frmSettings::frmSettings(QWidget *parent) :  QDialog(parent),  ui(new Ui::frmSettings)
{
  ui->setupUi(this);
  config = new KConfig("kdeneurrc");
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
  settintgGrid();
  createConnect();
  readSettings();
}

kXneurApp::frmSettings::~frmSettings()
{
  delete ui;
}

void kXneurApp::frmSettings::settintgGrid()
{
    ui->tabAbbreviations_lstListAbbreviations->verticalHeader()->setDefaultSectionSize(22);
    ui->tabAbbreviations_lstListAbbreviations->verticalHeader()->hide();
    ui->tabAbbreviations_lstListAbbreviations->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

}

void kXneurApp::frmSettings::readSettings()
{
    //tab Properties
    ui->tabProperties_chkEnableAutostart->setChecked(properties.readEntry("Autostart",false));
    if(ui->tabProperties_chkEnableAutostart->isChecked())
    {
        ui->tabProperties_spbDelayStartApp->setValue(properties.readEntry("WaiTime",15));
        ui->tabProperties_spbDelayStartApp->setEnabled(true);

    }
    ui->tabProperties_cmbTypeIconTray->setCurrentIndex(properties.readEntry("Typeicontray",0));
    if(ui->tabProperties_cmbTypeIconTray->currentIndex()==0)
    {
        ui->tabProperties_grpFolderIcon->setEnabled(true);
        ui->tabProperties_txtPathIconTray->setText(properties.readEntry("Iconpath",""));
    }
    ui->tabProperties_cmbUsedRenderingEngine->setCurrentIndex(properties.readEntry("TypeEngine",0));
}

void kXneurApp::frmSettings::Clicked(QAbstractButton *button)
{
  if(ui->cmdBox->standardButton(button) == QDialogButtonBox::Ok)
  {
      done(QDialog::Accepted);
      qDebug()<<"SAVE SETTINGS";
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
  connect(ui->tabProperties_cmbUsedRenderingEngine,SIGNAL(activated(int)),SLOT(TypeEngine(int)));
  connect(ui->tabProperties_cmdBrowseIconTray, SIGNAL(clicked()),SLOT(BrowseIconTray()));
  connect(ui->tabProperties_chkEnableAutostart, SIGNAL(clicked(bool)), SLOT(chekAutostart(bool)));
  connect(ui->tabProperties_spbDelayStartApp, SIGNAL(valueChanged(int)), SLOT(delayStartApp(int)));


  //tab abbreviations
  connect(ui->tabAbbreviations_cmdAdd, SIGNAL(clicked()), SLOT(addAbbreviation()));

}

void kXneurApp::frmSettings::RecoverKeyboardCommand()
{
    ui->tabProperties_txtKeyboardCommad->clear();
    ui->tabProperties_txtKeyboardCommad->setText("/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard");
    properties.writeEntry("KeyboardCommand", "/usr/bin/kcmshell4 --args=--tab=layouts kcm_keyboard");
}

void kXneurApp::frmSettings::EditKeyboardCommand()
{
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
    KStandardDirs dir;
    QString kdeneur = "kdeneur.desktop";
    QString str="[Desktop Entry]\nEncoding=UTF-8\nExec=kdeneur\nIcon=kdeneur\nTerminal=false\nType=Application\nCategories=Qt;KDE;Utility;\nStartupNotify=true\nName=kdeNeur\nName[ru]=kdeNeur\nComment=Automatic keyboard layout switcher\nComment[ru]=Автоматический переключатель раскладки клавиатуры";
    QString pathAutostart = dir.findResourceDir("xdgconf-autostart", "");
    QString fileDesktop = QString("%1%2").arg(pathAutostart).arg(kdeneur);
    if(cheked && KStandardDirs::exists(pathAutostart))
    {
        QFile file(fileDesktop);
        if(file.open(QIODevice::WriteOnly))
        {
           QTextStream out(&file);
           out.setCodec("UTF-8");
           out <<str;
           file.close();
        }
       properties.writeEntry("WaiTime",  ui->tabProperties_spbDelayStartApp->value());
    }
    else if(QFile::exists(fileDesktop))
    {
        if(!QFile::remove(fileDesktop))
        {
            qDebug()<<"ERROR: Don't del file";
        }
        properties.deleteEntry("WaiTime");
    }
    properties.writeEntry("Autostart",  cheked);
    ui->tabProperties_chkEnableAutostart->setEnabled(cheked);
    ui->tabProperties_spbDelayStartApp->setEnabled(cheked);
}

void kXneurApp::frmSettings::delayStartApp(int val)
{
    properties.writeEntry("WaiTime",  val);
}

void kXneurApp::frmSettings::TypeEngine(int selectIndex)
{
    properties.writeEntry("TypeEngine", selectIndex);
}

void kXneurApp::frmSettings::addAbbreviation()
{
    frmAddAbbreviature *frmAbb = new frmAddAbbreviature();

    if(frmAbb->exec() == QDialog::Accepted)
    {
        //QTableWidgetItem *item=new QTableWidgetItem ();
        ui->tabAbbreviations_lstListAbbreviations->setRowCount(ui->tabAbbreviations_lstListAbbreviations->rowCount()+1);
        ui->tabAbbreviations_lstListAbbreviations->setItem(ui->tabAbbreviations_lstListAbbreviations->rowCount()-1, 0, new QTableWidgetItem(frmAbb->abb));
        ui->tabAbbreviations_lstListAbbreviations->setItem(ui->tabAbbreviations_lstListAbbreviations->rowCount()-1, 1, new QTableWidgetItem(frmAbb->text));
    }
}
