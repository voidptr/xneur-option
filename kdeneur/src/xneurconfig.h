#ifndef XNEURCONFIG_H
#define XNEURCONFIG_H

#include <QString>
#include <QProcess>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <qwindowdefs.h>

namespace kXneurApp
{
    class xNeurConfig: public QObject
    {
        Q_OBJECT
    private:
        int xneur_pid;
        QString get_bind(int);
        QStringList notifyNames;

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
        void clearNeurConfig();
        void saveNeurConfig();

        void test(QString);


        //tab General
        void gen_main_manual_switch(bool);
        void gen_main_auto_learning(bool);
        void gen_main_keep_select(bool);
        void gen_main_rotate_layout(bool);
        void gen_main_check_lang(bool);
        void gen_tipo_correct_caps(bool);
        void gen_tipo_disable_caps(bool);
        void gen_tipo_correct_two_caps(bool);
        void gen_tipo_correct_space(bool);
        void gen_tipo_correct_small_letter(bool);
        void gen_tipo_correct_two_space(bool);
        void gen_tipo_correct_two_minus(bool);
        void gen_tipo_correct_c(bool);
        void gen_tipo_correct_tm(bool);
        void gen_tipo_correct_r(bool);

        //tab Layout
        void lay_number_layout(int);
        void lay_remember_layout_for_app(bool);
        void lay_save_list_language();
        QStringList lay_get_list_language();
        QStringList lay_get_list_app_one_layout();
        void lay_save_list_app_one_layout(QStringList);

        //tab hotkeys
        QMap <QString, QString> hot_get_list_command_hotkeys();
        QMap<QString, QMap<QString, QString> >  hot_get_list_user_actions();
        void hot_save_list_command_hotkeys();
        void hot_save_list_user_actions();

        //tab autocompletion
        void auto_enable_pattern(bool);
        void auto_add_apace(bool);
        QStringList auto_get_list_app_disable_autocomplite();
        void auto_save_list_app_disable_autocomplite(QStringList);

        //tab applications
        QStringList app_get_list_ignore_app();
        QStringList app_get_list_auto_mode_app();
        QStringList app_get_list_manual_mode_app();
        void app_save_list_ignore_app();
        void app_save_list_auto_mode_app();
        void app_save_list_manual_mode_app();

        //tab Notifications
            //tab SOUND
        void notif_enable_sound(bool);
        void notif_volume_sound(int);
        QMap<QString, QMultiMap<QString, QString> > notif_get_action_sound();
        void notif_save_action_sound();
            //tab OSD
        void notif_enable_show_osd(bool);
        void notif_set_font_osd(QString);
        QMap<QString, QMultiMap<QString, QString> >  notif_get_action_osd();
        void notif_save_action_osd();
            //tab POPUP MSG
        void notif_enable_show_popup_msg(bool);
        void notif_interval_popup_msg(int);
        QMap<QString, QMultiMap<QString, QString> >  notif_get_action_popup_msg();
        void notif_save_action_popup_msg();

        //tab Abbreviations
        void abbr_ignore_keyboarf_layout(bool);
        QMap <QString, QString> abbr_get_list_abbreviations();
        void abbr_save_list_abbreviations();


        //tab Log

        //tab Trobleshooting

        //tab Advanced

        //tab Plugins
        QMap<QString, QMultiMap<bool, QString> >  plug_get_list_plugins();
        void plug_save_list_plugins();

        //tab Properties

        //xconfig->play_sounds
        //void delayStartApp(int);

    public slots:
        void setNextLang();
        void restartNeur();

    signals:
        void setStatusXneur(bool);
    };
}
#endif // XNEURCONFIG_H
