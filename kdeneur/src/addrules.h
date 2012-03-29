#ifndef ADDRULES_H
#define ADDRULES_H

#include <QDialog>

namespace Ui
{
    class AddRules;
}

namespace kXneurApp
{
    class AddRules : public QDialog
    {
        Q_OBJECT
    public:
        explicit AddRules(QWidget *parent = 0);
        ~AddRules();
    private:
        Ui::AddRules *ui;
    private slots:
        void saveData();
        void closeForm();
    };
}
#endif // ADDRULES_H
