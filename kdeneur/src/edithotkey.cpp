#include "edithotkey.h"
#include "ui_edithotkey.h"

kXneurApp::EditHotKey::EditHotKey(QWidget *parent, QString action, QString key) : QDialog(parent), ui(new Ui::EditHotKey)
{
    ui->setupUi(this);
    ui->txtAction->setText(action);
    ui->txtKey->setText(key);
}

kXneurApp::EditHotKey::~EditHotKey()
{
    delete ui;
}

void kXneurApp::EditHotKey::keyPressEvent(QKeyEvent *event)
{
    qDebug()<< event->key();
}

void kXneurApp::EditHotKey::keyReleaseEvent(QKeyEvent *event)
{
    //qDebug()<< event->key();
}
