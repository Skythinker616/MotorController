#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("MotorController");

    loadAppStyleConf();//加载界面样式设置

    regAllWidget();//注册电机相关控件事件

    aboutDialog=new AboutDialog();
    pidDialog=new PidDialog();
    port=new QSerialPort();
    timConfDialog=new TimConfDialog();

    for(uint8_t i=0;i<MaxCanMotorNum;i++)//初始化CAN电机
    {
        canMotors[i].type=Motor_UnknownType;
        canMotors[i].id=0;
        memset(&canMotors[i].singPID,0,sizeof(SingPID));
        memset(&canMotors[i].casPID,0,sizeof(CasPID));
        initCtrlDialog((CanMotorIndex)i);
    }

    for(uint8_t i=0;i<MaxPwmMotorNum;i++)//初始化PWM电机
    {
        initPwmDialog((PwmMotorIndex)i);
    }

    initTimConf();//初始化定时器相关配置及窗口事件
}

MainWindow::~MainWindow()
{
    delete ui;
    delete pidDialog;
    delete port;
}

//在CAN控制模式切换时调用
void MainWindow::onCtrlModeChanged(uint8_t index, CtrlMode mode)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetMode,(char)index,(char)mode};//向下位机发送控制模式
    port->write(buf,sizeof(buf));
}

//在目标扭矩变化时调用
void MainWindow::onTorChanged(uint8_t index, float percent)
{
    if(!port->isOpen())
        return;
    char buf[3+4]={(char)FRAME_HEADER,(char)FrameCmd_SetTor,(char)index};//向下位机发送目标扭矩
    encodeFloat(buf+3,percent);
    port->write(buf,sizeof(buf));
}

//在目标速度变化时调用
void MainWindow::onSpdChanged(uint8_t index, float spd)
{
    if(!port->isOpen())
        return;
    char buf[3+4]={(char)FRAME_HEADER,(char)FrameCmd_SetSpd,(char)index};//向下位机发送目标速度
    encodeFloat(buf+3,spd);
    port->write(buf,sizeof(buf));
}

//在单级角度环目标角度变化时调用
void MainWindow::onSingAngChanged(uint8_t index, float ang)
{
    if(!port->isOpen())
        return;
    char buf[3+4]={(char)FRAME_HEADER,(char)FrameCmd_SetSingAng,(char)index};//向下位机发送目标速度
    encodeFloat(buf+3,ang);
    port->write(buf,sizeof(buf));
}

//在串级角度环目标角度变化时调用
void MainWindow::onCasAngChanged(uint8_t index, float ang)
{
    if(!port->isOpen())
        return;
    char buf[3+4]={(char)FRAME_HEADER,(char)FrameCmd_SetCasAng,(char)index};//向下位机发送目标速度
    encodeFloat(buf+3,ang);
    port->write(buf,sizeof(buf));
}

//在串级角度环速度限幅变化时调用
void MainWindow::onCasSpdChanged(uint8_t index, float spd)
{
    if(!port->isOpen())
        return;
    canMotors[index].casPID.outer.maxOut=spd;//更新串级PID数据
    char buf[3+4]={(char)FRAME_HEADER,(char)FrameCmd_SetCasSpd,(char)index};//向下位机发送速度限幅
    encodeFloat(buf+3,spd);
    port->write(buf,sizeof(buf));
}

//在CAN电机启停状态变化时调用
void MainWindow::onWorkStateChanged(uint8_t index, bool working)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetWorkState,(char)index,(char)working};//向下位机发送启停状态
    port->write(buf,sizeof(buf));
}

//在PWM分频系数变化时调用
void MainWindow::onPwmPscChanged(uint8_t index, uint16_t psc)
{
    if(index<=PwmMotor4)//根据通道修改对应定时器的分频系数
        onTim1PscChanged(psc);
    else
        onTim8PscChanged(psc);
}

//在PWM重装载值变化时调用
void MainWindow::onPwmArrChanged(uint8_t index, uint16_t arr)
{
    if(index<=PwmMotor4)//根据通道修改对应定时器的重装载值
        onTim1ArrChanged(arr);
    else
        onTim8ArrChanged(arr);
}

//在PWM比较值变化时调用
void MainWindow::onPwmCcrChanged(uint8_t index, uint16_t ccr)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmCcr,(char)index,(char)(ccr>>8),(char)(ccr&0xFF)};//向下位机发送比较值
    port->write(buf,sizeof(buf));
}

//在PWM启停状态变化时调用
void MainWindow::onPwmWorkStateChanged(uint8_t index, bool working)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmWorkState,(char)index,(char)working};//向下位机发送启停状态
    port->write(buf,sizeof(buf));
}

