//app header files
#include "kxneur.h"

//KDE header files
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

//Qt header files
#include <QDebug>

static const KLocalizedString description =ki18n("kXneur (KDE X Neural Switcher) is xNeur front-end for KDE ( http://xneur.ru ).\nThis version work with XNeur v.0.15 only");
static const char version[] = "0.15.0";

int main(int argc, char *argv[])
{
  //qDebug()<< KGlobal::dirs()->localkdedir();
  kXneurApp::kXneur neur(argc, argv);
  neur.setApplicationName("kXneur");
  neur.setWindowIcon(QIcon(":/icons/kxneur.png"));
  kXneurApp::kXneur::setQuitOnLastWindowClosed(false);
  KAboutData about("kXneur",0, ki18n("kXneur Keyboard switcher") ,version,description,
                    KAboutData::License_GPL, ki18n("(C) 2012  Sergei Chystyakov"), ki18n(""), "http://xneur.ru","xneur@lists.net.ru");
  about.addAuthor(ki18n("Sergei Chystyakov"),ki18n(""), "klaider@yandex.ru", "http://xneur.ru");
  about.addCredit(ki18n("Andrew Crew Kuznetsov"), ki18n("Development X Neural Switcher"),"andrewcrew@rambler.ru","http://xneur.ru");
  about.addCredit(ki18n("Yankin Nickolay Valerevich"), ki18n("Site"),"web@softodrom.ru","http://xneur.ru");
  KCmdLineArgs::init(argc, argv, &about, KCmdLineArgs::CmdLineArgKDE);
  return neur.exec();

}
