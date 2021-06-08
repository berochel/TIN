/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QHBoxLayout *horizontalLayout;
    QGridLayout *gridLayout;
    QComboBox *commandComboBox;
    QLabel *label_4;
    QPushButton *logOutButton;
    QPlainTextEdit *attr1plainTextEdit;
    QLabel *attr1Label;
    QPlainTextEdit *attr3plainTextEdit;
    QLabel *label;
    QPushButton *sendCommandButton;
    QLabel *attr3Label;
    QPlainTextEdit *attr2plainTextEdit;
    QPushButton *exitButton;
    QLabel *attr2Label;
    QLabel *loginIdLabel;
    QLabel *label_2;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(758, 385);
        horizontalLayout = new QHBoxLayout(Widget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        commandComboBox = new QComboBox(Widget);
        commandComboBox->setObjectName(QString::fromUtf8("commandComboBox"));
        commandComboBox->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(commandComboBox->sizePolicy().hasHeightForWidth());
        commandComboBox->setSizePolicy(sizePolicy);
        commandComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

        gridLayout->addWidget(commandComboBox, 3, 3, 1, 1);

        label_4 = new QLabel(Widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);
        QFont font;
        font.setPointSize(10);
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_4, 3, 2, 1, 1);

        logOutButton = new QPushButton(Widget);
        logOutButton->setObjectName(QString::fromUtf8("logOutButton"));
        logOutButton->setEnabled(false);
        logOutButton->setFont(font);

        gridLayout->addWidget(logOutButton, 6, 1, 1, 1);

        attr1plainTextEdit = new QPlainTextEdit(Widget);
        attr1plainTextEdit->setObjectName(QString::fromUtf8("attr1plainTextEdit"));
        attr1plainTextEdit->setEnabled(true);
        sizePolicy.setHeightForWidth(attr1plainTextEdit->sizePolicy().hasHeightForWidth());
        attr1plainTextEdit->setSizePolicy(sizePolicy);
        attr1plainTextEdit->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);

        gridLayout->addWidget(attr1plainTextEdit, 5, 1, 1, 1);

        attr1Label = new QLabel(Widget);
        attr1Label->setObjectName(QString::fromUtf8("attr1Label"));
        sizePolicy.setHeightForWidth(attr1Label->sizePolicy().hasHeightForWidth());
        attr1Label->setSizePolicy(sizePolicy);
        attr1Label->setFont(font);
        attr1Label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(attr1Label, 4, 1, 1, 1);

        attr3plainTextEdit = new QPlainTextEdit(Widget);
        attr3plainTextEdit->setObjectName(QString::fromUtf8("attr3plainTextEdit"));
        attr3plainTextEdit->setEnabled(true);
        sizePolicy.setHeightForWidth(attr3plainTextEdit->sizePolicy().hasHeightForWidth());
        attr3plainTextEdit->setSizePolicy(sizePolicy);
        attr3plainTextEdit->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        gridLayout->addWidget(attr3plainTextEdit, 5, 3, 1, 1);

        label = new QLabel(Widget);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font1;
        font1.setPointSize(25);
        label->setFont(font1);
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 0, 1, 1, 3);

        sendCommandButton = new QPushButton(Widget);
        sendCommandButton->setObjectName(QString::fromUtf8("sendCommandButton"));
        sendCommandButton->setEnabled(true);
        sendCommandButton->setFont(font);

        gridLayout->addWidget(sendCommandButton, 6, 2, 1, 1);

        attr3Label = new QLabel(Widget);
        attr3Label->setObjectName(QString::fromUtf8("attr3Label"));
        sizePolicy.setHeightForWidth(attr3Label->sizePolicy().hasHeightForWidth());
        attr3Label->setSizePolicy(sizePolicy);
        attr3Label->setFont(font);
        attr3Label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(attr3Label, 4, 3, 1, 1);

        attr2plainTextEdit = new QPlainTextEdit(Widget);
        attr2plainTextEdit->setObjectName(QString::fromUtf8("attr2plainTextEdit"));
        sizePolicy.setHeightForWidth(attr2plainTextEdit->sizePolicy().hasHeightForWidth());
        attr2plainTextEdit->setSizePolicy(sizePolicy);
        attr2plainTextEdit->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        gridLayout->addWidget(attr2plainTextEdit, 5, 2, 1, 1);

        exitButton = new QPushButton(Widget);
        exitButton->setObjectName(QString::fromUtf8("exitButton"));
        exitButton->setFont(font);

        gridLayout->addWidget(exitButton, 6, 3, 1, 1);

        attr2Label = new QLabel(Widget);
        attr2Label->setObjectName(QString::fromUtf8("attr2Label"));
        sizePolicy.setHeightForWidth(attr2Label->sizePolicy().hasHeightForWidth());
        attr2Label->setSizePolicy(sizePolicy);
        attr2Label->setFont(font);
        attr2Label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(attr2Label, 4, 2, 1, 1);

        loginIdLabel = new QLabel(Widget);
        loginIdLabel->setObjectName(QString::fromUtf8("loginIdLabel"));
        sizePolicy1.setHeightForWidth(loginIdLabel->sizePolicy().hasHeightForWidth());
        loginIdLabel->setSizePolicy(sizePolicy1);
        loginIdLabel->setFont(font);

        gridLayout->addWidget(loginIdLabel, 1, 3, 1, 1);

        label_2 = new QLabel(Widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 1, 2, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", nullptr));
        label_4->setText(QApplication::translate("Widget", "Select command", nullptr));
        logOutButton->setText(QApplication::translate("Widget", "Log out", nullptr));
        attr1Label->setText(QApplication::translate("Widget", "Login", nullptr));
        label->setText(QApplication::translate("Widget", "P2P Client", nullptr));
        sendCommandButton->setText(QApplication::translate("Widget", "Send Command", nullptr));
        attr3Label->setText(QApplication::translate("Widget", "Attribute3", nullptr));
        exitButton->setText(QApplication::translate("Widget", "Exit", nullptr));
        attr2Label->setText(QApplication::translate("Widget", "Password", nullptr));
        loginIdLabel->setText(QApplication::translate("Widget", "Not logged in", nullptr));
        label_2->setText(QApplication::translate("Widget", "Logged as:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