//在TIM1分频系数变化时调用
void MainWindow::onTim1PscChanged(uint16_t psc)
{
    if(timConf.tim1Psc!=psc && port->isOpen())//若新值与记录的psc值不一样则向下位机发送新的分频系数
    {
        char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmPsc,(char)PwmMotor1,(char)(psc>>8),(char)(psc&0xFF)};
        port->write(buf,sizeof(buf));
    }
    timConf.tim1Psc=psc;//更新当前psc记录值
    for(uint8_t index=0;index<=PwmMotor4;index++)//更新各PWM操控窗口的数据
        pwmDialogs[index]->setPsc(psc);
    timConfDialog->setUiData(timConf);//更新TIM配置窗口数据
}

//在TIM1重装载值变化时调用
void MainWindow::onTim1ArrChanged(uint16_t arr)
{
    if(timConf.tim1Arr!=arr && port->isOpen())//若新值与记录的arr值不一样则向下位机发送新的重装载值
    {
        char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmArr,(char)PwmMotor1,(char)(arr>>8),(char)(arr&0xFF)};
        port->write(buf,sizeof(buf));
    }
    timConf.tim1Arr=arr;//更新当前arr记录值
    for(uint8_t index=0;index<=PwmMotor4;index++)//更新各PWM操控窗口的数据
        pwmDialogs[index]->setArr(arr);
    timConfDialog->setUiData(timConf);//更新TIM配置窗口数据
}

//在TIM8分频系数变化时调用，操作与TIM1相同
void MainWindow::onTim8PscChanged(uint16_t psc)
{
    if(timConf.tim8Psc!=psc && port->isOpen())
    {
        char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmPsc,(char)PwmMotor5,(char)(psc>>8),(char)(psc&0xFF)};
        port->write(buf,sizeof(buf));
    }
    timConf.tim8Psc=psc;
    for(uint8_t index=PwmMotor5;index<=PwmMotor7;index++)
        pwmDialogs[index]->setPsc(psc);
    timConfDialog->setUiData(timConf);
}

//在TIM8重装载值变化时调用，操作与TIM1相同
void MainWindow::onTim8ArrChanged(uint16_t arr)
{
    if(timConf.tim8Arr!=arr && port->isOpen())
    {
        char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetPwmArr,(char)PwmMotor5,(char)(arr>>8),(char)(arr&0xFF)};
        port->write(buf,sizeof(buf));
    }
    timConf.tim8Arr=arr;
    for(uint8_t index=PwmMotor5;index<=PwmMotor7;index++)
        pwmDialogs[index]->setArr(arr);
    timConfDialog->setUiData(timConf);
}

//在CAN电机使能框勾选时调用
void MainWindow::on_chb_can_mot_en_toggled(bool checked)
{
    //计算发送者的信息
    QString checkBoxName=qobject_cast<QCheckBox*>(sender())->objectName();
    uint8_t canNum=checkBoxName.at(7).toLatin1()-'0',motNum=checkBoxName.at(12).toLatin1()-'0';
    uint8_t index=(canNum-1)*4+(motNum-1);
    //使能或失能对应的控件
    findChild<QLineEdit*>(QString("et_can%1_mot%2_name").arg(canNum).arg(motNum))->setEnabled(checked);
    findChild<QComboBox*>(QString("cb_can%1_mot%2_type").arg(canNum).arg(motNum))->setEnabled(checked);
    findChild<QSpinBox*>(QString("sb_can%1_mot%2_id").arg(canNum).arg(motNum))->setEnabled(checked);
    findChild<QPushButton*>(QString("bt_can%1_mot%2_pid").arg(canNum).arg(motNum))->setEnabled(checked);
    findChild<QPushButton*>(QString("bt_can%1_mot%2_cmd").arg(canNum).arg(motNum))->setEnabled(checked);
    //更新电机数据
    canMotors[index].enabled=checked;
}

//在CAN电机标识名称被编辑时调用
void MainWindow::on_et_can_mot_name_textChanged(const QString &name)
{
    //计算发送者对应的index
    QString lineEditName=qobject_cast<QLineEdit*>(sender())->objectName();
    uint8_t index=(lineEditName.at(6).toLatin1()-'1')*4+(lineEditName.at(11).toLatin1()-'1');
    //更新电机数据
    canMotors[index].noteTxt=name;
}

