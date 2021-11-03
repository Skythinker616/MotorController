#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowTitle("关于 MotorController");
    //将文本框中的参数用版本号和更新日期替换
    ui->txt_info->setText(ui->txt_info->text().arg(QApplication::applicationVersion()).arg(APP_BUILD_DATE));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
