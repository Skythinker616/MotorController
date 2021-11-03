#include "pwmctrldialog.h"
#include "ui_pwmctrldialog.h"

PwmCtrlDialog::PwmCtrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PwmCtrlDialog)
{
    ui->setupUi(this);
    working=false;
}

PwmCtrlDialog::~PwmCtrlDialog()
{
    delete ui;
}

//设置窗口信息（包括窗口标题和ID）
void PwmCtrlDialog::setWindowInfo(const QString &title, uint8_t id)
{
    setWindowTitle(title);
    windowID=id;
}

//设置显示的分频系数
void PwmCtrlDialog::setPsc(uint16_t psc)
{
    ui->txt_psc->setText(QString("分频系数=%1").arg(psc));
}

//电机型号修改槽函数
void PwmCtrlDialog::on_cb_type_currentTextChanged(const QString &arg1)
{
    if(arg1=="180°舵机")
    {
        curType=PwmMotor_Servo180;//设置当前类型
        ui->txt_value_type->setText("角度值(°)");
        ui->sb_value->setRange(0,180);//设置可调范围
        ui->txt_min->setText("0");
        ui->txt_max->setText("180");
        emit onArrChanged(windowID,19999);//发送信号传出新的重装载值和分频系数
        emit onPscChanged(windowID,71);
    }
    if(arg1=="270°舵机")
    {
        curType=PwmMotor_Servo270;
        ui->txt_value_type->setText("角度值(°)");
        ui->sb_value->setRange(0,270);
        ui->txt_min->setText("0");
        ui->txt_max->setText("270");
        emit onArrChanged(windowID,19999);
        emit onPscChanged(windowID,71);
    }
}

//电机控制数值改变槽函数
void PwmCtrlDialog::on_sb_value_valueChanged(double arg1)
{
    switch(curType)
    {
        case PwmMotor_Servo180:
        emit onCcrChanged(windowID,arg1/180.0f*2000+500);//修改比较值
        break;
        case PwmMotor_Servo270:
        emit onCcrChanged(windowID,arg1/270.0f*2000+500);
        break;
        default:
        break;
    }
}

//设定窗口显示的重装载值
void PwmCtrlDialog::setArr(uint16_t arr)
{
    ui->txt_arr->setText(QString("重装载值=%1").arg(arr));
    ui->sb_ccr->setRange(0,arr);
    ui->sb_ccr_max->setRange(0,arr);
    ui->sb_ccr_min->setRange(0,arr);
}

//导出配置数据（用于主窗口保存时调用）
void PwmCtrlDialog::getConf(PwmCtrlDialogConf &conf)
{
    if(ui->rb_mot->isChecked())
        conf.mode=PwmCtrl_Motor;
    else
        conf.mode=PwmCtrl_Param;
    conf.motorType=nameToType(ui->cb_type->currentText());
    conf.minCcr=ui->sb_ccr_min->value();
    conf.maxCcr=ui->sb_ccr_max->value();
}

//设置配置数据（用于主窗口导入时调用）
void PwmCtrlDialog::setConf(PwmCtrlDialogConf &conf)
{
    if(conf.mode==PwmCtrl_Motor)
        ui->rb_mot->setChecked(true);
    else if(conf.mode==PwmCtrl_Param)
        ui->rb_pwm->setChecked(true);
    ui->cb_type->setCurrentText(typeToName(conf.motorType));
    ui->sb_ccr_min->setValue(conf.minCcr);
    ui->sb_ccr_max->setValue(conf.maxCcr);
}

//电机数据拖动条数值改变槽函数
void PwmCtrlDialog::on_sl_mot_valueChanged(int value)
{
    ui->sb_value->setValue(value/1000.0f*(ui->txt_max->text().toInt()-ui->txt_min->text().toInt())+ui->txt_min->text().toInt());
}

//电机控制模式单选框切换槽函数
void PwmCtrlDialog::on_rb_mot_toggled(bool checked)
{
    ui->cb_type->setEnabled(checked);//设置关于电机控制模式的控件使能状态
    ui->sb_value->setEnabled(checked);
    ui->sl_mot->setEnabled(checked);
}

//PWM控制模式单选框切换槽函数
void PwmCtrlDialog::on_rb_pwm_toggled(bool checked)
{
    ui->sb_ccr->setEnabled(checked);//设置关于PWM控制模式的控件使能状态
    ui->sb_ccr_min->setEnabled(checked);
    ui->sb_ccr_max->setEnabled(checked);
    ui->sl_ccr->setEnabled(checked);
}

//切换输出开关槽函数
void PwmCtrlDialog::on_bt_work_clicked()
{
    if(working)
    {
        working=false;
        ui->bt_work->setText("开启输出");
        emit onWorkStateChanged(windowID,false);//发送信号
    }
    else
    {
        working=true;
        ui->bt_work->setText("停止输出");
        emit onWorkStateChanged(windowID,true);
    }
}

//PWM控制模式下比较值修改槽函数
void PwmCtrlDialog::on_sb_ccr_valueChanged(int arg1)
{
    emit onCcrChanged(windowID,arg1);
}

//PWM控制模式下比较值滑条拖动槽函数
void PwmCtrlDialog::on_sl_ccr_valueChanged(int value)
{
    ui->sb_ccr->setValue(value/1000.0f*(ui->sb_ccr_max->value()-ui->sb_ccr_min->value())+ui->sb_ccr_min->value());
}

//将电机类型枚举转换为名称字符串
const QString PwmCtrlDialog::typeToName(PwmMotorType type)
{
    switch(type)
    {
        case PwmMotor_Servo180: return "180°舵机";
        case PwmMotor_Servo270: return "270°舵机";
        default: return "";
    }
}

//将电机类型名称字符串转换为枚举
PwmMotorType PwmCtrlDialog::nameToType(const QString &name)
{
    if(name=="180°舵机")
        return PwmMotor_Servo180;
    else if(name=="270°舵机")
        return PwmMotor_Servo270;
    return PwmMotor_UnknownType;
}
