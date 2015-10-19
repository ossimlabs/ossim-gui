/********************************************************************************
** Form generated from reading UI file 'HistogramRemapperEditor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HISTOGRAMREMAPPEREDITOR_H
#define UI_HISTOGRAMREMAPPEREDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include "ossimGui/HistogramWidget.h"

QT_BEGIN_NAMESPACE

class Ui_HistogramRemapperEditor
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_9;
    QVBoxLayout *verticalLayout;
    QComboBox *m_stretchModeComboBox;
    QComboBox *m_bandComboBox;
    ossimGui::HistogramWidget *m_histogramWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *m_lowClipPercentLineEdit;
    QLabel *m_lowClipPercentLineEditLabel;
    QSpacerItem *spacer11_2;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *m_highClipPercentLineEdit;
    QLabel *m_highClipPercentLineEditLabel;
    QSpacerItem *spacer12_2;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *m_lowClipValueLineEdit;
    QLabel *m_lowClipValueLineEditLabel;
    QSpacerItem *spacer13_2;
    QHBoxLayout *horizontalLayout_5;
    QLineEdit *m_highClipValueLineEdit;
    QLabel *m_highClipValueLineEditLabel;
    QSpacerItem *spacer14_2;
    QHBoxLayout *horizontalLayout_6;
    QLineEdit *m_midPointLineEdit;
    QLabel *m_midPointLineEditLabel;
    QSpacerItem *spacer15_2;
    QHBoxLayout *horizontalLayout_7;
    QLineEdit *m_outputMinValue;
    QLabel *m_outputMinValueLabel;
    QSpacerItem *spacer16_2;
    QHBoxLayout *horizontalLayout_8;
    QLineEdit *m_outputMaxValue;
    QLabel *textLabel10_2;
    QSpacerItem *spacer17_2;
    QHBoxLayout *horizontalLayout_10;
    QLabel *m_histogramFileLineEditLabel;
    QLineEdit *m_histogramFileLineEdit;
    QPushButton *m_histogramFilePushButton;
    QHBoxLayout *horizontalLayout;
    QCheckBox *m_enableButton;
    QPushButton *m_resetButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    void setupUi(QDialog *HistogramRemapperEditor)
    {
        if (HistogramRemapperEditor->objectName().isEmpty())
            HistogramRemapperEditor->setObjectName(QString::fromUtf8("HistogramRemapperEditor"));
        HistogramRemapperEditor->resize(498, 342);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(HistogramRemapperEditor->sizePolicy().hasHeightForWidth());
        HistogramRemapperEditor->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(HistogramRemapperEditor);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_stretchModeComboBox = new QComboBox(HistogramRemapperEditor);
        m_stretchModeComboBox->setObjectName(QString::fromUtf8("m_stretchModeComboBox"));
        m_stretchModeComboBox->setMinimumSize(QSize(130, 0));

        verticalLayout->addWidget(m_stretchModeComboBox);

        m_bandComboBox = new QComboBox(HistogramRemapperEditor);
        m_bandComboBox->setObjectName(QString::fromUtf8("m_bandComboBox"));
        m_bandComboBox->setMinimumSize(QSize(130, 0));

        verticalLayout->addWidget(m_bandComboBox);

        m_histogramWidget = new ossimGui::HistogramWidget(HistogramRemapperEditor);
        m_histogramWidget->setObjectName(QString::fromUtf8("m_histogramWidget"));
        m_histogramWidget->setMinimumSize(QSize(258, 138));
        m_histogramWidget->setMaximumSize(QSize(999999, 999999));
        m_histogramWidget->setFrameShape(QFrame::StyledPanel);
        m_histogramWidget->setFrameShadow(QFrame::Raised);

        verticalLayout->addWidget(m_histogramWidget);


        horizontalLayout_9->addLayout(verticalLayout);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        m_lowClipPercentLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_lowClipPercentLineEdit->setObjectName(QString::fromUtf8("m_lowClipPercentLineEdit"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_lowClipPercentLineEdit->sizePolicy().hasHeightForWidth());
        m_lowClipPercentLineEdit->setSizePolicy(sizePolicy1);
        m_lowClipPercentLineEdit->setMinimumSize(QSize(60, 0));
        m_lowClipPercentLineEdit->setMaximumSize(QSize(60, 32767));
        m_lowClipPercentLineEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(m_lowClipPercentLineEdit);

        m_lowClipPercentLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_lowClipPercentLineEditLabel->setObjectName(QString::fromUtf8("m_lowClipPercentLineEditLabel"));
        m_lowClipPercentLineEditLabel->setWordWrap(false);

        horizontalLayout_2->addWidget(m_lowClipPercentLineEditLabel);

        spacer11_2 = new QSpacerItem(13, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(spacer11_2);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        m_highClipPercentLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_highClipPercentLineEdit->setObjectName(QString::fromUtf8("m_highClipPercentLineEdit"));
        sizePolicy1.setHeightForWidth(m_highClipPercentLineEdit->sizePolicy().hasHeightForWidth());
        m_highClipPercentLineEdit->setSizePolicy(sizePolicy1);
        m_highClipPercentLineEdit->setMinimumSize(QSize(60, 0));
        m_highClipPercentLineEdit->setMaximumSize(QSize(60, 32767));
        m_highClipPercentLineEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(m_highClipPercentLineEdit);

        m_highClipPercentLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_highClipPercentLineEditLabel->setObjectName(QString::fromUtf8("m_highClipPercentLineEditLabel"));
        m_highClipPercentLineEditLabel->setWordWrap(false);

        horizontalLayout_3->addWidget(m_highClipPercentLineEditLabel);

        spacer12_2 = new QSpacerItem(13, 21, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(spacer12_2);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        m_lowClipValueLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_lowClipValueLineEdit->setObjectName(QString::fromUtf8("m_lowClipValueLineEdit"));
        m_lowClipValueLineEdit->setMinimumSize(QSize(60, 0));
        m_lowClipValueLineEdit->setMaximumSize(QSize(60, 32767));
        m_lowClipValueLineEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(m_lowClipValueLineEdit);

        m_lowClipValueLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_lowClipValueLineEditLabel->setObjectName(QString::fromUtf8("m_lowClipValueLineEditLabel"));
        m_lowClipValueLineEditLabel->setWordWrap(false);

        horizontalLayout_4->addWidget(m_lowClipValueLineEditLabel);

        spacer13_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(spacer13_2);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        m_highClipValueLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_highClipValueLineEdit->setObjectName(QString::fromUtf8("m_highClipValueLineEdit"));
        m_highClipValueLineEdit->setMinimumSize(QSize(60, 0));
        m_highClipValueLineEdit->setMaximumSize(QSize(60, 32767));
        m_highClipValueLineEdit->setReadOnly(true);

        horizontalLayout_5->addWidget(m_highClipValueLineEdit);

        m_highClipValueLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_highClipValueLineEditLabel->setObjectName(QString::fromUtf8("m_highClipValueLineEditLabel"));
        m_highClipValueLineEditLabel->setWordWrap(false);

        horizontalLayout_5->addWidget(m_highClipValueLineEditLabel);

        spacer14_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(spacer14_2);


        verticalLayout_2->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        m_midPointLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_midPointLineEdit->setObjectName(QString::fromUtf8("m_midPointLineEdit"));
        sizePolicy1.setHeightForWidth(m_midPointLineEdit->sizePolicy().hasHeightForWidth());
        m_midPointLineEdit->setSizePolicy(sizePolicy1);
        m_midPointLineEdit->setMinimumSize(QSize(60, 0));
        m_midPointLineEdit->setMaximumSize(QSize(60, 32767));
        m_midPointLineEdit->setReadOnly(true);

        horizontalLayout_6->addWidget(m_midPointLineEdit);

        m_midPointLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_midPointLineEditLabel->setObjectName(QString::fromUtf8("m_midPointLineEditLabel"));
        m_midPointLineEditLabel->setWordWrap(false);

        horizontalLayout_6->addWidget(m_midPointLineEditLabel);

        spacer15_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(spacer15_2);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        m_outputMinValue = new QLineEdit(HistogramRemapperEditor);
        m_outputMinValue->setObjectName(QString::fromUtf8("m_outputMinValue"));
        m_outputMinValue->setMinimumSize(QSize(60, 0));
        m_outputMinValue->setMaximumSize(QSize(60, 32767));
        m_outputMinValue->setReadOnly(true);

        horizontalLayout_7->addWidget(m_outputMinValue);

        m_outputMinValueLabel = new QLabel(HistogramRemapperEditor);
        m_outputMinValueLabel->setObjectName(QString::fromUtf8("m_outputMinValueLabel"));
        m_outputMinValueLabel->setWordWrap(false);

        horizontalLayout_7->addWidget(m_outputMinValueLabel);

        spacer16_2 = new QSpacerItem(13, 19, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(spacer16_2);


        verticalLayout_2->addLayout(horizontalLayout_7);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        m_outputMaxValue = new QLineEdit(HistogramRemapperEditor);
        m_outputMaxValue->setObjectName(QString::fromUtf8("m_outputMaxValue"));
        m_outputMaxValue->setMinimumSize(QSize(60, 0));
        m_outputMaxValue->setMaximumSize(QSize(60, 32767));
        m_outputMaxValue->setReadOnly(true);

        horizontalLayout_8->addWidget(m_outputMaxValue);

        textLabel10_2 = new QLabel(HistogramRemapperEditor);
        textLabel10_2->setObjectName(QString::fromUtf8("textLabel10_2"));
        textLabel10_2->setWordWrap(false);

        horizontalLayout_8->addWidget(textLabel10_2);

        spacer17_2 = new QSpacerItem(13, 19, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(spacer17_2);


        verticalLayout_2->addLayout(horizontalLayout_8);


        horizontalLayout_9->addLayout(verticalLayout_2);


        verticalLayout_3->addLayout(horizontalLayout_9);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        m_histogramFileLineEditLabel = new QLabel(HistogramRemapperEditor);
        m_histogramFileLineEditLabel->setObjectName(QString::fromUtf8("m_histogramFileLineEditLabel"));
        m_histogramFileLineEditLabel->setMinimumSize(QSize(90, 0));
        m_histogramFileLineEditLabel->setWordWrap(false);

        horizontalLayout_10->addWidget(m_histogramFileLineEditLabel);

        m_histogramFileLineEdit = new QLineEdit(HistogramRemapperEditor);
        m_histogramFileLineEdit->setObjectName(QString::fromUtf8("m_histogramFileLineEdit"));
        m_histogramFileLineEdit->setMinimumSize(QSize(220, 0));
        m_histogramFileLineEdit->setReadOnly(true);

        horizontalLayout_10->addWidget(m_histogramFileLineEdit);

        m_histogramFilePushButton = new QPushButton(HistogramRemapperEditor);
        m_histogramFilePushButton->setObjectName(QString::fromUtf8("m_histogramFilePushButton"));
        m_histogramFilePushButton->setAutoDefault(false);

        horizontalLayout_10->addWidget(m_histogramFilePushButton);


        verticalLayout_3->addLayout(horizontalLayout_10);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_enableButton = new QCheckBox(HistogramRemapperEditor);
        m_enableButton->setObjectName(QString::fromUtf8("m_enableButton"));

        horizontalLayout->addWidget(m_enableButton);

        m_resetButton = new QPushButton(HistogramRemapperEditor);
        m_resetButton->setObjectName(QString::fromUtf8("m_resetButton"));

        horizontalLayout->addWidget(m_resetButton);

        m_okButton = new QPushButton(HistogramRemapperEditor);
        m_okButton->setObjectName(QString::fromUtf8("m_okButton"));

        horizontalLayout->addWidget(m_okButton);

        m_cancelButton = new QPushButton(HistogramRemapperEditor);
        m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));

        horizontalLayout->addWidget(m_cancelButton);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(HistogramRemapperEditor);

        QMetaObject::connectSlotsByName(HistogramRemapperEditor);
    } // setupUi

    void retranslateUi(QDialog *HistogramRemapperEditor)
    {
        HistogramRemapperEditor->setWindowTitle(QApplication::translate("HistogramRemapperEditor", "Dialog", 0, QApplication::UnicodeUTF8));
        m_bandComboBox->clear();
        m_bandComboBox->insertItems(0, QStringList()
         << QApplication::translate("HistogramRemapperEditor", "master", 0, QApplication::UnicodeUTF8)
        );
        m_lowClipPercentLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "low clip percent", 0, QApplication::UnicodeUTF8));
        m_highClipPercentLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "high clip percent", 0, QApplication::UnicodeUTF8));
        m_lowClipValueLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "low clip value", 0, QApplication::UnicodeUTF8));
        m_highClipValueLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "high clip value", 0, QApplication::UnicodeUTF8));
        m_midPointLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "mid point", 0, QApplication::UnicodeUTF8));
        m_outputMinValueLabel->setText(QApplication::translate("HistogramRemapperEditor", "output min value", 0, QApplication::UnicodeUTF8));
        textLabel10_2->setText(QApplication::translate("HistogramRemapperEditor", "output max value", 0, QApplication::UnicodeUTF8));
        m_histogramFileLineEditLabel->setText(QApplication::translate("HistogramRemapperEditor", "histogram file:", 0, QApplication::UnicodeUTF8));
        m_histogramFileLineEdit->setText(QString());
        m_histogramFilePushButton->setText(QApplication::translate("HistogramRemapperEditor", "set histogram file", 0, QApplication::UnicodeUTF8));
        m_enableButton->setText(QApplication::translate("HistogramRemapperEditor", "Enable", 0, QApplication::UnicodeUTF8));
        m_resetButton->setText(QApplication::translate("HistogramRemapperEditor", "Reset", 0, QApplication::UnicodeUTF8));
        m_okButton->setText(QApplication::translate("HistogramRemapperEditor", "Ok", 0, QApplication::UnicodeUTF8));
        m_cancelButton->setText(QApplication::translate("HistogramRemapperEditor", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HistogramRemapperEditor: public Ui_HistogramRemapperEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HISTOGRAMREMAPPEREDITOR_H
