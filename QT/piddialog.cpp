#include "piddialog.h"
#include "ui_piddialog.h"

PidDialog::PidDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PidDialog)
{
    ui->setupUi(this);
    curSingPID=NULL;
    curCasPID=NULL;
}

PidDialog::~PidDialog()
{
    delete ui;
}

//设置窗口数据（包含窗口标题，当前需要操作的PID结构）
void PidDialog::setData(const QString &title, SingPID &singPID, CasPID &casPID)
{
    setWindowTitle(title);
    curSingPID=&singPID;
    curCasPID=&casPID;
    updateUi(*curSingPID,*curCasPID);
}

//设置默认参数
void PidDialog::setDefaultParam(SingPID &spdPID, SingPID &singAngPID, CasPID &casAngPID)
{
    defSpdPID=&spdPID;
    defSingAngPID=&singAngPID;
    defCasAngPID=&casAngPID;
}

//确认按钮按下
void PidDialog::on_bt_ok_clicked()
{
    if(curCasPID && curSingPID)
        saveData(*curSingPID,*curCasPID);//将配置好的数据写入结构体
    accept();
}

//将界面配置的数据写入结构体
void PidDialog::saveData(SingPID &singPID,CasPID &casPID)
{
    singPID.kp=ui->sb_sing_kp->value();
    singPID.ki=ui->sb_sing_ki->value();
    singPID.kd=ui->sb_sing_kd->value();
    singPID.maxInt=ui->sb_sing_mi->value();
    singPID.maxOut=ui->sb_sing_mo->value();

    casPID.inner.kp=ui->sb_in_kp->value();
    casPID.inner.ki=ui->sb_in_ki->value();
    casPID.inner.kd=ui->sb_in_kd->value();
    casPID.inner.maxInt=ui->sb_in_mi->value();
    casPID.inner.maxOut=ui->sb_in_mo->value();

    casPID.outer.kp=ui->sb_out_kp->value();
    casPID.outer.ki=ui->sb_out_ki->value();
    casPID.outer.kd=ui->sb_out_kd->value();
    casPID.outer.maxInt=ui->sb_out_mi->value();
    casPID.outer.maxOut=ui->sb_out_mo->value();
}

//用参数singPID更新单级PID部分界面
void PidDialog::updateUi(SingPID &singPID)
{
    ui->sb_sing_kp->setValue(singPID.kp);
    ui->sb_sing_ki->setValue(singPID.ki);
    ui->sb_sing_kd->setValue(singPID.kd);
    ui->sb_sing_mi->setValue(singPID.maxInt);
    ui->sb_sing_mo->setValue(singPID.maxOut);
}

//用参数casPID更新串级PID部分界面
void PidDialog::updateUi(CasPID &casPID)
{
    ui->sb_in_kp->setValue(casPID.inner.kp);
    ui->sb_in_ki->setValue(casPID.inner.ki);
    ui->sb_in_kd->setValue(casPID.inner.kd);
    ui->sb_in_mi->setValue(casPID.inner.maxInt);
    ui->sb_in_mo->setValue(casPID.inner.maxOut);

    ui->sb_out_kp->setValue(casPID.outer.kp);
    ui->sb_out_ki->setValue(casPID.outer.ki);
    ui->sb_out_kd->setValue(casPID.outer.kd);
    ui->sb_out_mi->setValue(casPID.outer.maxInt);
    ui->sb_out_mo->setValue(casPID.outer.maxOut);
}

//同时更新单级和串级的界面
void PidDialog::updateUi(SingPID &singPID,CasPID &casPID)
{
   updateUi(singPID);
   updateUi(casPID);
}

//设置默认参数按钮槽函数
void PidDialog::on_bt_spd_pid_param_clicked()
{
    if(defSpdPID)
        updateUi(*defSpdPID);
}
void PidDialog::on_bt_singang_pid_param_clicked()
{
    if(defSingAngPID)
        updateUi(*defSingAngPID);
}
void PidDialog::on_bt_casang_pid_param_clicked()
{
    if(defCasAngPID)
        updateUi(*defCasAngPID);
}
