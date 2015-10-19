/********************************************************************************
** Form generated from reading UI file 'AutoMeasurementDialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTOMEASUREMENTDIALOG_H
#define UI_AUTOMEASUREMENTDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AutoMeasurementDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QTabWidget *m_tabWidget;
    QWidget *tab;
    QFrame *frame_2;
    QSpinBox *m_xGridSpinBox;
    QSpinBox *m_yGridSpinBox;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QLabel *m_detName;
    QLabel *m_extName;
    QLabel *m_matName;
    QWidget *layoutWidget1;
    QGridLayout *gridLayout_2;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QSpinBox *m_maxMatchSpinBox;
    QLabel *label_5;
    QLabel *label_6;
    QFrame *frame_3;
    QLabel *label_4;
    QFrame *frame_4;
    QCheckBox *m_cbUseGrid;
    QLabel *m_detName_2;
    QLabel *m_detName_3;
    QFrame *frame_5;
    QFrame *frame_6;
    QComboBox *m_matComboBox;
    QComboBox *m_extComboBox;
    QComboBox *m_detComboBox;
    QWidget *m_pointPositionTab;
    QPushButton *m_execMeasButton;
    QTextBrowser *m_measResultsBrowser;
    QPushButton *m_acceptMeasButton;
    QFrame *frame;
    QHBoxLayout *m_botPanel;
    QPushButton *m_resetMeasButton;
    QSpacerItem *horizontalSpacer_1;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *m_dismissButton;

    void setupUi(QDialog *AutoMeasurementDialog)
    {
        if (AutoMeasurementDialog->objectName().isEmpty())
            AutoMeasurementDialog->setObjectName(QString::fromUtf8("AutoMeasurementDialog"));
        AutoMeasurementDialog->resize(769, 359);
        verticalLayout_3 = new QVBoxLayout(AutoMeasurementDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_tabWidget = new QTabWidget(AutoMeasurementDialog);
        m_tabWidget->setObjectName(QString::fromUtf8("m_tabWidget"));
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        m_tabWidget->setAutoFillBackground(false);
        m_tabWidget->setTabPosition(QTabWidget::North);
        m_tabWidget->setTabShape(QTabWidget::Rounded);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        frame_2 = new QFrame(tab);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setGeometry(QRect(260, 73, 461, 101));
        frame_2->setFrameShape(QFrame::Box);
        frame_2->setFrameShadow(QFrame::Raised);
        m_xGridSpinBox = new QSpinBox(tab);
        m_xGridSpinBox->setObjectName(QString::fromUtf8("m_xGridSpinBox"));
        m_xGridSpinBox->setGeometry(QRect(20, 140, 53, 24));
        m_xGridSpinBox->setLayoutDirection(Qt::RightToLeft);
        m_xGridSpinBox->setAutoFillBackground(false);
        m_xGridSpinBox->setMinimum(1);
        m_xGridSpinBox->setMaximum(100);
        m_yGridSpinBox = new QSpinBox(tab);
        m_yGridSpinBox->setObjectName(QString::fromUtf8("m_yGridSpinBox"));
        m_yGridSpinBox->setGeometry(QRect(73, 140, 53, 24));
        m_yGridSpinBox->setLayoutDirection(Qt::LeftToRight);
        m_yGridSpinBox->setMinimum(1);
        m_yGridSpinBox->setMaximum(10);
        layoutWidget = new QWidget(tab);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(542, 83, 171, 81));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(4, 2, 0, 2);
        m_detName = new QLabel(layoutWidget);
        m_detName->setObjectName(QString::fromUtf8("m_detName"));
        QFont font;
        font.setPointSize(14);
        m_detName->setFont(font);

        gridLayout->addWidget(m_detName, 0, 0, 1, 1);

        m_extName = new QLabel(layoutWidget);
        m_extName->setObjectName(QString::fromUtf8("m_extName"));
        m_extName->setEnabled(true);
        QFont font1;
        font1.setPointSize(14);
        font1.setBold(false);
        font1.setItalic(false);
        font1.setWeight(50);
        m_extName->setFont(font1);
        m_extName->setAcceptDrops(false);
        m_extName->setFrameShape(QFrame::NoFrame);

        gridLayout->addWidget(m_extName, 1, 0, 1, 1);

        m_matName = new QLabel(layoutWidget);
        m_matName->setObjectName(QString::fromUtf8("m_matName"));
        m_matName->setEnabled(true);
        m_matName->setFont(font1);
        m_matName->setAcceptDrops(false);
        m_matName->setFrameShape(QFrame::NoFrame);

        gridLayout->addWidget(m_matName, 2, 0, 1, 1);

        layoutWidget1 = new QWidget(tab);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(475, 83, 64, 81));
        gridLayout_2 = new QGridLayout(layoutWidget1);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 2, 0, 2);
        label = new QLabel(layoutWidget1);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font2;
        font2.setPointSize(14);
        font2.setBold(true);
        font2.setItalic(false);
        font2.setWeight(75);
        label->setFont(font2);

        gridLayout_2->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(layoutWidget1);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font2);

        gridLayout_2->addWidget(label_2, 1, 0, 1, 1);

        label_3 = new QLabel(layoutWidget1);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font2);

        gridLayout_2->addWidget(label_3, 2, 0, 1, 1);

        m_maxMatchSpinBox = new QSpinBox(tab);
        m_maxMatchSpinBox->setObjectName(QString::fromUtf8("m_maxMatchSpinBox"));
        m_maxMatchSpinBox->setGeometry(QRect(20, 85, 51, 24));
        m_maxMatchSpinBox->setMinimum(1);
        m_maxMatchSpinBox->setMaximum(100);
        label_5 = new QLabel(tab);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(80, 90, 141, 16));
        label_6 = new QLabel(tab);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(133, 145, 71, 16));
        frame_3 = new QFrame(tab);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setGeometry(QRect(162, 10, 411, 31));
        frame_3->setAutoFillBackground(false);
        frame_3->setFrameShape(QFrame::Panel);
        frame_3->setFrameShadow(QFrame::Raised);
        frame_3->setLineWidth(2);
        label_4 = new QLabel(frame_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 0, 391, 31));
        QFont font3;
        font3.setPointSize(15);
        font3.setBold(true);
        font3.setItalic(true);
        font3.setUnderline(false);
        font3.setWeight(75);
        label_4->setFont(font3);
        label_4->setAlignment(Qt::AlignCenter);
        frame_4 = new QFrame(tab);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setGeometry(QRect(10, 73, 221, 101));
        frame_4->setFrameShape(QFrame::Box);
        frame_4->setFrameShadow(QFrame::Raised);
        m_cbUseGrid = new QCheckBox(frame_4);
        m_cbUseGrid->setObjectName(QString::fromUtf8("m_cbUseGrid"));
        m_cbUseGrid->setGeometry(QRect(11, 42, 188, 18));
        m_detName_2 = new QLabel(tab);
        m_detName_2->setObjectName(QString::fromUtf8("m_detName_2"));
        m_detName_2->setGeometry(QRect(420, 55, 141, 20));
        m_detName_2->setFont(font);
        m_detName_3 = new QLabel(tab);
        m_detName_3->setObjectName(QString::fromUtf8("m_detName_3"));
        m_detName_3->setGeometry(QRect(45, 55, 151, 20));
        m_detName_3->setFont(font);
        frame_5 = new QFrame(tab);
        frame_5->setObjectName(QString::fromUtf8("frame_5"));
        frame_5->setGeometry(QRect(10, 54, 221, 21));
        frame_5->setFrameShape(QFrame::Box);
        frame_5->setFrameShadow(QFrame::Raised);
        frame_6 = new QFrame(tab);
        frame_6->setObjectName(QString::fromUtf8("frame_6"));
        frame_6->setGeometry(QRect(260, 54, 461, 21));
        frame_6->setFrameShape(QFrame::Box);
        frame_6->setFrameShadow(QFrame::Raised);
        m_matComboBox = new QComboBox(tab);
        m_matComboBox->setObjectName(QString::fromUtf8("m_matComboBox"));
        m_matComboBox->setGeometry(QRect(270, 140, 191, 26));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_matComboBox->sizePolicy().hasHeightForWidth());
        m_matComboBox->setSizePolicy(sizePolicy);
        m_matComboBox->setLayoutDirection(Qt::LeftToRight);
        m_extComboBox = new QComboBox(tab);
        m_extComboBox->setObjectName(QString::fromUtf8("m_extComboBox"));
        m_extComboBox->setGeometry(QRect(270, 110, 191, 26));
        sizePolicy.setHeightForWidth(m_extComboBox->sizePolicy().hasHeightForWidth());
        m_extComboBox->setSizePolicy(sizePolicy);
        m_detComboBox = new QComboBox(tab);
        m_detComboBox->setObjectName(QString::fromUtf8("m_detComboBox"));
        m_detComboBox->setGeometry(QRect(270, 80, 191, 26));
        m_tabWidget->addTab(tab, QString());
        frame_2->raise();
        frame_5->raise();
        frame_3->raise();
        frame_4->raise();
        layoutWidget->raise();
        layoutWidget->raise();
        m_xGridSpinBox->raise();
        m_yGridSpinBox->raise();
        m_maxMatchSpinBox->raise();
        label_5->raise();
        label_6->raise();
        m_detName_2->raise();
        m_detName_3->raise();
        frame_6->raise();
        m_matComboBox->raise();
        m_extComboBox->raise();
        m_detComboBox->raise();
        m_pointPositionTab = new QWidget();
        m_pointPositionTab->setObjectName(QString::fromUtf8("m_pointPositionTab"));
        m_execMeasButton = new QPushButton(m_pointPositionTab);
        m_execMeasButton->setObjectName(QString::fromUtf8("m_execMeasButton"));
        m_execMeasButton->setGeometry(QRect(533, 10, 114, 32));
        QFont font4;
        font4.setBold(true);
        font4.setWeight(75);
        m_execMeasButton->setFont(font4);
        m_measResultsBrowser = new QTextBrowser(m_pointPositionTab);
        m_measResultsBrowser->setObjectName(QString::fromUtf8("m_measResultsBrowser"));
        m_measResultsBrowser->setGeometry(QRect(20, 0, 450, 242));
        m_measResultsBrowser->setFrameShadow(QFrame::Sunken);
        m_acceptMeasButton = new QPushButton(m_pointPositionTab);
        m_acceptMeasButton->setObjectName(QString::fromUtf8("m_acceptMeasButton"));
        m_acceptMeasButton->setGeometry(QRect(533, 50, 114, 32));
        m_acceptMeasButton->setFont(font4);
        frame = new QFrame(m_pointPositionTab);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(530, 6, 120, 80));
        frame->setFrameShape(QFrame::Box);
        frame->setFrameShadow(QFrame::Raised);
        m_tabWidget->addTab(m_pointPositionTab, QString());
        frame->raise();
        m_execMeasButton->raise();
        m_measResultsBrowser->raise();
        m_acceptMeasButton->raise();

        horizontalLayout->addWidget(m_tabWidget);


        verticalLayout_3->addLayout(horizontalLayout);

        m_botPanel = new QHBoxLayout();
        m_botPanel->setObjectName(QString::fromUtf8("m_botPanel"));
        m_botPanel->setSizeConstraint(QLayout::SetNoConstraint);
        m_resetMeasButton = new QPushButton(AutoMeasurementDialog);
        m_resetMeasButton->setObjectName(QString::fromUtf8("m_resetMeasButton"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_resetMeasButton->sizePolicy().hasHeightForWidth());
        m_resetMeasButton->setSizePolicy(sizePolicy1);

        m_botPanel->addWidget(m_resetMeasButton);

        horizontalSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_botPanel->addItem(horizontalSpacer_1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        m_botPanel->addItem(horizontalSpacer_2);

        m_dismissButton = new QPushButton(AutoMeasurementDialog);
        m_dismissButton->setObjectName(QString::fromUtf8("m_dismissButton"));
        sizePolicy1.setHeightForWidth(m_dismissButton->sizePolicy().hasHeightForWidth());
        m_dismissButton->setSizePolicy(sizePolicy1);

        m_botPanel->addWidget(m_dismissButton);


        verticalLayout_3->addLayout(m_botPanel);


        retranslateUi(AutoMeasurementDialog);

        m_tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AutoMeasurementDialog);
    } // setupUi

    void retranslateUi(QDialog *AutoMeasurementDialog)
    {
        AutoMeasurementDialog->setWindowTitle(QApplication::translate("AutoMeasurementDialog", "Auto Measurement", 0, QApplication::UnicodeUTF8));
        m_detName->setText(QApplication::translate("AutoMeasurementDialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        m_extName->setText(QApplication::translate("AutoMeasurementDialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        m_matName->setText(QApplication::translate("AutoMeasurementDialog", "TextLabel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("AutoMeasurementDialog", "Detector", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AutoMeasurementDialog", "Extractor", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("AutoMeasurementDialog", "Matcher", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("AutoMeasurementDialog", "Max Matches / Patch", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("AutoMeasurementDialog", "Grid Layout", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("AutoMeasurementDialog", "Left button/drag to select bounding box for collection", 0, QApplication::UnicodeUTF8));
        m_cbUseGrid->setText(QApplication::translate("AutoMeasurementDialog", "Use Grid Adapted Detection", 0, QApplication::UnicodeUTF8));
        m_detName_2->setText(QApplication::translate("AutoMeasurementDialog", "OpenCV Configuration", 0, QApplication::UnicodeUTF8));
        m_detName_3->setText(QApplication::translate("AutoMeasurementDialog", "Collection Configuration", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(tab), QApplication::translate("AutoMeasurementDialog", "Configuration", 0, QApplication::UnicodeUTF8));
        m_execMeasButton->setText(QApplication::translate("AutoMeasurementDialog", "Execute", 0, QApplication::UnicodeUTF8));
        m_acceptMeasButton->setText(QApplication::translate("AutoMeasurementDialog", "Accept", 0, QApplication::UnicodeUTF8));
        m_tabWidget->setTabText(m_tabWidget->indexOf(m_pointPositionTab), QApplication::translate("AutoMeasurementDialog", "Collection", 0, QApplication::UnicodeUTF8));
        m_resetMeasButton->setText(QApplication::translate("AutoMeasurementDialog", "Reset", 0, QApplication::UnicodeUTF8));
        m_dismissButton->setText(QApplication::translate("AutoMeasurementDialog", "Dismiss", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AutoMeasurementDialog: public Ui_AutoMeasurementDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTOMEASUREMENTDIALOG_H
