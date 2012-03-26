#include "frmsettings.h"
#include "ui_frmsettings.h"

//Qt header files
#include <QDebug>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QMessageBox>

kXneurApp::frmSettings::frmSettings(QWidget *parent, kXneurApp::xNeurConfig *cfg) :  QDialog(parent),  ui(new Ui::frmSettings)
{
  ui->setupUi(this);
  setAttribute( Qt::WA_DeleteOnClose, true);
  cfgNeur = cfg;
  cfgNeur->hot_get_list_command_hotkeys();
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

void kXneurApp::frmSettings::saveSettingsNeur()
{
    cfgNeur->clearNeurConfig();

    //tab General
    cfgNeur->gen_main_manual_switch(ui->chkGenMain_ManualSwitch->isChecked());
    cfgNeur->gen_main_auto_learning(ui->chkGenMain_AutoLearning->isChecked());
    cfgNeur->gen_main_keep_select(ui->chkGenMain_KeepSelect->isChecked());
    cfgNeur->gen_main_rotate_layout(ui->chkGenMain_RotateLayout->isChecked());
    cfgNeur->gen_main_check_lang(ui->chkGenMain_CheckLang->isChecked());
    cfgNeur->gen_tipo_correct_caps(ui->chkGenTipograph_CeorrectCAPS->isChecked());
    cfgNeur->gen_tipo_disable_caps(ui->chkGenTipograph_DisableCaps->isChecked());
    cfgNeur->gen_tipo_correct_two_caps(ui->chkGenTipograph_CorrectTwoCaps->isChecked());
    cfgNeur->gen_tipo_correct_space(ui->chkGenTipograph_CorrectSpace->isChecked());
    cfgNeur->gen_tipo_correct_small_letter(ui->chkGenTipograph_CorrectSmallLitter->isChecked());
    cfgNeur->gen_tipo_correct_two_space(ui->chkGenTipograph_CorrectTwoSpase->isChecked());
    cfgNeur->gen_tipo_correct_two_minus(ui->chkGenTipograph_CorrectTwoMinus->isChecked());
    cfgNeur->gen_tipo_correct_c(ui->chkGenTipograph_Correct_c_->isChecked());
    cfgNeur->gen_tipo_correct_tm(ui->chkGenTipograph_Correct_tm_->isChecked());
    cfgNeur->gen_tipo_correct_r(ui->chkGenTipograph_Correct_r_->isChecked());

    //tab Layout
    cfgNeur->lay_number_layout(ui->Layout_spbLayoutNumber->value()-1);
    cfgNeur->lay_remember_layout_for_app(ui->Layout_chkRememberKbLayout->isChecked());
    cfgNeur->lay_save_list_app_one_layout(getListFromWidget(ui->Layout_lstListApplicationOneKbLayout));



    //tab autocompletion
    cfgNeur->auto_enable_pattern(ui->tabAutocompletion_chkEnableAutocompl->isChecked());
    cfgNeur->auto_add_apace(ui->tabAutocompletion_chkAddSpace->isChecked());
    cfgNeur->auto_save_list_app_disable_autocomplite(getListFromWidget(ui->tabAutocompletion_lstApp));




    cfgNeur->saveNeurConfig();

}

void kXneurApp::frmSettings::settintgGrid()
{

    //general settings all tabwidgen on form
    QList<QTableWidget *> allTable = ui->tabWidget->findChildren<QTableWidget *>();
    for (int i=0; i< allTable.size();++i)
    {
        //allTable.at(i)->horizontalHeader()
        allTable.at(i)->verticalHeader()->setDefaultSectionSize(22);
        allTable.at(i)->verticalHeader()->hide();
        allTable.at(i)->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        allTable.at(i)->setSelectionMode(QAbstractItemView::SingleSelection);
        allTable.at(i)->setSelectionBehavior(QAbstractItemView::SelectRows);
        allTable.at(i)->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    //tab layout
    tab_lay_get_list_lang(cfgNeur->lay_get_list_language());
    tab_lay_get_list_app(cfgNeur->lay_get_list_app_one_layout());

    //tab HotKeys


    //tab autocomplection
    auto_get_list_app_autocomp(cfgNeur->auto_get_list_app_disable_autocomplite());

    //tab application
    ui->taApplication_lstAppNotUsed->addItems(cfgNeur->app_get_list_ignore_app());
    ui->taApplication_lstAppAutoMode->addItems(cfgNeur->app_get_list_auto_mode_app());
    ui->taApplication_lstAppManualMode->addItems(cfgNeur->app_get_list_manual_mode_app());


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
      saveSettingsNeur();
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

  //tab layout
  connect(ui->Layout_AddApp, SIGNAL(clicked()), SLOT(addApp_OneLayout()));
  connect(ui->Layout_DelApp,SIGNAL(clicked()),SLOT(removeApp_OneLayout()));
  connect(ui->Layout_cmdRulesChange, SIGNAL(clicked()),SLOT(rulesChange()));

  //tab abbreviations
  connect(ui->tabAbbreviations_cmdAdd, SIGNAL(clicked()), SLOT(addAbbreviation()));

  //tab properties
  connect(ui->tabProperties_cmdRecoverKeyCommand, SIGNAL(clicked()), SLOT(RecoverKeyboardCommand()));
  connect(ui->tabProperties_cmdEditKeyCommand, SIGNAL(clicked()),SLOT(EditKeyboardCommand()));
  connect(ui->tabProperties_cmbTypeIconTray, SIGNAL(activated(int)),SLOT(TypeIconTray(int)));
  connect(ui->tabProperties_cmbUsedRenderingEngine,SIGNAL(activated(int)),SLOT(TypeEngine(int)));
  connect(ui->tabProperties_cmdBrowseIconTray, SIGNAL(clicked()),SLOT(BrowseIconTray()));
  connect(ui->tabProperties_chkEnableAutostart, SIGNAL(clicked(bool)), SLOT(chekAutostart(bool)));
  connect(ui->tabProperties_spbDelayStartApp, SIGNAL(valueChanged(int)), SLOT(delayStartApp(int)));


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
        ui->tabAbbreviations_lstListAbbreviations->setRowCount(ui->tabAbbreviations_lstListAbbreviations->rowCount()+1);
        ui->tabAbbreviations_lstListAbbreviations->setItem(ui->tabAbbreviations_lstListAbbreviations->rowCount()-1, 0, new QTableWidgetItem(frmAbb->abb));
        ui->tabAbbreviations_lstListAbbreviations->setItem(ui->tabAbbreviations_lstListAbbreviations->rowCount()-1, 1, new QTableWidgetItem(frmAbb->text));
    }
}

void kXneurApp::frmSettings::tab_lay_get_list_lang(QStringList lstLng)
{
    ui->Layout_lstLayout->setRowCount(lstLng.size()/3);
    ui->Layout_lstLayout->setColumnCount(3);
    ui->Layout_lstLayout->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Description")));
    ui->Layout_lstLayout->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Layout")));
    ui->Layout_lstLayout->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Excluded")));
    //ui->Layout_lstLayout->
    int p=0;
    bool ok;
    for(int j=0;j<lstLng.size()/3;j++)
    {
        for (int i=0;i<3;i++)
        {
            lstLng.at(p).toInt(&ok);
            if(!ok)
            {
                ui->Layout_lstLayout->setItem(j,i, new QTableWidgetItem(lstLng.at(p)));
            }
            else
            {
                QTableWidgetItem *itm = new QTableWidgetItem();
                (lstLng.at(p).toInt()==0) ? itm->setCheckState(Qt::Unchecked) :itm->setCheckState(Qt::Checked) ;
                ui->Layout_lstLayout->setItem(j,i,itm);
            }
            p++;
        }
    }
}

void kXneurApp::frmSettings::tab_lay_get_list_app(QStringList lstApp)
{
    ui->Layout_lstListApplicationOneKbLayout->addItems(lstApp);
}

QStringList kXneurApp::frmSettings::getListFromWidget(QListWidget *wid)
{
    QStringList lstApp;
    QList<QListWidgetItem *> items =wid->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard);
    foreach(QListWidgetItem *item, items)
    {
      lstApp<<item->text();
    }
    return lstApp;
}

void kXneurApp::frmSettings::addApp_OneLayout()
{
    getNameApp *frm = new getNameApp();
    if(frm->exec() == QDialog::Accepted)
    {
        QString str = frm->appName;
        ui->Layout_lstListApplicationOneKbLayout->addItem(new QListWidgetItem(str));
    }
    delete frm;
}

void kXneurApp::frmSettings::removeApp_OneLayout()
{
    if (ui->Layout_lstListApplicationOneKbLayout->currentRow()<0)
    {
        QMessageBox::information(0,tr("Nothing deleted"), tr("You don't select an application that must be removed"), QMessageBox::Ok);
    }
    else
    {
        delete  ui->Layout_lstListApplicationOneKbLayout->takeItem( ui->Layout_lstListApplicationOneKbLayout->currentRow());
    }
}


//show  rule change layout
void kXneurApp::frmSettings::rulesChange()
{

}

void kXneurApp::frmSettings::auto_get_list_app_autocomp(QStringList lstApp)
{
    ui->tabAutocompletion_lstApp->addItems(lstApp);
}
