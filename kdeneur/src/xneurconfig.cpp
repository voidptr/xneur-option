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
    notifyNames << tr("Xneur started") << tr("Xneur reloaded") << tr("Xneur stopped") << tr("Keypress on layout 1")
                << tr( "Keypress on layout 2") << tr("Keypress on layout 3") << tr("Keypress on layout 4") << tr("Switch to layout 1")
                << tr( "Switch to layout 2") << tr("Switch to layout 3") << tr("Switch to layout 4") << tr("Correct word automatically")
                << tr( "Correct last word manually") << tr("Transliterate last word manually") << tr("Change case of last word manually")
                << tr( "Preview correction of last word manually") << tr("Correct last line") << tr("Correct selected text") << tr("Transliterate selected text")
                << tr( "Change case of selected text") << tr("Preview correction of selected text") << tr("Correct clipboard text") << tr("Transliterate clipboard text")
                << tr( "Change case of clipboard text") << tr("Preview correction of clipboard text") << tr("Expand abbreviations") << tr("Correct aCCIDENTAL caps")
                << tr( "Correct TWo INitial caps") << tr("Correct two space with a comma and a space") << tr("Correct two minus with a dash")
                << tr( "Correct (c) with a copyright sign") << tr("Correct (tm) with a trademark sign") << tr("Correct (r) with a registered sign")
                << tr( "Correct three points with a ellipsis sign") << tr("Execute user action") << tr("Block keyboard and mouse events")
                << tr( "Unblock keyboard and mouse events");
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
  key+=QString("%1").arg( xconfig->hotkeys[ind].key);
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
QMap <QString, QString> kXneurApp::xNeurConfig::hot_get_list_command_hotkeys()
{
    QStringList lstCommand;
    QString hot_key;
    QMap <QString, QString> tblHotKey;
    lstCommand << tr("Correct/Undo correction") << tr("Transliterate") << tr("Change case") << tr("Preview correction") << tr("Correct last line")
               << tr("Correct selected text") << tr("Transliterate selected text") << tr("Change case of selected text") << tr("Preview correction of selected text")
               << tr("Correct clipboard text") << tr("Transliterate clipboard text") << tr("Change case of clipboard text") << tr("Preview correction of clipboard text")
               << tr("Switch to layout 1") << tr("Switch to layout 2") << tr("Switch to layout 3") << tr("Switch to layout 4")
               << tr("Rotate layouts") << tr("Rotate layouts back") << tr("Expand abbreviations") << tr("Autocompletion confirmation")
               << tr("Block/Unblock keyboard and mouse events") << tr("Insert date");

    for(int i=0;i<MAX_HOTKEYS; ++i)
    {
        if(xconfig->hotkeys[i].key!=NULL)
        {
            hot_key = get_bind(i);
            tblHotKey.insert(lstCommand.at(i), hot_key);
        }
        else
        {
            hot_key="";
            tblHotKey.insert(lstCommand.at(i), hot_key);
        }
    }
return tblHotKey;
}

void kXneurApp::xNeurConfig::hot_save_list_command_hotkeys()
{
    //TODO
}

QMap<QString, QMap<QString, QString> >  kXneurApp::xNeurConfig::hot_get_list_user_actions()
{
    QString text;
    QStringList lstModifer;
    //   hot_key       name act  command act
    QMap<QString, QMap<QString, QString> > lstUserAction;
    QMap<QString, QString> lstNameCmd;
    lstModifer << "Shift" << "Control" << "Alt" << "Super";
    for (int action = 0; action < xconfig->actions_count; action++)
    {
        for (int i = 0; i < TOTAL_MODIFER; ++i)
        {
                if ((xconfig->actions[action].hotkey.modifiers & (0x1 << i)) == 0)
                {
                    continue;
                }

               text = QString("%1+").arg(lstModifer.at(i));
        }
        text += QString("%1").arg(xconfig->actions[action].hotkey.key);



        lstNameCmd.insert(xconfig->actions[action].name,xconfig->actions[action].command);
        lstUserAction.insert(text, lstNameCmd);
        lstNameCmd.clear();
        text="";
    }
    return lstUserAction;
}

