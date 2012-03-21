#ifndef XNEURCONFIG_H
#define XNEURCONFIG_H

#include <QString>
#include <QProcess>
#include <QObject>
#include <QDebug>
#include <qwindowdefs.h>

namespace kXneurApp
{
    class xNeurConfig: public QObject
    {
        Q_OBJECT
    private:
        int xneur_pid;

    private slots:
        void procxNeurStart();
        void procxNeurStop(int,QProcess::ExitStatus);

    public:
       explicit xNeurConfig(QObject *parent = 0);
        ~xNeurConfig();
        static Display *dpy;
        QProcess *procxNeur;
        struct _xneur_config *xconfig;
        bool xneurStop();
        bool xneurStart();
        bool init_libxnconfig();
        QString getCurrentLang();
        int getNeur_pid();

    public slots:
        void setNextLang();
        void restartNeur();

    signals:
        void setStatusXneur(bool);
    };
}
#endif // XNEURCONFIG_H
