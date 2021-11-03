#ifndef CTRLDIALOG_H
#define CTRLDIALOG_H

#include <QDialog>
#include <qdebug.h>

namespace Ui {
class CtrlDialog;
}

//控制模式枚举
enum CtrlMode{
    Ctrl_Tor,
    Ctrl_Spd,
    Ctrl_SingAng,
    Ctrl_CasAng
};

//控制窗口配置数据，存放需要被导出保存的数据
class CtrlDialogConf{
public:
    CtrlMode mode;
    float minSpd,maxSpd;
    float minSingAng,maxSingAng;
    float minCasAng,maxCasAng,casSpd;
};

class CtrlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CtrlDialog(QWidget *parent = nullptr);
    ~CtrlDialog();
    void setWindowInfo(const QString &title,uint8_t id);
    void getConf(CtrlDialogConf &conf);
    void setConf(CtrlDialogConf &conf);

signals:
    void modeChanged(uint8_t id,CtrlMode mode);
    void torChanged(uint8_t id,float percent);
    void spdChanged(uint8_t id,float spd);
    void singAngChanged(uint8_t id,float ang);
    void casAngChanged(uint8_t id,float ang);
    void casSpdChanged(uint8_t id,float spd);
    void workStateChanged(uint8_t id,bool working);

private slots:
    void on_rb_tor_toggled(bool checked);
    void on_rb_spd_toggled(bool checked);
    void on_chb_tor_slider_toggled(bool checked);
    void on_chb_spd_slider_toggled(bool checked);
    void on_chb_sing_ang_slider_toggled(bool checked);
    void on_chb_cas_ang_slider_toggled(bool checked);
    void on_sl_tor_valueChanged(int value);
    void on_sb_tor_editingFinished();
    void on_sb_tor_valueChanged(double arg1);
    void on_sl_spd_valueChanged(int value);
    void on_sb_spd_valueChanged(double arg1);
    void on_sl_sing_ang_valueChanged(int value);
    void on_sb_sing_ang_valueChanged(double arg1);
    void on_sl_cas_ang_valueChanged(int value);
    void on_sb_cas_ang_valueChanged(double arg1);
    void on_sb_cas_spd_valueChanged(double arg1);
    void on_bt_work_clicked();
    void on_rb_sing_ang_toggled(bool checked);
    void on_rb_cas_ang_toggled(bool checked);

private:
    Ui::CtrlDialog *ui;
    uint8_t windowID;
    bool working;
};

#endif // CTRLDIALOG_H
