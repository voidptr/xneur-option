#ifndef FRMSETTINGS_H
#define FRMSETTINGS_H

//Qt header files
#include <QDialog>
#include <QAbstractButton>

namespace Ui
{
  class frmSettings;
}

namespace kXneurApp
{
  class frmSettings : public QDialog
  {
    Q_OBJECT

  public:
    explicit frmSettings(QWidget *parent = 0);
    ~frmSettings();

  private slots:
    void Clicked(QAbstractButton *);


  private:
    Ui::frmSettings *ui;
  };
}
#endif // FRMSETTINGS_H