//在CAN电机型号修改时调用
void MainWindow::on_cb_can_mot_type_currentTextChanged(const QString &typeName)
{
    //计算发送者对应的index
    QString comboBoxName=qobject_cast<QComboBox*>(sender())->objectName();
    uint8_t index=(comboBoxName.at(6).toLatin1()-'1')*4+(comboBoxName.at(11).toLatin1()-'1');
    //更新电机数据
    canMotors[index].type=nameToType(typeName);
    //向下位机发送类型信息
    sendType((CanMotorIndex)index);
}

//在CAN电机ID修改时调用
void MainWindow::on_sb_can_mot_id_valueChanged(int id)
{
    //计算发送者对应index
    QString spinBoxName=qobject_cast<QSpinBox*>(sender())->objectName();
    uint8_t index=(spinBoxName.at(6).toLatin1()-'1')*4+(spinBoxName.at(11).toLatin1()-'1');
    canMotors[index].id=id;//更新电机数据
    sendId((CanMotorIndex)index);//向下位机发送ID信息
}

//在CAN电机PID配置按钮点击时调用
void MainWindow::on_bt_can_mot_pid_clicked()
{
    //计算发送者对应index
    QString pushButtonName=qobject_cast<QPushButton*>(sender())->objectName();
    uint8_t index=(pushButtonName.at(6).toLatin1()-'1')*4+(pushButtonName.at(11).toLatin1()-'1');
    //触发PID配置
    startPidConf((CanMotorIndex)index);
}

//在CAN电机进入操控按钮点击时调用
void MainWindow::on_bt_can_mot_cmd_clicked()
{
    //计算发送者对应index
    QString pushButtonName=qobject_cast<QPushButton*>(sender())->objectName();
    uint8_t index=(pushButtonName.at(6).toLatin1()-'1')*4+(pushButtonName.at(11).toLatin1()-'1');
    //显示操控窗口
    ctrlDialogs[index]->show();
    ctrlDialogs[index]->activateWindow();
}

//在PWM电机使能框状态变化时调用
void MainWindow::on_chb_pwm_mot_en_toggled(bool checked)
{
    //计算发送者信息
    QString checkBoxName=qobject_cast<QCheckBox*>(sender())->objectName();
    uint8_t motNum=checkBoxName.at(11).toLatin1()-'0';
    //使能对应的控件
    findChild<QLineEdit*>(QString("et_pwm_mot%1_name").arg(motNum))->setEnabled(checked);
    findChild<QPushButton*>(QString("bt_pwm_mot%1_cmd").arg(motNum))->setEnabled(checked);
    //更新电机数据
    pwmMotors[motNum-1].enabled=checked;
}

//在PWM电机标识名称被编辑时调用
void MainWindow::on_et_pwm_mot_name_textChanged(const QString &name)
{
    //计算发送者对应的index
    QString lineEditName=qobject_cast<QLineEdit*>(sender())->objectName();
    uint8_t index=lineEditName.at(10).toLatin1()-'1';
    //更新电机数据
    pwmMotors[index].noteTxt=name;
}

//在PWM电机进入操控按钮点击时调用
void MainWindow::on_bt_pwm_mot_cmd_clicked()
{
    //计算发送者对应的index
    QString pushButtonName=qobject_cast<QPushButton*>(sender())->objectName();
    uint8_t index=pushButtonName.at(10).toLatin1()-'1';
    //显示PWM操控窗口
    pwmDialogs[index]->show();
    pwmDialogs[index]->activateWindow();
}

//初始化CAN电机操控窗口
void MainWindow::initCtrlDialog(CanMotorIndex index)
{
    ctrlDialogs[index]=new CtrlDialog();
    ctrlDialogs[index]->setWindowInfo(QString("控制窗口：CAN%1-电机%2").arg(index/4+1).arg(index%4+1),index);//设置窗口标题和ID
    //连接操控窗口信号和主窗口的槽
    connect(ctrlDialogs[index],SIGNAL(modeChanged(uint8_t,CtrlMode)),this,SLOT(onCtrlModeChanged(uint8_t,CtrlMode)));
    connect(ctrlDialogs[index],SIGNAL(torChanged(uint8_t,float)),this,SLOT(onTorChanged(uint8_t,float)));
    connect(ctrlDialogs[index],SIGNAL(spdChanged(uint8_t,float)),this,SLOT(onSpdChanged(uint8_t,float)));
    connect(ctrlDialogs[index],SIGNAL(singAngChanged(uint8_t,float)),this,SLOT(onSingAngChanged(uint8_t,float)));
    connect(ctrlDialogs[index],SIGNAL(casAngChanged(uint8_t,float)),this,SLOT(onCasAngChanged(uint8_t,float)));
    connect(ctrlDialogs[index],SIGNAL(casSpdChanged(uint8_t,float)),this,SLOT(onCasSpdChanged(uint8_t,float)));
    connect(ctrlDialogs[index],SIGNAL(workStateChanged(uint8_t,bool)),this,SLOT(onWorkStateChanged(uint8_t,bool)));
}

//初始化PWM电机操控窗口
void MainWindow::initPwmDialog(PwmMotorIndex index)
{
    pwmDialogs[index]=new PwmCtrlDialog();
    pwmDialogs[index]->setWindowInfo(QString("PWM控制：PWM电机%1").arg(index+1),index);//设置窗口标题和ID
    //连接操控窗口信号和主窗口的槽
    connect(pwmDialogs[index],SIGNAL(onPscChanged(uint8_t,uint16_t)),this,SLOT(onPwmPscChanged(uint8_t,uint16_t)));
    connect(pwmDialogs[index],SIGNAL(onArrChanged(uint8_t,uint16_t)),this,SLOT(onPwmArrChanged(uint8_t,uint16_t)));
    connect(pwmDialogs[index],SIGNAL(onCcrChanged(uint8_t,uint16_t)),this,SLOT(onPwmCcrChanged(uint8_t,uint16_t)));
    connect(pwmDialogs[index],SIGNAL(onWorkStateChanged(uint8_t,bool)),this,SLOT(onPwmWorkStateChanged(uint8_t,bool)));
}

//初始化定时器配置窗口的信号槽
void MainWindow::initTimConf()
{
    //连接定时器配置窗口的信号和主窗口的槽
    connect(timConfDialog,SIGNAL(onTim1PscChanged(uint16_t)),this,SLOT(onTim1PscChanged(uint16_t)));
    connect(timConfDialog,SIGNAL(onTim1ArrChanged(uint16_t)),this,SLOT(onTim1ArrChanged(uint16_t)));
    connect(timConfDialog,SIGNAL(onTim8PscChanged(uint16_t)),this,SLOT(onTim8PscChanged(uint16_t)));
    connect(timConfDialog,SIGNAL(onTim8ArrChanged(uint16_t)),this,SLOT(onTim8ArrChanged(uint16_t)));
}

//注册主窗口所有电机相关控件的信号槽
void MainWindow::regAllWidget()
{
    for(uint8_t index=0;index<MaxCanMotorNum;index++)//CAN电机
    {
        connect(findChild<QCheckBox*>(QString("chb_can%1_mot%2_en").arg(index/4+1).arg(index%4+1)),SIGNAL(toggled(bool)),this,SLOT(on_chb_can_mot_en_toggled(bool)));
        connect(findChild<QLineEdit*>(QString("et_can%1_mot%2_name").arg(index/4+1).arg(index%4+1)),SIGNAL(textChanged(const QString&)),this,SLOT(on_et_can_mot_name_textChanged(const QString&)));
        connect(findChild<QComboBox*>(QString("cb_can%1_mot%2_type").arg(index/4+1).arg(index%4+1)),SIGNAL(currentTextChanged(const QString&)),this,SLOT(on_cb_can_mot_type_currentTextChanged(const QString&)));
        connect(findChild<QSpinBox*>(QString("sb_can%1_mot%2_id").arg(index/4+1).arg(index%4+1)),SIGNAL(valueChanged(int)),this,SLOT(on_sb_can_mot_id_valueChanged(int)));
        connect(findChild<QPushButton*>(QString("bt_can%1_mot%2_pid").arg(index/4+1).arg(index%4+1)),SIGNAL(clicked()),this,SLOT(on_bt_can_mot_pid_clicked()));
        connect(findChild<QPushButton*>(QString("bt_can%1_mot%2_cmd").arg(index/4+1).arg(index%4+1)),SIGNAL(clicked()),this,SLOT(on_bt_can_mot_cmd_clicked()));
    }
    for(uint8_t index=0;index<MaxPwmMotorNum;index++)//PWM电机
    {
        connect(findChild<QCheckBox*>(QString("chb_pwm_mot%1_en").arg(index+1)),SIGNAL(toggled(bool)),this,SLOT(on_chb_pwm_mot_en_toggled(bool)));
        connect(findChild<QLineEdit*>(QString("et_pwm_mot%1_name").arg(index+1)),SIGNAL(textChanged(const QString&)),this,SLOT(on_et_pwm_mot_name_textChanged(const QString&)));
        connect(findChild<QPushButton*>(QString("bt_pwm_mot%1_cmd").arg(index+1)),SIGNAL(clicked()),this,SLOT(on_bt_pwm_mot_cmd_clicked()));
    }
}

//将float数据编码为字节并写入目标地址
void MainWindow::encodeFloat(void *buf, float value)
{
    memcpy(buf,&value,4);//直接使用内存拷贝
}

//向下位机发送PID参数
void MainWindow::sendPidParam(CanMotorIndex index)
{
    if(!port->isOpen())
        return;
    char buf[3+sizeof(SingPID)+sizeof(CasPID)]={(char)FRAME_HEADER,(char)FrameCmd_SetPID,(char)index};
    memcpy(buf+3,&canMotors[index].singPID,sizeof(SingPID));//将单级和串级PID结构体的内存直接复制到缓冲区中发送
    memcpy(buf+3+sizeof(SingPID),&canMotors[index].casPID,sizeof(CasPID));
    port->write(buf,sizeof(buf));
}

//触发PID配置
void MainWindow::startPidConf(CanMotorIndex index)
{
    //设置窗口标题和要配置的PID结构体
    pidDialog->setData(QString("PID配置：CAN%1-电机%2").arg(index/4+1).arg(index%4+1),canMotors[index].singPID,canMotors[index].casPID);
    switch(canMotors[index].type)//根据所选的电机型号设置PID默认参数
    {
        case Motor_3508:
        pidDialog->setDefaultParam(defPidParam3508.spd,defPidParam3508.singAng,defPidParam3508.casAng);
        break;
        case Motor_6020:
        pidDialog->setDefaultParam(defPidParam6020.spd,defPidParam6020.singAng,defPidParam6020.casAng);
        break;
        case Motor_2006:
        pidDialog->setDefaultParam(defPidParam2006.spd,defPidParam2006.singAng,defPidParam2006.casAng);
        break;
        default:
        break;
    }

    if(pidDialog->exec()==QDialog::Accepted)//等待用户配置完成，向下位机发送数据
        sendPidParam(index);
}

//向下位机发送CAN电机类型
void MainWindow::sendType(CanMotorIndex index)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetType,(char)index,(char)canMotors[index].type};
    port->write(buf,sizeof(buf));
}

//CAN电机类型名称转换为枚举量
CanMotorType MainWindow::nameToType(QString name)
{
    if(name=="3508")
        return Motor_3508;
    else if(name=="6020")
        return Motor_6020;
    else if(name=="2006")
        return Motor_2006;
    return Motor_UnknownType;
}

//CAN电机类型枚举量转换为类型名称
const QString MainWindow::typeToName(CanMotorType type)
{
    if(type==Motor_3508)
        return "3508";
    if(type==Motor_6020)
        return "6020";
    if(type==Motor_2006)
        return "2006";
    return "";
}

//向下位机发送CAN电机ID
void MainWindow::sendId(CanMotorIndex index)
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetId,(char)index,(char)canMotors[index].id};
    port->write(buf,sizeof(buf));
}

//向下位机完整发送TIM配置（包含tim1和tim8的psc和arr）
void MainWindow::sendTimConf()
{
    if(!port->isOpen())
        return;
    char buf[]={(char)FRAME_HEADER,(char)FrameCmd_SetTimConf,
               (char)(timConf.tim1Psc>>8),(char)(timConf.tim1Psc&0xFF),
               (char)(timConf.tim1Arr>>8),(char)(timConf.tim1Arr&0xFF),
               (char)(timConf.tim8Psc>>8),(char)(timConf.tim8Psc&0xFF),
               (char)(timConf.tim8Arr>>8),(char)(timConf.tim8Arr&0xFF)};
    port->write(buf,sizeof(buf));
}

//将配置保存到配置文件中（ini格式）
void MainWindow::saveToFile(const QString &filename)
{
    QSettings settings(filename,QSettings::IniFormat);//创建QSettings
    settings.setIniCodec("GBK");

    for(uint8_t index=0;index<MaxCanMotorNum;index++)//写入CAN电机配置
    {
        settings.beginGroup(QString("CAN-MOT%1").arg(index+1));//每个电机占有一个配置组
        //写入基础数据
        settings.setValue("Enabled",canMotors[index].enabled);
        settings.setValue("NoteName",canMotors[index].noteTxt);
        settings.setValue("Type",canMotors[index].type);
        settings.setValue("ID",canMotors[index].id);
        //写入单级PID参数
        QByteArray singPidMemArray((const char*)&canMotors[index].singPID,sizeof(SingPID));//（直接将内存转为QByteArray储存）
        settings.setValue("SingPid",singPidMemArray);
        //写入串级PID参数
        QByteArray casPidMemArray((const char*)&canMotors[index].casPID,sizeof(CasPID));
        settings.setValue("CasPid",casPidMemArray);
        //写入控制窗口数据
        CtrlDialogConf ctrlDialogConf;
        ctrlDialogs[index]->getConf(ctrlDialogConf);//获取控制窗口需要保存的数据
        QByteArray ctrlDialogConfMemArray((const char*)&ctrlDialogConf,sizeof(CtrlDialogConf));
        settings.setValue("CtrlDialog",ctrlDialogConfMemArray);

        settings.endGroup();
    }

    for(uint8_t index=0;index<MaxPwmMotorNum;index++)//写入PWM电机配置
    {
        settings.beginGroup(QString("PWM-MOT%1").arg(index+1));//每个电机占有一个配置组
        //写入基础数据
        settings.setValue("Enabled",pwmMotors[index].enabled);
        settings.setValue("NoteName",pwmMotors[index].noteTxt);
        //写入PWM控制窗口数据
        PwmCtrlDialogConf pwmDialogConf;
        pwmDialogs[index]->getConf(pwmDialogConf);//获取控制窗口需要保存的数据
        QByteArray pwmDialogConfMemArray((const char*)&pwmDialogConf,sizeof(PwmCtrlDialogConf));
        settings.setValue("PwmDialog",pwmDialogConfMemArray);

        settings.endGroup();
    }

    settings.beginGroup("Global");//写入全局配置

    QByteArray timConfMemArray((const char*)&timConf,sizeof(TimConf));//写入TIM配置数据
    settings.setValue("TimConf",timConfMemArray);

    settings.endGroup();
}

//从INI文件中读取配置信息并更新界面
void MainWindow::loadFromFile(const QString &filename)
{
    QSettings settings(filename,QSettings::IniFormat);//创建QSettings对象
    settings.setIniCodec("GBK");

    for(uint8_t index=0;index<MaxCanMotorNum;index++)//读取CAN电机配置
    {
        settings.beginGroup(QString("CAN-MOT%1").arg(index+1));//切换到对应配置组
        //读取基础配置
        canMotors[index].enabled=settings.value("Enabled").toBool();
        canMotors[index].noteTxt=settings.value("NoteName").toString();
        canMotors[index].type=(CanMotorType)settings.value("Type").toInt();
        canMotors[index].id=settings.value("ID").toInt();
        //读取单级PID参数（使用QByteArray读出后内存拷贝到结构体中）
        QByteArray singPidMemArray=settings.value("SingPid").toByteArray();
        memcpy(&canMotors[index].singPID,singPidMemArray.data(),sizeof(SingPID));
        //读取串级PID参数
        QByteArray casPidMemArray=settings.value("CasPid").toByteArray();
        memcpy(&canMotors[index].casPID,casPidMemArray.data(),sizeof(CasPID));
        //读取控制窗口数据
        CtrlDialogConf ctrlDialogConf;
        QByteArray ctrlDialogConfMemArray=settings.value("CtrlDialog").toByteArray();
        memcpy(&ctrlDialogConf,ctrlDialogConfMemArray.data(),sizeof(CtrlDialogConf));
        ctrlDialogs[index]->setConf(ctrlDialogConf);//将读取到的数据更新到控制窗口

        settings.endGroup();
    }

    for(uint8_t index=0;index<MaxPwmMotorNum;index++)//读取PWM电机配置
    {
        settings.beginGroup(QString("PWM-MOT%1").arg(index+1));//切换到对应配置组
        //读取基础配置
        pwmMotors[index].enabled=settings.value("Enabled").toBool();
        pwmMotors[index].noteTxt=settings.value("NoteName").toString();
        //读取控制窗口数据
        PwmCtrlDialogConf pwmDialogConf;
        QByteArray pwmDialogConfMemArray=settings.value("PwmDialog").toByteArray();
        memcpy(&pwmDialogConf,pwmDialogConfMemArray.data(),sizeof(PwmCtrlDialogConf));
        pwmDialogs[index]->setConf(pwmDialogConf);//将读取到的数据更新到控制窗口

        settings.endGroup();
    }

    settings.beginGroup("Global");//读取全局配置

    QByteArray timConfMemArray=settings.value("TimConf").toByteArray();//读取TIM配置数据
    memcpy(&timConf,timConfMemArray.data(),sizeof(TimConf));
    timConfDialog->setUiData(timConf);

    settings.endGroup();

    updateUi();//更新主窗口UI界面
}

