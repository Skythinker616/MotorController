#ifndef TIMCONFDIALOG_H
#define TIMCONFDIALOG_H

#include <QDialog>

namespace Ui {
class TimConfDialog;
}

class TimConf{
public:
    uint16_t tim1Psc,tim1Arr,tim8Psc,tim8Arr;
};

class TimConfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimConfDialog(QWidget *parent = nullptr);
    ~TimConfDialog();
    void setUiData(TimConf &conf);

signals:
    void onTim1PscChanged(uint16_t psc);
    void onTim1ArrChanged(uint16_t arr);
    void onTim8PscChanged(uint16_t psc);
    void onTim8ArrChanged(uint16_t arr);

private slots:
    void on_sb_tim1_psc_valueChanged(int arg1);
    void on_sb_tim1_arr_valueChanged(int arg1);
    void on_sb_tim8_psc_valueChanged(int arg1);
    void on_sb_tim8_arr_valueChanged(int arg1);

private:
    Ui::TimConfDialog *ui;
};

#endif // TIMCONFDIALOG_H
