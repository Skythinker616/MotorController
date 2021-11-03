#include "ctrldialog.h"
#include "ui_ctrldialog.h"

CtrlDialog::CtrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CtrlDialog)
{
    ui->setupUi(this);
    working=false;
}

CtrlDialog::~CtrlDialog()
{
    delete ui;
}

//设置窗口信息（包括窗口标题和ID）
void CtrlDialog::setWindowInfo(const QString &title, uint8_t id)
{
    setWindowTitle(title);
    windowID=id;
}

//导出配置到参数conf中
void CtrlDialog::getConf(CtrlDialogConf &conf)
{
    //导出控制模式
    if(ui->rb_tor->isChecked())
        conf.mode=Ctrl_Tor;
    else if(ui->rb_spd->isChecked())
        conf.mode=Ctrl_Spd;
    else if(ui->rb_sing_ang->isChecked())
        conf.mode=Ctrl_SingAng;
    else if(ui->rb_cas_ang->isChecked())
        conf.mode=Ctrl_CasAng;
    else
        conf.mode=Ctrl_Tor;

    //导出各滑块的上下限
    conf.minSpd=ui->sb_spd_min->value();
    conf.maxSpd=ui->sb_spd_max->value();
    conf.minSingAng=ui->sb_sing_ang_min->value();
    conf.maxSingAng=ui->sb_sing_ang_max->value();
    conf.minCasAng=ui->sb_cas_ang_min->value();
    conf.maxCasAng=ui->sb_cas_ang_max->value();
    conf.casSpd=ui->sb_cas_spd->value();
}

//从参数conf导入配置
void CtrlDialog::setConf(CtrlDialogConf &conf)
{
    //导入控制模式
    switch(conf.mode)
    {
        case Ctrl_Tor: ui->rb_tor->setChecked(true); break;
        case Ctrl_Spd: ui->rb_spd->setChecked(true); break;
        case Ctrl_SingAng: ui->rb_sing_ang->setChecked(true); break;
        case Ctrl_CasAng: ui->rb_cas_ang->setChecked(true); break;
    }

    //导入各滑块的上下限
    ui->sb_spd_min->setValue(conf.minSpd);
    ui->sb_spd_max->setValue(conf.maxSpd);
    ui->sb_sing_ang_min->setValue(conf.minSingAng);
    ui->sb_sing_ang_max->setValue(conf.maxSingAng);
    ui->sb_cas_ang_min->setValue(conf.minCasAng);
    ui->sb_cas_ang_max->setValue(conf.maxCasAng);
    ui->sb_cas_spd->setValue(conf.casSpd);
}

//各个滑块使能复选框的槽函数（即设置对应控件的使能状态）
void CtrlDialog::on_chb_tor_slider_toggled(bool checked)
{
    ui->sl_tor->setEnabled(checked);
}
void CtrlDialog::on_chb_spd_slider_toggled(bool checked)
{
    ui->sl_spd->setEnabled(checked);
    ui->sb_spd_min->setEnabled(checked);
    ui->sb_spd_max->setEnabled(checked);
}
void CtrlDialog::on_chb_sing_ang_slider_toggled(bool checked)
{
    ui->sl_sing_ang->setEnabled(checked);
    ui->sb_sing_ang_min->setEnabled(checked);
    ui->sb_sing_ang_max->setEnabled(checked);
}
void CtrlDialog::on_chb_cas_ang_slider_toggled(bool checked)
{
    ui->sl_cas_ang->setEnabled(checked);
    ui->sb_cas_ang_min->setEnabled(checked);
    ui->sb_cas_ang_max->setEnabled(checked);
}

//修改目标扭矩的槽函数
void CtrlDialog::on_sl_tor_valueChanged(int value)
{
    ui->sb_tor->setValue(value/1000.0f*100);
}
void CtrlDialog::on_sb_tor_editingFinished()
{
    ui->sl_tor->setValue(ui->sb_tor->value()/100.0f*1000);
}
void CtrlDialog::on_sb_tor_valueChanged(double arg1)
{
    emit torChanged(windowID,arg1);
}

//修改目标速度的槽函数
void CtrlDialog::on_sl_spd_valueChanged(int value)
{
    ui->sb_spd->setValue(value/1000.0f*(ui->sb_spd_max->value()-ui->sb_spd_min->value())+ui->sb_spd_min->value());
}
void CtrlDialog::on_sb_spd_valueChanged(double arg1)
{
    emit spdChanged(windowID,arg1);
}

//修改单级目标角度的槽函数
void CtrlDialog::on_sl_sing_ang_valueChanged(int value)
{
    ui->sb_sing_ang->setValue(value/1000.0f*(ui->sb_sing_ang_max->value()-ui->sb_sing_ang_min->value())+ui->sb_sing_ang_min->value());
}
void CtrlDialog::on_sb_sing_ang_valueChanged(double arg1)
{
    emit singAngChanged(windowID,arg1);
}

//修改串级目标角度的槽函数
void CtrlDialog::on_sl_cas_ang_valueChanged(int value)
{
    ui->sb_cas_ang->setValue(value/1000.0f*(ui->sb_cas_ang_max->value()-ui->sb_cas_ang_min->value())+ui->sb_cas_ang_min->value());
}
void CtrlDialog::on_sb_cas_ang_valueChanged(double arg1)
{
    emit casAngChanged(windowID,arg1);
}

//修改串级速度限幅的槽函数
void CtrlDialog::on_sb_cas_spd_valueChanged(double arg1)
{
    emit casSpdChanged(windowID,arg1);
}

//启停电机按钮槽函数
void CtrlDialog::on_bt_work_clicked()
{
    if(working)
    {
        working=false;
        ui->bt_work->setText("启动");
    }
    else
    {
        working=true;
        ui->bt_work->setText("停止");
    }
    emit workStateChanged(windowID,working);
}

//切换控制模式的槽函数
void CtrlDialog::on_rb_tor_toggled(bool checked)
{
    if(checked)
        emit modeChanged(windowID,Ctrl_Tor);
}
void CtrlDialog::on_rb_spd_toggled(bool checked)
{
    if(checked)
        emit modeChanged(windowID,Ctrl_Spd);
}
void CtrlDialog::on_rb_sing_ang_toggled(bool checked)
{
    if(checked)
        emit modeChanged(windowID,Ctrl_SingAng);
}
void CtrlDialog::on_rb_cas_ang_toggled(bool checked)
{
    if(checked)
        emit modeChanged(windowID,Ctrl_CasAng);
}