//用各电机当前数据更新主窗口UI界面
void MainWindow::updateUi()
{
    for(uint8_t index=0;index<MaxCanMotorNum;index++)//更新CAN电机相关控件
    {
        findChild<QCheckBox*>(QString("chb_can%1_mot%2_en").arg(index/4+1).arg(index%4+1))->setChecked(canMotors[index].enabled);
        findChild<QLineEdit*>(QString("et_can%1_mot%2_name").arg(index/4+1).arg(index%4+1))->setText(canMotors[index].noteTxt);
        findChild<QComboBox*>(QString("cb_can%1_mot%2_type").arg(index/4+1).arg(index%4+1))->setCurrentText(typeToName(canMotors[index].type));
        findChild<QSpinBox*>(QString("sb_can%1_mot%2_id").arg(index/4+1).arg(index%4+1))->setValue(canMotors[index].id);
    }

    for(uint8_t index=0;index<MaxPwmMotorNum;index++)//更新PWM电机相关控件
    {
        findChild<QCheckBox*>(QString("chb_pwm_mot%1_en").arg(index+1))->setChecked(pwmMotors[index].enabled);
        findChild<QLineEdit*>(QString("et_pwm_mot%1_name").arg(index+1))->setText(pwmMotors[index].noteTxt);
    }
}

//暂停程序执行一段事件（但仍可以处理应用事件）
void MainWindow::sleep(uint32_t ms)
{
    QTime untilTime=QTime::currentTime().addMSecs(ms);
    while(QTime::currentTime()<untilTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);//在等待的过程中处理应用事件
}

//设置界面样式
void MainWindow::setAppStyle(AppStyle style)
{
    if(appStyle==style)
        return;

    appStyle=style;//更新style记录值

    ui->action_default_style->setChecked(false);//先将各样式对应的菜单项取消勾选，下方根据传入参数勾选
    ui->action_light_blue_style->setChecked(false);
    ui->action_dark_blue_style->setChecked(false);

    if(style==Style_Default)//切换为默认样式
    {
        qApp->setStyleSheet("");
        ui->action_default_style->setChecked(true);
    }
    else if(style==Style_LightBlue)//切换为LightBlue样式
    {
        QFile qss(":/qss/light-blue.qss");//设置qss文件
        if(qss.open(QIODevice::ReadOnly))
            qApp->setStyleSheet(QLatin1String(qss.readAll()));
        ui->action_light_blue_style->setChecked(true);
    }
    else if(style==Style_DarkBlue)//切换为DarkBlue样式
    {
        QFile qss(":/qss/dark-blue.qss");//设置qss文件
        if(qss.open(QIODevice::ReadOnly))
            qApp->setStyleSheet(QLatin1String(qss.readAll()));
        ui->action_dark_blue_style->setChecked(true);
    }

    //将style设置写入配置文件
    QSettings styleSettings(QApplication::applicationDirPath()+"/style-conf.ini",QSettings::IniFormat);
    styleSettings.setValue("style",appStyle);
}

//从配置文件读取界面样式设置并更新界面
void MainWindow::loadAppStyleConf()
{
    QFileInfo styleIniFile(QApplication::applicationDirPath()+"/style-conf.ini");
    if(styleIniFile.exists())
    {
        QSettings styleSettings(QApplication::applicationDirPath()+"/style-conf.ini",QSettings::IniFormat);
        setAppStyle((AppStyle)styleSettings.value("style").toInt());
    }
    else
        setAppStyle(Style_Default);
}

//串口刷新按钮点击时调用
void MainWindow::on_bt_refresh_clicked()
{
    ui->cb_com->clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        ui->cb_com->addItem(info.portName());
    }
}

//开关串口按钮点击时调用
void MainWindow::on_bt_switch_clicked()
{
    if(port->isOpen())
    {
        port->clear();
        port->close();
        ui->bt_switch->setText("打开串口");
    }
    else
    {
        if(ui->cb_com->currentText().isEmpty())
        {
            QMessageBox::information(NULL,"错误","请选择串口号");
            return;
        }
        port->setPortName(ui->cb_com->currentText());
        if(!port->open(QIODevice::ReadWrite))
        {
            QMessageBox::information(NULL,"错误","串口打开失败");
            return;
        }
        port->setBaudRate(QSerialPort::Baud115200,QSerialPort::AllDirections);
        port->setDataBits(QSerialPort::Data8);
        port->setFlowControl(QSerialPort::NoFlowControl);
        port->setParity(QSerialPort::NoParity);
        port->setStopBits(QSerialPort::OneStop);
        //connect(port,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));
        ui->bt_switch->setText("关闭串口");
    }
}

