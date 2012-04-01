#ifndef ADDUSERACTION_H
#define ADDUSERACTION_H

#include <QDialog>

namespace Ui
{
    class addUserAction;
}

namespace kXneurApp
{
    class addUserAction : public QDialog
    {
        Q_OBJECT

    public:
        explicit addUserAction(QString nm="", QString key="", QString cmd="", QWidget *parent = 0);
        ~addUserAction();
        QString name, hot_key, command;
    private slots:
        void saveAction();
        void cancelAction();
    private:
        Ui::addUserAction *ui;
    };
}

#endif // ADDUSERACTION_H
