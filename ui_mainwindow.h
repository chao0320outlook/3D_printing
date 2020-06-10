/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <my_opengl.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *newaction;
    QAction *open_act;
    QAction *save_act;
    QWidget *centralWidget;
    My_Opengl *openGLWidget;
    QPushButton *pushButton;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_4;
    QDoubleSpinBox *doubleSpinBox_X;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_Y;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinBox_H;
    QDoubleSpinBox *doubleSpinBox_Zoom;
    QLabel *label_2;
    QPushButton *pushButton_2;
    QPushButton *pushButton_4;
    QPushButton *pushButton_3;
    QPushButton *pushButton_6;
    QCheckBox *checkBox;
    QGroupBox *groupBox_2;
    QPushButton *pushButton_up;
    QPushButton *pushButton_right;
    QPushButton *pushButton_front;
    QPushButton *pushButton_down;
    QPushButton *pushButton_left;
    QPushButton *pushButton_back;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QSlider *horizontalSlider;
    QSlider *horizontalSlider_2;
    QSlider *horizontalSlider_3;
    QPushButton *pushButton_5;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menu_2;
    QMenu *menu_3;
    QMenu *menu_4;
    QMenu *menu_5;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1231, 608);
        newaction = new QAction(MainWindow);
        newaction->setObjectName(QString::fromUtf8("newaction"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/new_1.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        newaction->setIcon(icon);
        open_act = new QAction(MainWindow);
        open_act->setObjectName(QString::fromUtf8("open_act"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/open_1.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        open_act->setIcon(icon1);
        save_act = new QAction(MainWindow);
        save_act->setObjectName(QString::fromUtf8("save_act"));
        save_act->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/save1.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        save_act->setIcon(icon2);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        openGLWidget = new My_Opengl(centralWidget);
        openGLWidget->setObjectName(QString::fromUtf8("openGLWidget"));
        openGLWidget->setEnabled(true);
        openGLWidget->setGeometry(QRect(266, 4, 951, 541));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(20, 20, 75, 61));
        QFont font;
        font.setPointSize(20);
        pushButton->setFont(font);
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(100, 10, 169, 138));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        QFont font1;
        font1.setPointSize(12);
        label_4->setFont(font1);

        gridLayout->addWidget(label_4, 3, 0, 1, 2);

        doubleSpinBox_X = new QDoubleSpinBox(groupBox);
        doubleSpinBox_X->setObjectName(QString::fromUtf8("doubleSpinBox_X"));
        QFont font2;
        font2.setPointSize(11);
        doubleSpinBox_X->setFont(font2);
        doubleSpinBox_X->setMinimum(1.000000000000000);
        doubleSpinBox_X->setMaximum(9999.000000000000000);

        gridLayout->addWidget(doubleSpinBox_X, 0, 1, 1, 2);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font3;
        font3.setPointSize(12);
        font3.setBold(false);
        font3.setWeight(50);
        label->setFont(font3);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        doubleSpinBox_Y = new QDoubleSpinBox(groupBox);
        doubleSpinBox_Y->setObjectName(QString::fromUtf8("doubleSpinBox_Y"));
        doubleSpinBox_Y->setFont(font2);
        doubleSpinBox_Y->setMinimum(1.000000000000000);
        doubleSpinBox_Y->setMaximum(9999.000000000000000);

        gridLayout->addWidget(doubleSpinBox_Y, 1, 1, 1, 2);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font3);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        doubleSpinBox_H = new QDoubleSpinBox(groupBox);
        doubleSpinBox_H->setObjectName(QString::fromUtf8("doubleSpinBox_H"));
        doubleSpinBox_H->setFont(font2);
        doubleSpinBox_H->setDecimals(2);
        doubleSpinBox_H->setMinimum(1.000000000000000);
        doubleSpinBox_H->setMaximum(9999.000000000000000);

        gridLayout->addWidget(doubleSpinBox_H, 2, 1, 1, 2);

        doubleSpinBox_Zoom = new QDoubleSpinBox(groupBox);
        doubleSpinBox_Zoom->setObjectName(QString::fromUtf8("doubleSpinBox_Zoom"));
        doubleSpinBox_Zoom->setFont(font2);
        doubleSpinBox_Zoom->setDecimals(3);
        doubleSpinBox_Zoom->setMinimum(0.050000000000000);
        doubleSpinBox_Zoom->setMaximum(99.000000000000000);
        doubleSpinBox_Zoom->setSingleStep(0.100000000000000);

        gridLayout->addWidget(doubleSpinBox_Zoom, 3, 2, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font3);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(20, 90, 75, 61));
        pushButton_2->setFont(font);
        pushButton_4 = new QPushButton(centralWidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(20, 200, 101, 41));
        QFont font4;
        font4.setPointSize(14);
        pushButton_4->setFont(font4);
        pushButton_3 = new QPushButton(centralWidget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(30, 290, 71, 31));
        pushButton_6 = new QPushButton(centralWidget);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));
        pushButton_6->setGeometry(QRect(140, 200, 101, 41));
        pushButton_6->setFont(font4);
        checkBox = new QCheckBox(centralWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(30, 250, 181, 31));
        checkBox->setFont(font4);
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(100, 40, 271, 111));
        pushButton_up = new QPushButton(groupBox_2);
        pushButton_up->setObjectName(QString::fromUtf8("pushButton_up"));
        pushButton_up->setGeometry(QRect(220, 40, 31, 23));
        pushButton_right = new QPushButton(groupBox_2);
        pushButton_right->setObjectName(QString::fromUtf8("pushButton_right"));
        pushButton_right->setGeometry(QRect(180, 40, 31, 23));
        pushButton_front = new QPushButton(groupBox_2);
        pushButton_front->setObjectName(QString::fromUtf8("pushButton_front"));
        pushButton_front->setGeometry(QRect(180, 10, 31, 23));
        pushButton_down = new QPushButton(groupBox_2);
        pushButton_down->setObjectName(QString::fromUtf8("pushButton_down"));
        pushButton_down->setGeometry(QRect(220, 70, 31, 23));
        pushButton_left = new QPushButton(groupBox_2);
        pushButton_left->setObjectName(QString::fromUtf8("pushButton_left"));
        pushButton_left->setGeometry(QRect(180, 70, 31, 23));
        pushButton_back = new QPushButton(groupBox_2);
        pushButton_back->setObjectName(QString::fromUtf8("pushButton_back"));
        pushButton_back->setGeometry(QRect(220, 10, 31, 23));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 10, 16, 21));
        QFont font5;
        font5.setPointSize(14);
        font5.setBold(false);
        font5.setWeight(50);
        label_5->setFont(font5);
        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 70, 21, 21));
        label_6->setFont(font5);
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 40, 16, 21));
        label_7->setFont(font5);
        horizontalSlider = new QSlider(groupBox_2);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(40, 10, 131, 22));
        horizontalSlider->setMaximum(359);
        horizontalSlider->setSingleStep(1);
        horizontalSlider->setOrientation(Qt::Horizontal);
        horizontalSlider->setInvertedAppearance(false);
        horizontalSlider->setInvertedControls(false);
        horizontalSlider->setTickPosition(QSlider::NoTicks);
        horizontalSlider_2 = new QSlider(groupBox_2);
        horizontalSlider_2->setObjectName(QString::fromUtf8("horizontalSlider_2"));
        horizontalSlider_2->setGeometry(QRect(40, 40, 131, 22));
        horizontalSlider_2->setMaximum(359);
        horizontalSlider_2->setOrientation(Qt::Horizontal);
        horizontalSlider_3 = new QSlider(groupBox_2);
        horizontalSlider_3->setObjectName(QString::fromUtf8("horizontalSlider_3"));
        horizontalSlider_3->setGeometry(QRect(40, 70, 131, 22));
        horizontalSlider_3->setMaximum(359);
        horizontalSlider_3->setOrientation(Qt::Horizontal);
        pushButton_5 = new QPushButton(centralWidget);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));
        pushButton_5->setGeometry(QRect(120, 290, 49, 25));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1231, 26));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        menu_2 = new QMenu(menuBar);
        menu_2->setObjectName(QString::fromUtf8("menu_2"));
        menu_3 = new QMenu(menuBar);
        menu_3->setObjectName(QString::fromUtf8("menu_3"));
        menu_4 = new QMenu(menuBar);
        menu_4->setObjectName(QString::fromUtf8("menu_4"));
        menu_5 = new QMenu(menuBar);
        menu_5->setObjectName(QString::fromUtf8("menu_5"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menu_2->menuAction());
        menuBar->addAction(menu_3->menuAction());
        menuBar->addAction(menu_4->menuAction());
        menuBar->addAction(menu_5->menuAction());
        menu->addAction(newaction);
        menu->addAction(open_act);
        menu->addAction(save_act);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        newaction->setText(QCoreApplication::translate("MainWindow", "\346\226\260\345\273\272", nullptr));
#if QT_CONFIG(shortcut)
        newaction->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        open_act->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200", nullptr));
#if QT_CONFIG(shortcut)
        open_act->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        save_act->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
#if QT_CONFIG(shortcut)
        save_act->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        pushButton->setText(QCoreApplication::translate("MainWindow", "\345\260\272\345\257\270", nullptr));
        groupBox->setTitle(QString());
        label_4->setText(QCoreApplication::translate("MainWindow", "\347\274\251\346\224\276", nullptr));
        doubleSpinBox_X->setSuffix(QCoreApplication::translate("MainWindow", " mm", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\351\225\277", nullptr));
        doubleSpinBox_Y->setSuffix(QCoreApplication::translate("MainWindow", " mm", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "\351\253\230", nullptr));
        doubleSpinBox_H->setSuffix(QCoreApplication::translate("MainWindow", " mm", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\345\256\275", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "\350\247\222\345\272\246", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "\346\243\200\346\265\213", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "\351\207\215\347\275\256\344\275\215\347\275\256", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "\346\224\257\346\222\221\347\224\237\346\210\220", nullptr));
        checkBox->setText(QCoreApplication::translate("MainWindow", "\346\230\276\347\244\272\351\234\200\350\246\201\346\224\257\346\222\221\344\275\215\347\275\256", nullptr));
        groupBox_2->setTitle(QString());
        pushButton_up->setText(QString());
        pushButton_right->setText(QString());
        pushButton_front->setText(QString());
        pushButton_down->setText(QString());
        pushButton_left->setText(QString());
        pushButton_back->setText(QString());
        label_5->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Z", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Y", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        menu_2->setTitle(QCoreApplication::translate("MainWindow", "\347\274\226\350\276\221", nullptr));
        menu_3->setTitle(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
        menu_4->setTitle(QCoreApplication::translate("MainWindow", "\345\217\202\346\225\260", nullptr));
        menu_5->setTitle(QCoreApplication::translate("MainWindow", "\345\270\256\345\212\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