//定时器配置按钮按下时调用
void MainWindow::on_bt_tim_conf_clicked()
{
    //打开定时器配置窗口
    timConfDialog->show();
    timConfDialog->activateWindow();
}

//导入配置菜单项点击时调用
void MainWindow::on_action_import_triggered()
{
    //弹出文件选择框
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(QStringLiteral("选中文件"));
    fileDialog->setDirectory(".");
    fileDialog->setNameFilter(tr("Ini File (*.ini)"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    fileDialog->setViewMode(QFileDialog::Detail);
    if(fileDialog->exec())//等待用户选择文件
    {
        QStringList fileList=fileDialog->selectedFiles();
        QString fileName=fileList.at(0);
        loadFromFile(fileName);//从文件中读取配置
    }
}

//导出配置菜单项点击时调用
void MainWindow::on_action_export_triggered()
{
    //弹出文件选择框
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(QStringLiteral("选中文件"));
    fileDialog->setDirectory(".");
    fileDialog->setNameFilter(tr("Ini File (*.ini)"));
    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setViewMode(QFileDialog::Detail);
    if(fileDialog->exec())//等待用户选择文件
    {
        QStringList fileList=fileDialog->selectedFiles();
        QString fileName=fileList.at(0);
        if(!fileName.endsWith(".ini"))//若用户输入的文件名不以ini结尾则加上后缀
            fileName.append(".ini");
        QFileInfo file(fileName);//判断是否已经存在文件，若存在则弹出覆盖警告框
        if(file.exists())
        {
            QMessageBox messageBox(QMessageBox::NoIcon,"警告", "文件已存在，是否覆盖？",QMessageBox::Yes | QMessageBox::No, NULL);
            if(messageBox.exec()!=QMessageBox::Yes)
                return;
        }
        saveToFile(fileName);//将配置写入文件
    }
}

//按钮同步到下位机点击时调用
void MainWindow::on_action_sync_triggered()
{
    if(!port->isOpen())
        return;

    for(uint8_t index=0;index<MaxCanMotorNum;index++)//完整发送一次CAN电机数据
    {
        sendType((CanMotorIndex)index);
        sleep(5);//等待一下避免粘包
        sendId((CanMotorIndex)index);
        sleep(5);
        sendPidParam((CanMotorIndex)index);
        sleep(5);
        CtrlDialogConf ctrlConf;
        ctrlDialogs[index]->getConf(ctrlConf);
        onCtrlModeChanged(index,ctrlConf.mode);
        sleep(5);
    }

    sendTimConf();//发送定时器配置数据
}

//端口说明菜单项点击时调用
void MainWindow::on_action_ports_help_triggered()
{
    //调用系统默认方式打开图片
    QDesktopServices::openUrl(QUrl("file:///"+QCoreApplication::applicationDirPath()+"/img/BoardC-Ports.png",QUrl::TolerantMode));
}

//PWM说明菜单项点击时调用
void MainWindow::on_action_pwm_help_triggered()
{
    //调用系统默认方式打开图片
    QDesktopServices::openUrl(QUrl("file:///"+QCoreApplication::applicationDirPath()+"/img/BoardC-Pwm.png",QUrl::TolerantMode));
}

//CAN说明菜单项点击时调用
void MainWindow::on_action_can_help_triggered()
{
    //调用系统默认方式打开图片
    QDesktopServices::openUrl(QUrl("file:///"+QCoreApplication::applicationDirPath()+"/img/BoardC-Can.png",QUrl::TolerantMode));
}

//关于菜单项点击时调用
void MainWindow::on_action_about_triggered()
{
    //显示关于对话框
    aboutDialog->show();
    aboutDialog->activateWindow();
}

//使用说明菜单项点击时调用
void MainWindow::on_action_help_triggered()
{
    //调用系统默认方式打开HTML文档
    QDesktopServices::openUrl(QUrl("file:///"+QCoreApplication::applicationDirPath()+"/help.html",QUrl::TolerantMode));
}

//默认界面样式菜单项点击时调用
void MainWindow::on_action_default_style_triggered()
{
    setAppStyle(Style_Default);
}

//LightBlue(明亮风格)样式菜单项点击时调用
void MainWindow::on_action_light_blue_style_triggered()
{
    setAppStyle(Style_LightBlue);
}

//DarkBlue(黑暗风格)样式菜单项点击时调用
void MainWindow::on_action_dark_blue_style_triggered()
{
    setAppStyle(Style_DarkBlue);
}
