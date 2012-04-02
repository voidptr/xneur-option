#include "edithotkey.h"
#include "ui_edithotkey.h"
#include "xneurconfig.h"

extern "C"
{
#include <X11/XKBlib.h>
}


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

    QString key = QString("%1").arg(XKeysymToString(XkbKeycodeToKeysym(kXneurApp::xNeurConfig::dpy, event->nativeScanCode(),0,0)));
    QString md_key = modif(event->modifiers());
    ui->txtKey->setText(QString("%1%2").arg(md_key).arg(key));
    qDebug()<< "USER PRESS " << QString("%1%2").arg(md_key).arg(key);
    key.clear();
    md_key.clear();
}

void kXneurApp::EditHotKey::keyReleaseEvent(QKeyEvent *event)
{
//    hot_keys += QString("%1").arg(XKeysymToString(XkbKeycodeToKeysym(kXneurApp::xNeurConfig::dpy, event->nativeScanCode(),0,0)));
//    ui->txtKey->clear();
//    ui->txtKey->setText(hot_keys);
  //  qDebug() << XKeysymToString(XkbKeycodeToKeysym(kXneurApp::xNeurConfig::dpy, event->nativeScanCode(),0,0));
    //qDebug()<< QChar(event->key());
//    qDebug()<< event->tex
}

QString kXneurApp::EditHotKey::modif(int value)
{
    QString key="";
    switch(value)
    {
    case Qt::ShiftModifier:
        key = QString("%1 + ").arg("Shift");
       break;
    case Qt::ControlModifier:
        key = QString("%1 + ").arg("Control");
       break;
    case Qt::AltModifier:
        key = QString("%1 + ").arg("Alt");
       break;
    case Qt::MetaModifier:
        key = QString("%1 + ").arg("Super");
       break;
    }
    return key;
}
