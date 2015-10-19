/********************************************************************************
** Form generated from reading UI file 'BandSelectorEditor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BANDSELECTOREDITOR_H
#define UI_BANDSELECTOREDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_BandSelectorEditor
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *m_outputBandGroup;
    QHBoxLayout *horizontalLayout_3;
    QRadioButton *m_oneBandButton;
    QRadioButton *m_threeBandButton;
    QRadioButton *m_nBandButton;
    QHBoxLayout *m_bandInputLayout;
    QLabel *m_bandsLabel;
    QLineEdit *m_bandInput;
    QPushButton *m_clearBandInputButton;
    QGroupBox *m_bandSelectionGroup;
    QHBoxLayout *horizontalLayout;
    QListWidget *m_inputBandList;
    QHBoxLayout *m_buttonLayout;
    QCheckBox *m_enableButton;
    QPushButton *m_resetButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    void setupUi(QDialog *BandSelectorEditor)
    {
        if (BandSelectorEditor->objectName().isEmpty())
            BandSelectorEditor->setObjectName(QString::fromUtf8("BandSelectorEditor"));
        BandSelectorEditor->resize(425, 386);
        verticalLayout_2 = new QVBoxLayout(BandSelectorEditor);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        m_outputBandGroup = new QGroupBox(BandSelectorEditor);
        m_outputBandGroup->setObjectName(QString::fromUtf8("m_outputBandGroup"));
        horizontalLayout_3 = new QHBoxLayout(m_outputBandGroup);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        m_oneBandButton = new QRadioButton(m_outputBandGroup);
        m_oneBandButton->setObjectName(QString::fromUtf8("m_oneBandButton"));

        horizontalLayout_3->addWidget(m_oneBandButton);

        m_threeBandButton = new QRadioButton(m_outputBandGroup);
        m_threeBandButton->setObjectName(QString::fromUtf8("m_threeBandButton"));

        horizontalLayout_3->addWidget(m_threeBandButton);

        m_nBandButton = new QRadioButton(m_outputBandGroup);
        m_nBandButton->setObjectName(QString::fromUtf8("m_nBandButton"));

        horizontalLayout_3->addWidget(m_nBandButton);


        verticalLayout_2->addWidget(m_outputBandGroup);

        m_bandInputLayout = new QHBoxLayout();
        m_bandInputLayout->setObjectName(QString::fromUtf8("m_bandInputLayout"));
        m_bandsLabel = new QLabel(BandSelectorEditor);
        m_bandsLabel->setObjectName(QString::fromUtf8("m_bandsLabel"));

        m_bandInputLayout->addWidget(m_bandsLabel);

        m_bandInput = new QLineEdit(BandSelectorEditor);
        m_bandInput->setObjectName(QString::fromUtf8("m_bandInput"));
        m_bandInput->setMaxLength(32767);
        m_bandInput->setReadOnly(false);

        m_bandInputLayout->addWidget(m_bandInput);

        m_clearBandInputButton = new QPushButton(BandSelectorEditor);
        m_clearBandInputButton->setObjectName(QString::fromUtf8("m_clearBandInputButton"));

        m_bandInputLayout->addWidget(m_clearBandInputButton);


        verticalLayout_2->addLayout(m_bandInputLayout);

        m_bandSelectionGroup = new QGroupBox(BandSelectorEditor);
        m_bandSelectionGroup->setObjectName(QString::fromUtf8("m_bandSelectionGroup"));
        horizontalLayout = new QHBoxLayout(m_bandSelectionGroup);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_inputBandList = new QListWidget(m_bandSelectionGroup);
        m_inputBandList->setObjectName(QString::fromUtf8("m_inputBandList"));

        horizontalLayout->addWidget(m_inputBandList);


        verticalLayout_2->addWidget(m_bandSelectionGroup);

        m_buttonLayout = new QHBoxLayout();
        m_buttonLayout->setObjectName(QString::fromUtf8("m_buttonLayout"));
        m_enableButton = new QCheckBox(BandSelectorEditor);
        m_enableButton->setObjectName(QString::fromUtf8("m_enableButton"));

        m_buttonLayout->addWidget(m_enableButton);

        m_resetButton = new QPushButton(BandSelectorEditor);
        m_resetButton->setObjectName(QString::fromUtf8("m_resetButton"));

        m_buttonLayout->addWidget(m_resetButton);

        m_okButton = new QPushButton(BandSelectorEditor);
        m_okButton->setObjectName(QString::fromUtf8("m_okButton"));

        m_buttonLayout->addWidget(m_okButton);

        m_cancelButton = new QPushButton(BandSelectorEditor);
        m_cancelButton->setObjectName(QString::fromUtf8("m_cancelButton"));

        m_buttonLayout->addWidget(m_cancelButton);


        verticalLayout_2->addLayout(m_buttonLayout);


        retranslateUi(BandSelectorEditor);

        QMetaObject::connectSlotsByName(BandSelectorEditor);
    } // setupUi

    void retranslateUi(QDialog *BandSelectorEditor)
    {
        BandSelectorEditor->setWindowTitle(QApplication::translate("BandSelectorEditor", "Dialog", 0, QApplication::UnicodeUTF8));
        m_outputBandGroup->setTitle(QApplication::translate("BandSelectorEditor", "Output Bands", 0, QApplication::UnicodeUTF8));
        m_oneBandButton->setText(QApplication::translate("BandSelectorEditor", "1-Band", 0, QApplication::UnicodeUTF8));
        m_threeBandButton->setText(QApplication::translate("BandSelectorEditor", "3-Band", 0, QApplication::UnicodeUTF8));
        m_nBandButton->setText(QApplication::translate("BandSelectorEditor", "N-Band", 0, QApplication::UnicodeUTF8));
        m_bandsLabel->setText(QApplication::translate("BandSelectorEditor", "Bands:", 0, QApplication::UnicodeUTF8));
        m_clearBandInputButton->setText(QApplication::translate("BandSelectorEditor", "Clear", 0, QApplication::UnicodeUTF8));
        m_bandSelectionGroup->setTitle(QApplication::translate("BandSelectorEditor", "Input Band Selection", 0, QApplication::UnicodeUTF8));
        m_enableButton->setText(QApplication::translate("BandSelectorEditor", "Enable", 0, QApplication::UnicodeUTF8));
        m_resetButton->setText(QApplication::translate("BandSelectorEditor", "Reset", 0, QApplication::UnicodeUTF8));
        m_okButton->setText(QApplication::translate("BandSelectorEditor", "Ok", 0, QApplication::UnicodeUTF8));
        m_cancelButton->setText(QApplication::translate("BandSelectorEditor", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class BandSelectorEditor: public Ui_BandSelectorEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BANDSELECTOREDITOR_H
