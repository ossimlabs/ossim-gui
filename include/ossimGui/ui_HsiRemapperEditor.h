/********************************************************************************
** Form generated from reading UI file 'HsiRemapperEditor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HSIREMAPPEREDITOR_H
#define UI_HSIREMAPPEREDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_HsiRemapperEditor
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QRadioButton *m_redButton;
    QRadioButton *m_yellowButton;
    QRadioButton *m_greenButton;
    QRadioButton *m_cyanButton;
    QRadioButton *m_blueButton;
    QRadioButton *m_magentaButton;
    QRadioButton *m_allButton;
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QLabel *m_hueOffsetLabel;
    QSlider *m_hueOffsetSlider;
    QLabel *m_hueOffsetValueLabel;
    QHBoxLayout *hboxLayout1;
    QLabel *m_hueLowRangeLabel;
    QSlider *m_hueLowRangeSlider;
    QLabel *m_hueLowRangeValueLabel;
    QHBoxLayout *hboxLayout2;
    QLabel *m_hueHighRangeLabel;
    QSlider *m_hueHighRangeSlider;
    QLabel *m_hueHighRangeValueLabel;
    QHBoxLayout *hboxLayout3;
    QLabel *m_hueBlendRangeLabel;
    QSlider *m_hueBlendRangeSlider;
    QLabel *m_hueBlendRangeValueLabel;
    QHBoxLayout *hboxLayout4;
    QLabel *m_saturationOffsetLabel;
    QSlider *m_saturationOffsetSlider;
    QLabel *m_saturationOffsetValueLabel;
    QHBoxLayout *hboxLayout5;
    QLabel *m_intensityOffsetLabel;
    QSlider *m_intensityOffsetSlider;
    QLabel *m_intensityOffsetValueLabel;
    QHBoxLayout *hboxLayout6;
    QLabel *m_lowIntensityClipLabel;
    QSlider *m_lowIntensityClipSlider;
    QLabel *m_lowIntensityClipValueLabel;
    QHBoxLayout *hboxLayout7;
    QLabel *m_highIntensityClipLabel;
    QSlider *m_highIntensityClipSlider;
    QLabel *m_highIntensityClipValueLabel;
    QHBoxLayout *hboxLayout8;
    QLabel *m_whiteObjectClipLabel;
    QSlider *m_whiteObjectClipSlider;
    QLabel *m_whiteObjectClipValueLabel;
    QFrame *line1;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *m_enableButton;
    QPushButton *m_resetGroupButton;
    QPushButton *m_resetAllButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    void setupUi(QDialog *HsiRemapperEditor)
    {
        if (HsiRemapperEditor->objectName().isEmpty())
            HsiRemapperEditor->setObjectName(QString::fromUtf8("HsiRemapperEditor"));
        HsiRemapperEditor->resize(507, 429);
        verticalLayout = new QVBoxLayout(HsiRemapperEditor);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_redButton = new QRadioButton(HsiRemapperEditor);
        m_redButton->setObjectName(QString::fromUtf8("m_redButton"));

        horizontalLayout->addWidget(m_redButton);

        m_yellowButton = new QRadioButton(HsiRemapperEditor);
        m_yellowButton->setObjectName(QString::fromUtf8("m_yellowButton"));

        horizontalLayout->addWidget(m_yellowButton);

        m_greenButton = new QRadioButton(HsiRemapperEditor);
        m_greenButton->setObjectName(QString::fromUtf8("m_greenButton"));

        horizontalLayout->addWidget(m_greenButton);

        m_cyanButton = new QRadioButton(HsiRemapperEditor);
        m_cyanButton->setObjectName(QString::fromUtf8("m_cyanButton"));

        horizontalLayout->addWidget(m_cyanButton);

        m_blueButton = new QRadioButton(HsiRemapperEditor);
        m_blueButton->setObjectName(QString::fromUtf8("m_blueButton"));

        horizontalLayout->addWidget(m_blueButton);

        m_magentaButton = new QRadioButton(HsiRemapperEditor);
        m_magentaButton->setObjectName(QString::fromUtf8("m_magentaButton"));

        horizontalLayout->addWidget(m_magentaButton);

        m_allButton = new QRadioButton(HsiRemapperEditor);
        m_allButton->setObjectName(QString::fromUtf8("m_allButton"));
        m_allButton->setChecked(true);

        horizontalLayout->addWidget(m_allButton);


        verticalLayout->addLayout(horizontalLayout);

        vboxLayout = new QVBoxLayout();
        vboxLayout->setSpacing(8);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        m_hueOffsetLabel = new QLabel(HsiRemapperEditor);
        m_hueOffsetLabel->setObjectName(QString::fromUtf8("m_hueOffsetLabel"));
        m_hueOffsetLabel->setMinimumSize(QSize(110, 0));
        m_hueOffsetLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_hueOffsetLabel->setWordWrap(false);

        hboxLayout->addWidget(m_hueOffsetLabel);

        m_hueOffsetSlider = new QSlider(HsiRemapperEditor);
        m_hueOffsetSlider->setObjectName(QString::fromUtf8("m_hueOffsetSlider"));
        m_hueOffsetSlider->setMinimumSize(QSize(290, 0));
        m_hueOffsetSlider->setMinimum(-180);
        m_hueOffsetSlider->setMaximum(180);
        m_hueOffsetSlider->setPageStep(1);
        m_hueOffsetSlider->setTracking(false);
        m_hueOffsetSlider->setOrientation(Qt::Horizontal);
        m_hueOffsetSlider->setTickPosition(QSlider::TicksBelow);
        m_hueOffsetSlider->setTickInterval(60);

        hboxLayout->addWidget(m_hueOffsetSlider);

        m_hueOffsetValueLabel = new QLabel(HsiRemapperEditor);
        m_hueOffsetValueLabel->setObjectName(QString::fromUtf8("m_hueOffsetValueLabel"));
        m_hueOffsetValueLabel->setMinimumSize(QSize(40, 0));
        m_hueOffsetValueLabel->setAlignment(Qt::AlignVCenter);
        m_hueOffsetValueLabel->setWordWrap(false);

        hboxLayout->addWidget(m_hueOffsetValueLabel);


        vboxLayout->addLayout(hboxLayout);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
        m_hueLowRangeLabel = new QLabel(HsiRemapperEditor);
        m_hueLowRangeLabel->setObjectName(QString::fromUtf8("m_hueLowRangeLabel"));
        m_hueLowRangeLabel->setMinimumSize(QSize(110, 0));
        m_hueLowRangeLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_hueLowRangeLabel->setWordWrap(false);

        hboxLayout1->addWidget(m_hueLowRangeLabel);

        m_hueLowRangeSlider = new QSlider(HsiRemapperEditor);
        m_hueLowRangeSlider->setObjectName(QString::fromUtf8("m_hueLowRangeSlider"));
        m_hueLowRangeSlider->setMinimumSize(QSize(290, 0));
        m_hueLowRangeSlider->setMinimum(-30);
        m_hueLowRangeSlider->setMaximum(30);
        m_hueLowRangeSlider->setPageStep(1);
        m_hueLowRangeSlider->setTracking(false);
        m_hueLowRangeSlider->setOrientation(Qt::Horizontal);
        m_hueLowRangeSlider->setTickPosition(QSlider::TicksBelow);
        m_hueLowRangeSlider->setTickInterval(10);

        hboxLayout1->addWidget(m_hueLowRangeSlider);

        m_hueLowRangeValueLabel = new QLabel(HsiRemapperEditor);
        m_hueLowRangeValueLabel->setObjectName(QString::fromUtf8("m_hueLowRangeValueLabel"));
        m_hueLowRangeValueLabel->setMinimumSize(QSize(40, 0));
        m_hueLowRangeValueLabel->setAlignment(Qt::AlignVCenter);
        m_hueLowRangeValueLabel->setWordWrap(false);

        hboxLayout1->addWidget(m_hueLowRangeValueLabel);


        vboxLayout->addLayout(hboxLayout1);

        hboxLayout2 = new QHBoxLayout();
        hboxLayout2->setSpacing(6);
        hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
        m_hueHighRangeLabel = new QLabel(HsiRemapperEditor);
        m_hueHighRangeLabel->setObjectName(QString::fromUtf8("m_hueHighRangeLabel"));
        m_hueHighRangeLabel->setMinimumSize(QSize(110, 0));
        m_hueHighRangeLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_hueHighRangeLabel->setWordWrap(false);

        hboxLayout2->addWidget(m_hueHighRangeLabel);

        m_hueHighRangeSlider = new QSlider(HsiRemapperEditor);
        m_hueHighRangeSlider->setObjectName(QString::fromUtf8("m_hueHighRangeSlider"));
        m_hueHighRangeSlider->setMinimumSize(QSize(290, 0));
        m_hueHighRangeSlider->setMinimum(-30);
        m_hueHighRangeSlider->setMaximum(30);
        m_hueHighRangeSlider->setPageStep(1);
        m_hueHighRangeSlider->setTracking(false);
        m_hueHighRangeSlider->setOrientation(Qt::Horizontal);
        m_hueHighRangeSlider->setTickPosition(QSlider::TicksBelow);
        m_hueHighRangeSlider->setTickInterval(10);

        hboxLayout2->addWidget(m_hueHighRangeSlider);

        m_hueHighRangeValueLabel = new QLabel(HsiRemapperEditor);
        m_hueHighRangeValueLabel->setObjectName(QString::fromUtf8("m_hueHighRangeValueLabel"));
        m_hueHighRangeValueLabel->setMinimumSize(QSize(40, 0));
        m_hueHighRangeValueLabel->setAlignment(Qt::AlignVCenter);
        m_hueHighRangeValueLabel->setWordWrap(false);

        hboxLayout2->addWidget(m_hueHighRangeValueLabel);


        vboxLayout->addLayout(hboxLayout2);

        hboxLayout3 = new QHBoxLayout();
        hboxLayout3->setSpacing(6);
        hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
        m_hueBlendRangeLabel = new QLabel(HsiRemapperEditor);
        m_hueBlendRangeLabel->setObjectName(QString::fromUtf8("m_hueBlendRangeLabel"));
        m_hueBlendRangeLabel->setMinimumSize(QSize(110, 0));
        m_hueBlendRangeLabel->setWordWrap(false);

        hboxLayout3->addWidget(m_hueBlendRangeLabel);

        m_hueBlendRangeSlider = new QSlider(HsiRemapperEditor);
        m_hueBlendRangeSlider->setObjectName(QString::fromUtf8("m_hueBlendRangeSlider"));
        m_hueBlendRangeSlider->setMinimumSize(QSize(290, 0));
        m_hueBlendRangeSlider->setMaximum(30);
        m_hueBlendRangeSlider->setPageStep(1);
        m_hueBlendRangeSlider->setValue(15);
        m_hueBlendRangeSlider->setTracking(false);
        m_hueBlendRangeSlider->setOrientation(Qt::Horizontal);
        m_hueBlendRangeSlider->setTickPosition(QSlider::TicksBelow);
        m_hueBlendRangeSlider->setTickInterval(5);

        hboxLayout3->addWidget(m_hueBlendRangeSlider);

        m_hueBlendRangeValueLabel = new QLabel(HsiRemapperEditor);
        m_hueBlendRangeValueLabel->setObjectName(QString::fromUtf8("m_hueBlendRangeValueLabel"));
        m_hueBlendRangeValueLabel->setMinimumSize(QSize(40, 0));
        m_hueBlendRangeValueLabel->setWordWrap(false);

        hboxLayout3->addWidget(m_hueBlendRangeValueLabel);


        vboxLayout->addLayout(hboxLayout3);

        hboxLayout4 = new QHBoxLayout();
        hboxLayout4->setSpacing(6);
        hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
        m_saturationOffsetLabel = new QLabel(HsiRemapperEditor);
        m_saturationOffsetLabel->setObjectName(QString::fromUtf8("m_saturationOffsetLabel"));
        m_saturationOffsetLabel->setMinimumSize(QSize(110, 0));
        m_saturationOffsetLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_saturationOffsetLabel->setWordWrap(false);

        hboxLayout4->addWidget(m_saturationOffsetLabel);

        m_saturationOffsetSlider = new QSlider(HsiRemapperEditor);
        m_saturationOffsetSlider->setObjectName(QString::fromUtf8("m_saturationOffsetSlider"));
        m_saturationOffsetSlider->setMinimumSize(QSize(290, 0));
        m_saturationOffsetSlider->setMinimum(-200);
        m_saturationOffsetSlider->setMaximum(200);
        m_saturationOffsetSlider->setPageStep(1);
        m_saturationOffsetSlider->setTracking(false);
        m_saturationOffsetSlider->setOrientation(Qt::Horizontal);
        m_saturationOffsetSlider->setTickPosition(QSlider::TicksBelow);
        m_saturationOffsetSlider->setTickInterval(40);

        hboxLayout4->addWidget(m_saturationOffsetSlider);

        m_saturationOffsetValueLabel = new QLabel(HsiRemapperEditor);
        m_saturationOffsetValueLabel->setObjectName(QString::fromUtf8("m_saturationOffsetValueLabel"));
        m_saturationOffsetValueLabel->setMinimumSize(QSize(40, 0));
        m_saturationOffsetValueLabel->setWordWrap(false);

        hboxLayout4->addWidget(m_saturationOffsetValueLabel);


        vboxLayout->addLayout(hboxLayout4);

        hboxLayout5 = new QHBoxLayout();
        hboxLayout5->setSpacing(6);
        hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
        m_intensityOffsetLabel = new QLabel(HsiRemapperEditor);
        m_intensityOffsetLabel->setObjectName(QString::fromUtf8("m_intensityOffsetLabel"));
        m_intensityOffsetLabel->setMinimumSize(QSize(110, 0));
        m_intensityOffsetLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_intensityOffsetLabel->setWordWrap(false);

        hboxLayout5->addWidget(m_intensityOffsetLabel);

        m_intensityOffsetSlider = new QSlider(HsiRemapperEditor);
        m_intensityOffsetSlider->setObjectName(QString::fromUtf8("m_intensityOffsetSlider"));
        m_intensityOffsetSlider->setMinimumSize(QSize(290, 0));
        m_intensityOffsetSlider->setMinimum(-100);
        m_intensityOffsetSlider->setMaximum(100);
        m_intensityOffsetSlider->setPageStep(1);
        m_intensityOffsetSlider->setTracking(false);
        m_intensityOffsetSlider->setOrientation(Qt::Horizontal);
        m_intensityOffsetSlider->setTickPosition(QSlider::TicksBelow);
        m_intensityOffsetSlider->setTickInterval(40);

        hboxLayout5->addWidget(m_intensityOffsetSlider);

        m_intensityOffsetValueLabel = new QLabel(HsiRemapperEditor);
        m_intensityOffsetValueLabel->setObjectName(QString::fromUtf8("m_intensityOffsetValueLabel"));
        m_intensityOffsetValueLabel->setMinimumSize(QSize(40, 0));
        m_intensityOffsetValueLabel->setWordWrap(false);

        hboxLayout5->addWidget(m_intensityOffsetValueLabel);


        vboxLayout->addLayout(hboxLayout5);

        hboxLayout6 = new QHBoxLayout();
        hboxLayout6->setSpacing(6);
        hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
        m_lowIntensityClipLabel = new QLabel(HsiRemapperEditor);
        m_lowIntensityClipLabel->setObjectName(QString::fromUtf8("m_lowIntensityClipLabel"));
        m_lowIntensityClipLabel->setMinimumSize(QSize(110, 0));
        m_lowIntensityClipLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_lowIntensityClipLabel->setWordWrap(false);

        hboxLayout6->addWidget(m_lowIntensityClipLabel);

        m_lowIntensityClipSlider = new QSlider(HsiRemapperEditor);
        m_lowIntensityClipSlider->setObjectName(QString::fromUtf8("m_lowIntensityClipSlider"));
        m_lowIntensityClipSlider->setMinimumSize(QSize(290, 0));
        m_lowIntensityClipSlider->setMaximum(400);
        m_lowIntensityClipSlider->setPageStep(1);
        m_lowIntensityClipSlider->setTracking(false);
        m_lowIntensityClipSlider->setOrientation(Qt::Horizontal);
        m_lowIntensityClipSlider->setTickPosition(QSlider::TicksBelow);
        m_lowIntensityClipSlider->setTickInterval(40);

        hboxLayout6->addWidget(m_lowIntensityClipSlider);

        m_lowIntensityClipValueLabel = new QLabel(HsiRemapperEditor);
        m_lowIntensityClipValueLabel->setObjectName(QString::fromUtf8("m_lowIntensityClipValueLabel"));
        m_lowIntensityClipValueLabel->setMinimumSize(QSize(40, 0));
        m_lowIntensityClipValueLabel->setWordWrap(false);

        hboxLayout6->addWidget(m_lowIntensityClipValueLabel);


        vboxLayout->addLayout(hboxLayout6);

        hboxLayout7 = new QHBoxLayout();
        hboxLayout7->setSpacing(6);
        hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
        m_highIntensityClipLabel = new QLabel(HsiRemapperEditor);
        m_highIntensityClipLabel->setObjectName(QString::fromUtf8("m_highIntensityClipLabel"));
        m_highIntensityClipLabel->setMinimumSize(QSize(110, 0));
        m_highIntensityClipLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        m_highIntensityClipLabel->setWordWrap(false);

        hboxLayout7->addWidget(m_highIntensityClipLabel);

        m_highIntensityClipSlider = new QSlider(HsiRemapperEditor);
        m_highIntensityClipSlider->setObjectName(QString::fromUtf8("m_highIntensityClipSlider"));
        m_highIntensityClipSlider->setMinimumSize(QSize(290, 0));
        m_highIntensityClipSlider->setMaximum(400);
        m_highIntensityClipSlider->setPageStep(1);
        m_highIntensityClipSlider->setValue(400);
        m_highIntensityClipSlider->setTracking(false);
        m_highIntensityClipSlider->setOrientation(Qt::Horizontal);
        m_highIntensityClipSlider->setTickPosition(QSlider::TicksBelow);
        m_highIntensityClipSlider->setTickInterval(40);

        hboxLayout7->addWidget(m_highIntensityClipSlider);

        m_highIntensityClipValueLabel = new QLabel(HsiRemapperEditor);
        m_highIntensityClipValueLabel->setObjectName(QString::fromUtf8("m_highIntensityClipValueLabel"));
        m_highIntensityClipValueLabel->setMinimumSize(QSize(40, 0));
        m_highIntensityClipValueLabel->setWordWrap(false);

        hboxLayout7->addWidget(m_highIntensityClipValueLabel);


        vboxLayout->addLayout(hboxLayout7);

        hboxLayout8 = new QHBoxLayout();
        hboxLayout8->setSpacing(6);
        hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
        m_whiteObjectClipLabel = new QLabel(HsiRemapperEditor);
        m_whiteObjectClipLabel->setObjectName(QString::fromUtf8("m_whiteObjectClipLabel"));
        m_whiteObjectClipLabel->setMinimumSize(QSize(110, 0));
        m_whiteObjectClipLabel->setWordWrap(false);

        hboxLayout8->addWidget(m_whiteObjectClipLabel);

        m_whiteObjectClipSlider = new QSlider(HsiRemapperEditor);
        m_whiteObjectClipSlider->setObjectName(QString::fromUtf8("m_whiteObjectClipSlider"));
        m_whiteObjectClipSlider->setMinimumSize(QSize(290, 0));
        m_whiteObjectClipSlider->setMinimum(320);
        m_whiteObjectClipSlider->setMaximum(400);
        m_whiteObjectClipSlider->setPageStep(1);
        m_whiteObjectClipSlider->setValue(400);
        m_whiteObjectClipSlider->setTracking(false);
        m_whiteObjectClipSlider->setOrientation(Qt::Horizontal);
        m_whiteObjectClipSlider->setTickPosition(QSlider::TicksBelow);
        m_whiteObjectClipSlider->setTickInterval(8);

        hboxLayout8->addWidget(m_whiteObjectClipSlider);

        m_whiteObjectClipValueLabel = new QLabel(HsiRemapperEditor);
        m_whiteObjectClipValueLabel->setObjectName(QString::fromUtf8("m_whiteObjectClipValueLabel"));
        m_whiteObjectClipValueLabel->setMinimumSize(QSize(40, 0));
        m_whiteObjectClipValueLabel->setWordWrap(false);

        hboxLayout8->addWidget(m_whiteObjectClipValueLabel);


        vboxLayout->addLayout(hboxLayout8);


        verticalLayout->addLayout(vboxLayout);

        line1 = new QFrame(HsiRemapperEditor);
        line1->setObjectName(QString::fromUtf8("line1"));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        m_enableButton = new QCheckBox(HsiRemapperEditor);
        m_enableButton->setObjectName(QString::fromUtf8("m_enableButton"));

        horizontalLayout_2->addWidget(m_enableButton);

        m_resetGroupButton = new QPushButton(HsiRemapperEditor);
        m_resetGroupButton->setObjectName(QString::fromUtf8("m_resetGroupButton"));
        m_resetGroupButton->setAutoDefault(false);

        horizontalLayout_2->addWidget(m_resetGroupButton);

        m_resetAllButton = new QPushButton(HsiRemapperEditor);
        m_resetAllButton->setObjectName(QString::fromUtf8("m_resetAllButton"));
        m_resetAllButton->setAutoDefault(false);

        horizontalLayout_2->addWidget(m_resetAllButton);

        m_okButton = new QPushButton(HsiRemapperEditor);
        m_okButton->setObjectName(QString::fromUtf8("m_okButton"));
        m_okButton->setAutoDefault(false);

        horizontalLayout_2->addWidget(m_okButton);

        m_cancelButton = new QPushButton(HsiRemapperEditor);
        m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));
        m_cancelButton->setAutoDefault(false);

        horizontalLayout_2->addWidget(m_cancelButton);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(HsiRemapperEditor);

        QMetaObject::connectSlotsByName(HsiRemapperEditor);
    } // setupUi

    void retranslateUi(QDialog *HsiRemapperEditor)
    {
        HsiRemapperEditor->setWindowTitle(QApplication::translate("HsiRemapperEditor", "HSI Remapper Property Editor", 0, QApplication::UnicodeUTF8));
        m_redButton->setText(QApplication::translate("HsiRemapperEditor", "red", 0, QApplication::UnicodeUTF8));
        m_yellowButton->setText(QApplication::translate("HsiRemapperEditor", "yellow", 0, QApplication::UnicodeUTF8));
        m_greenButton->setText(QApplication::translate("HsiRemapperEditor", "green", 0, QApplication::UnicodeUTF8));
        m_cyanButton->setText(QApplication::translate("HsiRemapperEditor", "cyan", 0, QApplication::UnicodeUTF8));
        m_blueButton->setText(QApplication::translate("HsiRemapperEditor", "blue", 0, QApplication::UnicodeUTF8));
        m_magentaButton->setText(QApplication::translate("HsiRemapperEditor", "magenta", 0, QApplication::UnicodeUTF8));
        m_allButton->setText(QApplication::translate("HsiRemapperEditor", "all", 0, QApplication::UnicodeUTF8));
        m_hueOffsetLabel->setText(QApplication::translate("HsiRemapperEditor", "hue offset:", 0, QApplication::UnicodeUTF8));
        m_hueOffsetValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_hueLowRangeLabel->setText(QApplication::translate("HsiRemapperEditor", "hue low range:", 0, QApplication::UnicodeUTF8));
        m_hueLowRangeValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_hueHighRangeLabel->setText(QApplication::translate("HsiRemapperEditor", "hue high range:", 0, QApplication::UnicodeUTF8));
        m_hueHighRangeValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_hueBlendRangeLabel->setText(QApplication::translate("HsiRemapperEditor", "hue blend range:", 0, QApplication::UnicodeUTF8));
        m_hueBlendRangeValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_saturationOffsetLabel->setText(QApplication::translate("HsiRemapperEditor", "saturation offset:", 0, QApplication::UnicodeUTF8));
        m_saturationOffsetValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_intensityOffsetLabel->setText(QApplication::translate("HsiRemapperEditor", "intensity offset", 0, QApplication::UnicodeUTF8));
        m_intensityOffsetValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_lowIntensityClipLabel->setText(QApplication::translate("HsiRemapperEditor", "low intensity clip:", 0, QApplication::UnicodeUTF8));
        m_lowIntensityClipValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_highIntensityClipLabel->setText(QApplication::translate("HsiRemapperEditor", "high intensity clip:", 0, QApplication::UnicodeUTF8));
        m_highIntensityClipValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_whiteObjectClipLabel->setText(QApplication::translate("HsiRemapperEditor", "white object clip", 0, QApplication::UnicodeUTF8));
        m_whiteObjectClipValueLabel->setText(QApplication::translate("HsiRemapperEditor", "0", 0, QApplication::UnicodeUTF8));
        m_enableButton->setText(QApplication::translate("HsiRemapperEditor", "enable", 0, QApplication::UnicodeUTF8));
        m_resetGroupButton->setText(QApplication::translate("HsiRemapperEditor", "reset group", 0, QApplication::UnicodeUTF8));
        m_resetAllButton->setText(QApplication::translate("HsiRemapperEditor", "reset all", 0, QApplication::UnicodeUTF8));
        m_okButton->setText(QApplication::translate("HsiRemapperEditor", "Ok", 0, QApplication::UnicodeUTF8));
        m_cancelButton->setText(QApplication::translate("HsiRemapperEditor", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HsiRemapperEditor: public Ui_HsiRemapperEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HSIREMAPPEREDITOR_H
