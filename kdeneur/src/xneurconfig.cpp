extern "C"
{
    #include <xneur/xneur.h>
    #include <xneur/xnconfig.h>
    #include <xneur/list_char.h>
}



#include "xneurconfig.h"

#define MAX_LANGUAGES 4
#define TOTAL_MODIFER 4
#define XNEUR_NEEDED_MAJOR_VERSION 15
#define XNEUR_BUILD_MINOR_VERSION 0

extern "C"
{
    #include "xkb.h"
}

Display *kXneurApp::xNeurConfig::dpy=NULL;

kXneurApp::xNeurConfig::xNeurConfig(QObject *parent) :  QObject(parent)
{
    xconfig = NULL;
    init_libxnconfig();
    xneur_pid = xconfig->get_pid(xconfig);
    dpy=XOpenDisplay(NULL);
    procxNeur = new QProcess();
    connect(procxNeur, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(procxNeurStop(int,QProcess::ExitStatus)));
    connect(procxNeur, SIGNAL(started()), SLOT(procxNeurStart()));

}

kXneurApp::xNeurConfig::~xNeurConfig()
{
    XCloseDisplay(dpy);
}
QString kXneurApp::xNeurConfig::get_bind(int ind)
{
    QString key;
    QStringList lstModifer;
    lstModifer << "Shift" << "Control" << "Alt" << "Super";
    for (int i = 0; i < TOTAL_MODIFER; ++i)
    {
        if ((xconfig->hotkeys[ind].modifiers & (0x1 << i)) == 0)
            continue;

        key = QString("%1+").arg(lstModifer.at(i));
    }
  key+=QString("%1+").arg( xconfig->hotkeys[ind].key);
  return key;
}

bool kXneurApp::xNeurConfig::init_libxnconfig()
{
    if (xconfig != NULL)
    {
        return true;
    }

    if((xconfig = xneur_config_init())==NULL)
    {
        qDebug()<<"ERROR: lib init fail";
        return false;
    }
    int major_version, minor_version;
    xconfig->get_library_version(&major_version, &minor_version);
    if ((major_version != XNEUR_NEEDED_MAJOR_VERSION) || (minor_version != XNEUR_BUILD_MINOR_VERSION))
    {
        qDebug()<<QString(tr("Wrong XNeur configuration library api version.\nPlease, install libxnconfig version 0.%1.%2")).arg(XNEUR_NEEDED_MAJOR_VERSION).arg(XNEUR_BUILD_MINOR_VERSION);
        xconfig->uninit(xconfig);
        exit(1);
    }
    qDebug()<<QString(tr("Using libxnconfig API version 0.%1.%2 (build with 0.%3.%4)")).arg(major_version).arg(minor_version).arg(XNEUR_NEEDED_MAJOR_VERSION).arg(XNEUR_BUILD_MINOR_VERSION);
    if (!xconfig->load(xconfig))
    {
        qDebug()<< tr("XNeur's config broken or was created with old version!\nPlease, remove ~/.xneur/. It should solve the problem!\nIf you don't want to loose your configuration, back it up\nand manually patch new configuration which will be created after first run.");
        xconfig->uninit(xconfig);
        exit(1);
    }
  return true;
}

bool kXneurApp::xNeurConfig::xneurStop()
{

    if ( xneur_pid > 0 )
    {
        if ( xconfig->kill(xconfig) )
        {
            xconfig->set_pid(xconfig, 0);
        }
        else
        {
           qDebug()<< "ERROR: Fail xNeur stopped [don't kill xconfig']";
           return false;
        }
        xneur_pid = 0;
        //TODO
//        if(running)
//        {
//           procxNeurStop(0,QProcess::NormalExit);
//        }
        return true;
    }
    qDebug()<< "MSG: xNeur isn't running";
    return false;

    //

}

bool kXneurApp::xNeurConfig::xneurStart()
{
    if ( !init_libxnconfig())
    {
        qDebug("ERROR: start xNeur Fail [not init libxnconfig]");
        return false;
    }
    xneur_pid = xconfig->get_pid(xconfig);
    if ( xneur_pid > 0 )
    {
        if (!xneurStop())
        {
            return false;
        }
    }
    procxNeur->start("xneur",QIODevice::ReadWrite);
    xneur_pid = procxNeur->pid();
    if ( xneur_pid > 0 )
    {
        init_libxnconfig();
        return true;
    }
    qDebug() << tr("ERROR: start xNeur Fail") /*<< error*/;
    xneur_pid = 0;
    qDebug("start -- fail 3\n");
    return false;
}

