#ifndef PIDDIALOG_H
#define PIDDIALOG_H

#include <QDialog>

namespace Ui {
class PidDialog;
}

//单级PID参数结构体
struct Q_PACKED SingPID{
    float kp,ki,kd,maxInt,maxOut;
};

//串级PID参数结构体
struct Q_PACKED CasPID{
    SingPID inner,outer;
};

class PidDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PidDialog(QWidget *parent = nullptr);
    ~PidDialog();
    void setData(const QString &title,SingPID &singPID,CasPID &casPID);
    void setDefaultParam(SingPID &spdPID,SingPID &singAngPID,CasPID &casAngPID);

private slots:
    void on_bt_ok_clicked();
    void on_bt_spd_pid_param_clicked();
    void on_bt_singang_pid_param_clicked();
    void on_bt_casang_pid_param_clicked();

private:
    Ui::PidDialog *ui;
    //指向需要修改的PID结构(实际对象在主窗口中定义)
    SingPID *curSingPID;
    CasPID *curCasPID;
    //指向默认PID参数
    SingPID *defSpdPID,*defSingAngPID;
    CasPID *defCasAngPID;
    void saveData(SingPID &singPID,CasPID &casPID);
    void updateUi(SingPID &singPID);
    void updateUi(CasPID &casPID);
    void updateUi(SingPID &singPID,CasPID &casPID);
};

#endif // PIDDIALOG_H
