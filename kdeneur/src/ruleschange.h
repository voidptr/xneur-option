#ifndef RULESCHANGE_H
#define RULESCHANGE_H

#include <QDialog>
//#include <QDialogButtonBox>

namespace Ui
{
    class RulesChange;
}
namespace kXneurApp
{
    class RulesChange : public QDialog
    {
        Q_OBJECT

    public:
        explicit RulesChange(QWidget *parent = 0);
        ~RulesChange();

    private:
        Ui::RulesChange *ui;
        void createConnect();
    private slots:
        void closeForm();
        void addWords();
        void editWors();
        void delWords();

    };
}

#endif // RULESCHANGE_H