int kXneurApp::xNeurConfig::getNeur_pid()
{
    return xconfig->get_pid(xconfig);
}

QString kXneurApp::xNeurConfig::getCurrentLang()
{
    return QString("%1").arg(xconfig->handle->languages[get_active_kbd_group(dpy)].dir);
}

void kXneurApp::xNeurConfig::setNextLang()
{
    set_next_kbd_group(dpy);
}

void kXneurApp::xNeurConfig::restartNeur()
{
   xconfig->reload(xconfig);
}

void kXneurApp::xNeurConfig::procxNeurStop(int exitcode, QProcess::ExitStatus exitstatus)
{
  emit setStatusXneur(false);
  qDebug()<<"MSG: xNeur stopped:" << " ExitCode " << exitcode << " ExitStatus " << exitstatus;
  if(exitstatus >0)
  {
      qDebug()<< tr("ERROR: Warning process xNeur crashed, please look log file and inform the author xNeur. Thank You!");
  }
}

void kXneurApp::xNeurConfig::procxNeurStart()
{
  qDebug()<<"MSG: xNeur started.";
  emit setStatusXneur(true);
}

void kXneurApp::xNeurConfig::clearNeurConfig()
{
    xconfig->clear(xconfig);
}

void kXneurApp::xNeurConfig::saveNeurConfig()
{
      xconfig->save(xconfig);
}

void kXneurApp::xNeurConfig::test(QString str)
{
    qDebug()<<str;
}

//void kXneurApp::xNeurConfig::delayStartApp(int time)
//{
//    xconfig->
//}

