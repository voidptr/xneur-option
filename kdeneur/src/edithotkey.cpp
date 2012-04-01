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
 /*   qDebug()<< event->nativeVirtualKey();
    qDebug()<< event->nativeScanCode();
    qDebug()<<event->nativeModifiers()*/;
    //qDebug()<< event->text();
}

void kXneurApp::EditHotKey::keyReleaseEvent(QKeyEvent *event)
{
    qDebug()<< QChar(event->key());
//    qDebug()<< event->text();

}
