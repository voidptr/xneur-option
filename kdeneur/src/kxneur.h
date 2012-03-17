#ifndef KXNEUR_H
#define KXNEUR_H

//app header files
#include "kxneurtray.h"

//Qt header files
#include <QApplication>
#include <QProcess>

namespace kXneurApp
{
    class kXneur : public QApplication
    {
        Q_OBJECT

    public:
        kXneur(int&, char **);
        ~kXneur();
         static Display *dpy;
    public slots:

    private:
      bool running;
      kXneurTray *trayApp;
      QProcess *procxNeur;
      int xneur_pid;
      struct _xneur_config *xconfig;
      void settignsTray();
      bool xneurStop();
      bool xneurStart();
      bool init_libxnconfig();
    signals:
      void changeIconTray(QString);
    private slots:
      void layoutChanged(QString);
      void startStopNeur();
      void nextLang();
      void restartNeur();
      void procxNeurStop(int,QProcess::ExitStatus);
      void procxNeurStart();

    };
}
#endif // KXNEUR_H
