#ifndef PWMCTRLDIALOG_H
#define PWMCTRLDIALOG_H

#include <QDialog>

namespace Ui {
class PwmCtrlDialog;
}

//PWM电机类型枚举
enum PwmMotorType{
    PwmMotor_UnknownType,
    PwmMotor_Servo180,
    PwmMotor_Servo270
};

//PWM控制模式枚举
enum PwmCtrlMode{
    PwmCtrl_Motor,
    PwmCtrl_Param
};

//控制窗口配置数据，存放需要被导出保存的数据
class PwmCtrlDialogConf{
public:
    PwmCtrlMode mode;
    PwmMotorType motorType;
    uint16_t minCcr,maxCcr;
};

class PwmCtrlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PwmCtrlDialog(QWidget *parent = nullptr);
    ~PwmCtrlDialog();
    void setWindowInfo(const QString &title,uint8_t id);
    void setPsc(uint16_t psc);
    void setArr(uint16_t arr);
    void getConf(PwmCtrlDialogConf &conf);
    void setConf(PwmCtrlDialogConf &conf);

signals:
    void onPscChanged(uint8_t id,uint16_t psc);
    void onArrChanged(uint8_t id,uint16_t arr);
    void onCcrChanged(uint8_t id,uint16_t ccr);
    void onWorkStateChanged(uint8_t id,bool working);

private slots:
    void on_cb_type_currentTextChanged(const QString &arg1);
    void on_sb_value_valueChanged(double arg1);
    void on_sl_mot_valueChanged(int value);
    void on_rb_mot_toggled(bool checked);
    void on_rb_pwm_toggled(bool checked);
    void on_bt_work_clicked();
    void on_sb_ccr_valueChanged(int arg1);
    void on_sl_ccr_valueChanged(int value);

private:
    Ui::PwmCtrlDialog *ui;
    bool working=false;//标记当前PWM通道是否开启
    uint8_t windowID=0;//窗口ID（代表绑定的电机索引）
    uint16_t curARR=20000,curCCR=0;//当前重装载值和比较值
    PwmMotorType curType=PwmMotor_UnknownType;//当前电机类型
    const QString typeToName(PwmMotorType type);
    PwmMotorType nameToType(const QString &name);
};

#endif // PWMCTRLDIALOG_H
