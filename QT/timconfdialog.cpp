#include "timconfdialog.h"
#include "ui_timconfdialog.h"

TimConfDialog::TimConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimConfDialog)
{
    ui->setupUi(this);
    setWindowTitle("定时器配置");
}

TimConfDialog::~TimConfDialog()
{
    delete ui;
}

void TimConfDialog::setUiData(TimConf &conf)
{
    ui->sb_tim1_psc->setValue(conf.tim1Psc);
    ui->sb_tim1_arr->setValue(conf.tim1Arr);
    ui->sb_tim8_psc->setValue(conf.tim8Psc);
    ui->sb_tim8_arr->setValue(conf.tim8Arr);
}

void TimConfDialog::on_sb_tim1_psc_valueChanged(int arg1)
{
    emit onTim1PscChanged(arg1);
}

void TimConfDialog::on_sb_tim1_arr_valueChanged(int arg1)
{
    emit onTim1ArrChanged(arg1);
}

void TimConfDialog::on_sb_tim8_psc_valueChanged(int arg1)
{
    emit onTim8PscChanged(arg1);
}

void TimConfDialog::on_sb_tim8_arr_valueChanged(int arg1)
{
    emit onTim8ArrChanged(arg1);
}
