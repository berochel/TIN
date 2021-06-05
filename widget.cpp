#include "widget.h"
#include "ui_widget.h"

#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    user_id = -1;
    ui->setupUi(this);
    QStringList list=(QStringList()<<"login"<<"create user"<<"create group"<<"join group"<<"leave group"<<"list requests"<<"accept request"<<"list groups"<<"list files"<<"upload file"<<"download file"<<"show downloads"<<"stop share");
    ui->commandComboBox->addItems(list);
}

Widget::~Widget()
{
    delete ui;
}

std::string Widget::getAttribute1()
{
    return ui->attr1plainTextEdit->toPlainText().toStdString();
}

std::string Widget::getAttribute2()
{
    return ui->attr2plainTextEdit->toPlainText().toStdString();
}

std::string Widget::getAttribute3()
{
    return ui->attr3plainTextEdit->toPlainText().toStdString();
}

void Widget::setAttribute1(QString s)
{
    ui->attr1Label->setText(s);
}

void Widget::setAttribute2(QString s)
{
    ui->attr2Label->setText(s);
}

void Widget::setAttribute3(QString s)
{
    ui->attr3Label->setText(s);
}

void Widget::clearTextFields()
{
    ui->attr1plainTextEdit->clear();
    ui->attr2plainTextEdit->clear();
    ui->attr3plainTextEdit->clear();
}

void Widget::setUserID(int id)
{
    user_id = id;
    if (user_id >= 0)
    {
        ui->logOutButton->setEnabled(true);
        ui->loginIdLabel->setText(QString::number(user_id));
        QMessageBox msgBox;
        msgBox.setText("You have been logged in.");
        msgBox.exec();
    }
    else
    {
        ui->logOutButton->setDisabled(true);
        ui->loginIdLabel->setText(QString("Not logged in"));
        QMessageBox msgBox;
        msgBox.setText("You have been logged out.");
        msgBox.exec();
    }
}

int Widget::getUserID()
{
    return user_id;
}

std::string Widget::getCommand()
{
    return command;
    command = "";
}

void Widget::on_exitButton_clicked()
{
    setUserID(-1);
    exit(0);
}

void Widget::on_logOutButton_clicked()
{
    command = "19";
    clearTextFields();
    setUserID(-1);
}

void Widget::on_sendCommandButton_clicked()
{
    std::string text = ui->commandComboBox->currentText().toStdString();
    std::string attr1 = getAttribute1();
    std::string attr2 = getAttribute2();
    std::string attr3 = getAttribute3();
    command = "";
    if (text == "create user"){
        command = "10 " + attr1 + " " + attr2;
    } else if (text == "login"){
        command = "11 " + attr1 + " " + attr2;
        setUserID(10);
    }else if (text == "create group"){
        command = "20 " + attr1;
    } else if (text == "join group"){
        command = "21 " + attr1;
    } else if (text == "leave group"){
        command = "29 " + attr1;
    } else if (text == "list requests"){
        command = "42 " + attr1;
    } else if (text == "accept request"){
        command = "43 " + attr1 + " " + attr2;
    } else if (text == "list groups"){
        command = "22";
    } else if (text == "list files"){
        command = "32 " + attr1;
    } else if (text == "upload file"){
        command = "30 " + attr1 + " " + attr2;
    } else if (text == "download file"){
        command = "31 " + attr1 + " " + attr2 + " " + attr3;
    } else if (text == "show downloads"){
        command = "35";
    } else if (text == "stop share"){
        command = "39 " + attr1 + " " + attr2;
    }
    clearTextFields();
}

void Widget::on_commandComboBox_currentTextChanged(const QString &arg1)
{
    std::string text = ui->commandComboBox->currentText().toStdString();
    if (text == "create user"){
        setAttribute1("user id");
        setAttribute2("password");
        setAttribute3("");
    } else if (text == "login"){
        setAttribute1("user id");
        setAttribute2("password");
        setAttribute3("");
    } else if (text == "create group"){
        setAttribute1("group id");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "join group"){
        setAttribute1("group id");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "leave group"){
        setAttribute1("group id");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "list requests"){
        setAttribute1("group id");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "accept request"){
        setAttribute1("group id");
        setAttribute2("user id");
        setAttribute3("");
    } else if (text == "list groups"){
        setAttribute1("");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "list files"){
        setAttribute1("group id");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "upload file"){
        setAttribute1("file path");
        setAttribute2("group id");
        setAttribute3("");
    } else if (text == "download file"){
        setAttribute1("group id");
        setAttribute2("file name");
        setAttribute3("destination path");
    } else if (text == "show downloads"){
        setAttribute1("");
        setAttribute2("");
        setAttribute3("");
    } else if (text == "stop share"){
        setAttribute1("group id");
        setAttribute2("file name");
        setAttribute3("");
    } else {
        setAttribute1("");
        setAttribute2("");
        setAttribute3("");
    }
}
