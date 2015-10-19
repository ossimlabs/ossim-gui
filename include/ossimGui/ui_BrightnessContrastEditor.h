/********************************************************************************
** Form generated from reading UI file 'BrightnessContrastEditor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BRIGHTNESSCONTRASTEDITOR_H
#define UI_BRIGHTNESSCONTRASTEDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_BrightnessContrastEditor
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QLabel *m_brightnessLabel;
    QLabel *m_contrastLabel;
    QVBoxLayout *verticalLayout;
    QSlider *m_brightnessSlider;
    QSlider *m_contrastSlider;
    QVBoxLayout *verticalLayout_3;
    QLineEdit *m_brightnessEdit;
    QLineEdit *m_contrastEdit;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QCheckBox *m_enabled;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *BrightnessContrastEditor)
    {
        if (BrightnessContrastEditor->objectName().isEmpty())
            BrightnessContrastEditor->setObjectName(QString::fromUtf8("BrightnessContrastEditor"));
        BrightnessContrastEditor->setWindowModality(Qt::NonModal);
        BrightnessContrastEditor->resize(717, 124);
        BrightnessContrastEditor->setModal(true);
        verticalLayout_4 = new QVBoxLayout(BrightnessContrastEditor);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        m_brightnessLabel = new QLabel(BrightnessContrastEditor);
        m_brightnessLabel->setObjectName(QString::fromUtf8("m_brightnessLabel"));

        verticalLayout_2->addWidget(m_brightnessLabel);

        m_contrastLabel = new QLabel(BrightnessContrastEditor);
        m_contrastLabel->setObjectName(QString::fromUtf8("m_contrastLabel"));

        verticalLayout_2->addWidget(m_contrastLabel);


        horizontalLayout->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_brightnessSlider = new QSlider(BrightnessContrastEditor);
        m_brightnessSlider->setObjectName(QString::fromUtf8("m_brightnessSlider"));
        m_brightnessSlider->setMinimum(-100);
        m_brightnessSlider->setMaximum(100);
        m_brightnessSlider->setOrientation(Qt::Horizontal);
        m_brightnessSlider->setTickPosition(QSlider::NoTicks);

        verticalLayout->addWidget(m_brightnessSlider);

        m_contrastSlider = new QSlider(BrightnessContrastEditor);
        m_contrastSlider->setObjectName(QString::fromUtf8("m_contrastSlider"));
        m_contrastSlider->setMinimum(-100);
        m_contrastSlider->setMaximum(100);
        m_contrastSlider->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(m_contrastSlider);


        horizontalLayout->addLayout(verticalLayout);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        m_brightnessEdit = new QLineEdit(BrightnessContrastEditor);
        m_brightnessEdit->setObjectName(QString::fromUtf8("m_brightnessEdit"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_brightnessEdit->sizePolicy().hasHeightForWidth());
        m_brightnessEdit->setSizePolicy(sizePolicy);
        m_brightnessEdit->setReadOnly(true);

        verticalLayout_3->addWidget(m_brightnessEdit);

        m_contrastEdit = new QLineEdit(BrightnessContrastEditor);
        m_contrastEdit->setObjectName(QString::fromUtf8("m_contrastEdit"));
        sizePolicy.setHeightForWidth(m_contrastEdit->sizePolicy().hasHeightForWidth());
        m_contrastEdit->setSizePolicy(sizePolicy);
        m_contrastEdit->setReadOnly(true);

        verticalLayout_3->addWidget(m_contrastEdit);


        horizontalLayout->addLayout(verticalLayout_3);


        verticalLayout_4->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        m_enabled = new QCheckBox(BrightnessContrastEditor);
        m_enabled->setObjectName(QString::fromUtf8("m_enabled"));

        horizontalLayout_2->addWidget(m_enabled);

        m_okButton = new QPushButton(BrightnessContrastEditor);
        m_okButton->setObjectName(QString::fromUtf8("m_okButton"));

        horizontalLayout_2->addWidget(m_okButton);

        m_cancelButton = new QPushButton(BrightnessContrastEditor);
        m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));

        horizontalLayout_2->addWidget(m_cancelButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_4->addLayout(horizontalLayout_2);


        retranslateUi(BrightnessContrastEditor);

        QMetaObject::connectSlotsByName(BrightnessContrastEditor);
    } // setupUi

    void retranslateUi(QDialog *BrightnessContrastEditor)
    {
        BrightnessContrastEditor->setWindowTitle(QApplication::translate("BrightnessContrastEditor", "Dialog", 0, QApplication::UnicodeUTF8));
        m_brightnessLabel->setText(QApplication::translate("BrightnessContrastEditor", "Brightness:", 0, QApplication::UnicodeUTF8));
        m_contrastLabel->setText(QApplication::translate("BrightnessContrastEditor", "Contrast:", 0, QApplication::UnicodeUTF8));
        m_enabled->setText(QApplication::translate("BrightnessContrastEditor", "Enabled", 0, QApplication::UnicodeUTF8));
        m_okButton->setText(QApplication::translate("BrightnessContrastEditor", "Ok", 0, QApplication::UnicodeUTF8));
        m_cancelButton->setText(QApplication::translate("BrightnessContrastEditor", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class BrightnessContrastEditor: public Ui_BrightnessContrastEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BRIGHTNESSCONTRASTEDITOR_H
