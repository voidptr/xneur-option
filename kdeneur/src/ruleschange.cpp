#include "ruleschange.h"
#include "ui_ruleschange.h"
#include "addrules.h"

kXneurApp::RulesChange::RulesChange(QWidget *parent) :   QDialog(parent),   ui(new Ui::RulesChange)
{
    ui->setupUi(this);
    createConnect();
    ui->listWords->verticalHeader()->setDefaultSectionSize(22);
    ui->listWords->verticalHeader()->hide();
    ui->listWords->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->listWords->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listWords->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWords->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

kXneurApp::RulesChange::~RulesChange()
{
    delete ui;
}

void kXneurApp::RulesChange::createConnect()
{
    connect(ui->cmdAdd, SIGNAL(clicked()),SLOT(addWords()));
    connect(ui->cmdEdit, SIGNAL(clicked()),SLOT(editWors()));
    connect(ui->cmdRemove, SIGNAL(clicked()), SLOT(delWords()));
    connect(ui->cmdClose, SIGNAL(clicked()), SLOT(closeForm()));
}

void kXneurApp::RulesChange::addWords()
{
    kXneurApp::AddRules *frm = new kXneurApp::AddRules();
    if(frm->exec()==QDialog::Accepted)
    {

    }
    delete frm;
}

void kXneurApp::RulesChange::editWors()
{

}

void kXneurApp::RulesChange::delWords()
{
    ui->listWords->removeRow(ui->listWords->currentRow());
}

void kXneurApp::RulesChange::closeForm()
{
    //TODO
    this->close();
}
