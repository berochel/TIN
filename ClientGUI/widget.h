#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    std::string getAttribute1();
    std::string getAttribute2();
    std::string getAttribute3();
    void setAttribute1(QString s);
    void setAttribute2(QString s);
    void setAttribute3(QString s);
    void clearTextFields();
    void setUserID(int id);
    int getUserID();
    std::string getCommand();

private slots:
    void on_exitButton_clicked();

    void on_logOutButton_clicked();

    void on_sendCommandButton_clicked();

    void on_commandComboBox_currentTextChanged(const QString &arg1);

private:
    Ui::Widget *ui;
    int user_id;
    std::string command;
};
#endif // WIDGET_H
