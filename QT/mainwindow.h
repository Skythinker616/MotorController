#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qserialport.h>
#include <qserialportinfo.h>
#include <qmessagebox.h>
#include <string.h>
#include <piddialog.h>
#include <qdebug.h>
#include <ctrldialog.h>
#include <pwmctrldialog.h>
#include <qsettings.h>
#include <qfiledialog.h>
#include <timconfdialog.h>
#include <QTime>
#include <windows.h>
#include <aboutdialog.h>
#include <qdesktopservices.h>

#define FRAME_HEADER 0xAA

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

//界面样式
enum AppStyle{
    Style_Default,
    Style_LightBlue,
    Style_DarkBlue
};

//CAN电机类型
enum CanMotorType{
    Motor_UnknownType,
    Motor_3508,
    Motor_6020,
    Motor_2006
};

//CAN电机索引
enum CanMotorIndex{
    Can1Motor1,
    Can1Motor2,
    Can1Motor3,
    Can1Motor4,
    Can2Motor1,
    Can2Motor2,
    Can2Motor3,
    Can2Motor4,
    MaxCanMotorNum
};

//PWM电机索引
enum PwmMotorIndex{
    PwmMotor1,
    PwmMotor2,
    PwmMotor3,
    PwmMotor4,
    PwmMotor5,
    PwmMotor6,
    PwmMotor7,
    MaxPwmMotorNum
};

//通信数据帧命令码
enum FrameCmdCode{
    FrameCmd_SetMode,
    FrameCmd_SetTor,
    FrameCmd_SetSpd,
    FrameCmd_SetSingAng,
    FrameCmd_SetCasAng,
    FrameCmd_SetCasSpd,
    FrameCmd_SetPID,
    FrameCmd_SetWorkState,
    FrameCmd_StopAll,
    FrameCmd_SetType,
    FrameCmd_SetId,
    FrameCmd_SetTimConf,
    FrameCmd_SetPwmPsc,
    FrameCmd_SetPwmArr,
    FrameCmd_SetPwmCcr,
    FrameCmd_SetPwmWorkState
};

//CAN电机数据(每个CAN电机分配一个对象)
class CanMotor
{
public:
    bool enabled;//是否使能（即配置窗口最左端的复选框）
    QString noteTxt;//标识名称
    CanMotorType type;//电机类型
    uint8_t id;//电机ID
    SingPID singPID;//单级PID参数
    CasPID casPID;//串级PID参数
};

//PWM电机数据(每个PWM电机分配一个对象)
class PwmMotor
{
public:
    bool enabled;//是否使能（即配置窗口最左端的复选框）
    QString noteTxt;//标识名称
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCtrlModeChanged(uint8_t index,CtrlMode mode);
    void onTorChanged(uint8_t index,float percent);
    void onSpdChanged(uint8_t index,float spd);
    void onSingAngChanged(uint8_t index,float ang);
    void onCasAngChanged(uint8_t index,float ang);
    void onCasSpdChanged(uint8_t index,float spd);
    void onWorkStateChanged(uint8_t index,bool working);
    void onPwmPscChanged(uint8_t index,uint16_t psc);
    void onPwmArrChanged(uint8_t index,uint16_t arr);
    void onPwmCcrChanged(uint8_t index,uint16_t ccr);
    void onPwmWorkStateChanged(uint8_t index,bool working);
    void onTim1PscChanged(uint16_t psc);
    void onTim1ArrChanged(uint16_t arr);
    void onTim8PscChanged(uint16_t psc);
    void onTim8ArrChanged(uint16_t arr);
    void on_chb_can_mot_en_toggled(bool checked);
    void on_et_can_mot_name_textChanged(const QString &name);
    void on_cb_can_mot_type_currentTextChanged(const QString &typeName);
    void on_sb_can_mot_id_valueChanged(int id);
    void on_bt_can_mot_pid_clicked();
    void on_bt_can_mot_cmd_clicked();
    void on_chb_pwm_mot_en_toggled(bool checked);
    void on_et_pwm_mot_name_textChanged(const QString &name);
    void on_bt_pwm_mot_cmd_clicked();
    void on_bt_refresh_clicked();
    void on_bt_switch_clicked();
    void on_bt_tim_conf_clicked();
    void on_action_import_triggered();
    void on_action_export_triggered();
    void on_action_sync_triggered();
    void on_action_ports_help_triggered();
    void on_action_pwm_help_triggered();
    void on_action_can_help_triggered();
    void on_action_about_triggered();
    void on_action_help_triggered();
    void on_action_default_style_triggered();
    void on_action_light_blue_style_triggered();
    void on_action_dark_blue_style_triggered();

private:
    //各型号电机默认PID参数
    struct{
        SingPID spd{10,0.5,0,10000,20000};
        SingPID singAng{0.3,0,5,0,20000};
        CasPID casAng{{10,0.5,0,10000,20000},{0.3,0,0,0,1000}};
    }defPidParam3508;
    struct{
        SingPID spd{50,1,0,12000,20000};
        SingPID singAng{10,0.5,0,10000,20000};
        CasPID casAng{{50,1,0,12000,20000},{1,0,0,0,100}};
    }defPidParam6020;
    struct{
        SingPID spd{10,0.5,0,5000,10000};
        SingPID singAng{0.1,0,3,0,10000};
        CasPID casAng{{10,0.5,0,5000,10000},{0.1,0,0,0,1000}};
    }defPidParam2006;

    //各窗口指针
    Ui::MainWindow *ui;
    AboutDialog *aboutDialog;
    PidDialog *pidDialog;
    TimConfDialog *timConfDialog;
    CtrlDialog *ctrlDialogs[MaxCanMotorNum];//每个CAN电机一个，下标与电机索引对应
    PwmCtrlDialog *pwmDialogs[MaxPwmMotorNum];//每个PWM电机一个，下标与电机索引对应

    //电机数据结构体
    CanMotor canMotors[MaxCanMotorNum];//每个CAN电机一个，下标与电机索引对应
    PwmMotor pwmMotors[MaxPwmMotorNum];//每个PWM电机一个，下标与电机索引对应

    QSerialPort *port;//串口对象指针
    AppStyle appStyle=(AppStyle)-1;//当前界面样式(初值-1是为了在启动时调用样式切换)
    TimConf timConf{71,19999,71,19999};//定时器配置数据


    void initCtrlDialog(CanMotorIndex index);
    void initPwmDialog(PwmMotorIndex index);
    void initTimConf();
    void regAllWidget();
    void encodeFloat(void *buf,float value);
    void sendPidParam(CanMotorIndex index);
    void startPidConf(CanMotorIndex index);
    void sendType(CanMotorIndex index);
    CanMotorType nameToType(QString name);
    const QString typeToName(CanMotorType type);
    void sendId(CanMotorIndex index);
    void sendTimConf();
    void saveToFile(const QString &filename);
    void loadFromFile(const QString &filename);
    void updateUi();
    void sleep(uint32_t ms);
    void setAppStyle(AppStyle style);
    void loadAppStyleConf();
};
#endif // MAINWINDOW_H
