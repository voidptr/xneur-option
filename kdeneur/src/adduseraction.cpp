#include "adduseraction.h"
#include "ui_adduseraction.h"

kXneurApp::addUserAction::addUserAction(QString nm, QString key, QString cmd, QWidget *parent) :QDialog(parent),ui(new Ui::addUserAction)
{
    ui->setupUi(this);
    ui->txtActionName->setText(nm);
    ui->txtKeyBind->setText(key);
    ui->txtCommandAction->setText(cmd);
    connect(ui->cmdBox, SIGNAL(accepted()), SLOT(saveAction()));
    connect(ui->cmdBox,SIGNAL(rejected()),SLOT(cancelAction()));
}


kXneurApp::addUserAction::~addUserAction()
{
    delete ui;
}

void kXneurApp::addUserAction::saveAction()
{
    if(!ui->txtActionName->text().trimmed().isEmpty() && !ui->txtKeyBind->text().trimmed().isEmpty() && !ui->txtCommandAction->text().trimmed().isEmpty())
    {
        done(Accepted);
        name=ui->txtActionName->text();
        hot_key= ui->txtKeyBind->text();
        command = ui->txtCommandAction->text();
        this->close();
    }
}

void kXneurApp::addUserAction::cancelAction()
{
    done(Rejected);
    this->close();
}