/*================================= tab General =================================*/
void kXneurApp::xNeurConfig::gen_main_manual_switch(bool stat)
{
  xconfig->manual_mode = stat;
}
void kXneurApp::xNeurConfig::gen_main_auto_learning(bool stat)
{
    xconfig->educate = stat;
}
void kXneurApp::xNeurConfig::gen_main_keep_select(bool stat)
{
    xconfig->save_selection_after_convert=stat;
}
void kXneurApp::xNeurConfig::gen_main_rotate_layout(bool stat)
{
    xconfig->rotate_layout_after_convert=stat;
}
void kXneurApp::xNeurConfig::gen_main_check_lang(bool stat)
{
    xconfig->check_lang_on_process=stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_caps(bool stat)
{
    xconfig->correct_incidental_caps = stat;
}
void kXneurApp::xNeurConfig::gen_tipo_disable_caps(bool stat)
{
    xconfig->disable_capslock=stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_two_caps(bool stat)
{
    xconfig->correct_two_capital_letter =stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_space(bool stat)
{
    xconfig->correct_space_with_punctuation=stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_small_letter(bool stat)
{
    xconfig->correct_capital_letter_after_dot = stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_two_space(bool stat)
{
    xconfig->correct_two_space_with_comma_and_space =stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_two_minus(bool stat)
{
    xconfig->correct_two_minus_with_dash = stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_c(bool stat)
{
    xconfig->correct_c_with_copyright =stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_tm(bool stat)
{
    xconfig->correct_tm_with_trademark = stat;
}
void kXneurApp::xNeurConfig::gen_tipo_correct_r(bool stat)
{
    xconfig->correct_r_with_registered = stat;
}

/*================================= tab Layout =================================*/

void kXneurApp::xNeurConfig::lay_number_layout(int curIndex)
{
    xconfig->default_group = curIndex;
}
void kXneurApp::xNeurConfig::lay_remember_layout_for_app(bool stat)
{
    xconfig->remember_layout = stat;
}

void kXneurApp::xNeurConfig::lay_save_list_language()
{

}
QStringList kXneurApp::xNeurConfig::lay_get_list_language()
{
    QStringList lstLng;
    for(int lng=0; lng< xconfig->handle->total_languages && lng < MAX_LANGUAGES; lng++)
    {
        lstLng<< xconfig->handle->languages[lng].name;
        lstLng<<xconfig->handle->languages[lng].dir;
        lstLng<<QString("%1").arg(xconfig->handle->languages[lng].excluded);
    }
    return lstLng;
}

QStringList kXneurApp::xNeurConfig::lay_get_list_app_one_layout()
{
    QStringList lstApp;

    for(int i=0; i<xconfig->layout_remember_apps->data_count;i++)
    {
        lstApp << xconfig->layout_remember_apps->data[0].string;
    }
    return lstApp;
}
void kXneurApp::xNeurConfig::lay_save_list_app_one_layout(QStringList lstApp)
{
    //TODO save Add app for one layout
    qDebug()<<lstApp.size();
}

/*================================= tab HotKeys =================================*/
void kXneurApp::xNeurConfig::hot_get_list_command_hotkeys()
{
    QStringList lstCommand;
//    QStringList lstModifer;
//    lstModifer << "Shift" << "Control" << "Alt" << "Super";
    lstCommand << tr("Correct/Undo correction") << tr("Transliterate") << tr("Change case") << tr("Preview correction") << tr("Correct last line")
               << tr("Correct selected text") << tr("Transliterate selected text") << tr("Change case of selected text") << tr("Preview correction of selected text")
               << tr("Correct clipboard text") << tr("Transliterate clipboard text") << tr("Change case of clipboard text") << tr("Preview correction of clipboard text")
               << tr("Switch to layout 1") << tr("Switch to layout 2") << tr("Switch to layout 3") << tr("Switch to layout 4")
               << tr("Rotate layouts") << tr("Rotate layouts back") << tr("Expand abbreviations") << tr("Autocompletion confirmation")
               << tr("Block/Unblock keyboard and mouse events") << tr("Insert date");
    QString hot_key;
    qDebug() << "MAX_HOTKEY " << MAX_HOTKEYS;
    qDebug() << "Size lstCommand " << lstCommand.size();

//    for(int i=0;i<MAX_HOTKEYS; ++i)
//    {
//        if(xconfig->hotkeys[i].key!=NULL)
//        {
//            hot_key = get_bind(i);
//            qDebug()<< "LST COMMAND " << lstCommand.at(i) << " HOT KEY " << hot_key;
//        }
//    }

//    char *binds = "";
//            if (xconfig->hotkeys[i].key != NULL)
//                binds = concat_bind(i);

//    //        if ((xconfig->hotkeys[action].modifiers & (0x1 << i)) == 0)
//    //            continue;


//            //qDebug()<<  ;
//    for(int p=0; p< lstModifer.size();++p)
//    {
//        if()
//        {
//        }
//    }
//    for (int i = 0; i < total_modifiers; i++)
//        {


//            strcat(text, modifier_names[i]);
//            strcat(text, "+");
//        }
}



/*================================= tab Autocompletion =================================*/

void kXneurApp::xNeurConfig::auto_enable_pattern(bool stat)
{
    xconfig->autocompletion =stat;
}

void kXneurApp::xNeurConfig::auto_add_apace(bool stat)
{
    xconfig->add_space_after_autocompletion =stat;
}

QStringList kXneurApp::xNeurConfig::auto_get_list_app_disable_autocomplite()
{
    QStringList lstApp;
    for(int i=0; i<xconfig->autocompletion_excluded_apps->data_count; ++i)
    {
        lstApp<< xconfig->autocompletion_excluded_apps->data[i].string;
    }
    return lstApp;
}

void kXneurApp::xNeurConfig::auto_save_list_app_disable_autocomplite(QStringList lstApp)
{
    //TODO
    qDebug()<<lstApp.size();
}


/*================================= tab Aplication =================================*/

QStringList kXneurApp::xNeurConfig::app_get_list_ignore_app()
{
    QStringList lstApp;

    for (int i = 0; i < xconfig->excluded_apps->data_count; i++)
    {
        lstApp<< xconfig->excluded_apps->data[i].string;
    }
    return lstApp;
}

QStringList kXneurApp::xNeurConfig::app_get_list_auto_mode_app()
{
    QStringList lstApp;

    for (int i = 0; i < xconfig->auto_apps->data_count; i++)
    {
        lstApp<< xconfig->auto_apps->data[i].string;
    }
    return lstApp;
}

QStringList kXneurApp::xNeurConfig::app_get_list_manual_mode_app()
{
    QStringList lstApp;
    for (int i = 0; i < xconfig->manual_apps->data_count; i++)
    {
        lstApp<<  xconfig->manual_apps->data[i].string;
    }

    return lstApp;
}

void kXneurApp::xNeurConfig::app_save_list_ignore_app()
{

}

void kXneurApp::xNeurConfig::app_save_list_auto_mode_app()
{

}

void kXneurApp::xNeurConfig::app_save_list_manual_mode_app()
{

}