void kXneurApp::xNeurConfig::hot_save_list_user_actions()
{
    //TODO
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


/*================================= tab Application =================================*/

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
//TODO
}

void kXneurApp::xNeurConfig::app_save_list_auto_mode_app()
{
//TODO
}

void kXneurApp::xNeurConfig::app_save_list_manual_mode_app()
{
    //TODO

}

/*================================= tab Notifications =================================*/
                            /*========== tab SOUND ==========*/
void kXneurApp::xNeurConfig::notif_enable_sound(bool stat)
{
    xconfig->play_sounds = stat;
}

void kXneurApp::xNeurConfig::notif_volume_sound(int volume)
{
    xconfig->volume_percent = volume;
}

QMap<QString, QMultiMap<QString, QString> > kXneurApp::xNeurConfig::notif_get_action_sound()
{
    QMap<QString, QMultiMap<QString, QString> > lstSound;
    QMultiMap <QString, QString> lstFile;
    for (int i = 0; i <notifyNames.size(); ++i)
    {
        lstFile.insert(QString("%1").arg(xconfig->sounds[i].enabled), QString("%1").arg(xconfig->sounds[i].file));
        lstSound.insert(notifyNames.at(i), lstFile);
        lstFile.clear();
    }
    return lstSound;
}

void kXneurApp::xNeurConfig::notif_save_action_sound(){}
                            /*========== tab OSD ==========*/
void kXneurApp::xNeurConfig::notif_enable_show_osd(bool stat)
{
    xconfig->show_osd = stat;
}
void kXneurApp::xNeurConfig::notif_set_font_osd(QString osd_font)
{
    xconfig->osd_font = osd_font.toAscii().data();
}
QMap<QString, QMultiMap<QString, QString> >  kXneurApp::xNeurConfig::notif_get_action_osd()
{
    QMap<QString, QMultiMap<QString, QString> > lstOSD;
    QMultiMap <QString, QString> lstFile;
    for (int i = 0; i <notifyNames.size(); ++i)
    {
        lstFile.insert(QString("%1").arg(xconfig->osds[i].enabled), QString("%1").arg(xconfig->osds[i].file));
        lstOSD.insert(notifyNames.at(i), lstFile);
        lstFile.clear();
    }
    return lstOSD;
}
void kXneurApp::xNeurConfig::notif_save_action_osd(){}
                            /*========== tab POPUP MSG ==========*/
void kXneurApp::xNeurConfig::notif_enable_show_popup_msg(bool stat)
{
    xconfig->show_popup = stat;
}
void kXneurApp::xNeurConfig::notif_interval_popup_msg(int interval)
{
    xconfig->popup_expire_timeout = interval;
}
QMap<QString, QMultiMap<QString, QString> >  kXneurApp::xNeurConfig::notif_get_action_popup_msg()
{
    QMap<QString, QMultiMap<QString, QString> > lstPOPUP;
    QMultiMap <QString, QString> lstFile;
    for (int i = 0; i <notifyNames.size(); ++i)
    {
        lstFile.insert(QString("%1").arg(xconfig->popups[i].enabled), QString("%1").arg(xconfig->popups[i].file));
        lstPOPUP.insert(notifyNames.at(i), lstFile);
        lstFile.clear();
    }
    return lstPOPUP;
}
void kXneurApp::xNeurConfig::notif_save_action_popup_msg()
{

}

/*================================= tab Abbreviations =================================*/
void kXneurApp::xNeurConfig::abbr_ignore_keyboarf_layout(bool stat)
{
    xconfig->abbr_ignore_layout = stat;
}

QMap <QString, QString> kXneurApp::xNeurConfig::abbr_get_list_abbreviations()
{
    QString tmpStr;
    QMap <QString, QString> lstAbb;
    for (int i = 0; i < xconfig->abbreviations->data_count; ++i)
    {
        tmpStr = xconfig->abbreviations->data[i].string;
        lstAbb.insert(tmpStr.left(tmpStr.indexOf(" ")), tmpStr.right(tmpStr.length() -tmpStr.indexOf(" ")));
    }
return lstAbb;
}

void kXneurApp::xNeurConfig::abbr_save_list_abbreviations()
{

}

