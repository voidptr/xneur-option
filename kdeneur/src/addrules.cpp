#include "addrules.h"
#include "ui_addrules.h"


kXneurApp::AddRules::AddRules(QWidget *parent) :  QDialog(parent), ui(new Ui::AddRules)
{
    ui->setupUi(this);
    connect(ui->cmdBox, SIGNAL(accepted()),SLOT(saveData()));
    connect(ui->cmdBox, SIGNAL(rejected()),SLOT(closeForm()));
}

kXneurApp::AddRules::~AddRules()
{
    delete ui;
}

void kXneurApp::AddRules::saveData()
{
    done(Accepted);
    close();
}

void kXneurApp::AddRules::closeForm()
{
    done(Rejected);
    close();
}
