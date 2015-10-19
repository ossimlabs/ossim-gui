/********************************************************************************
** Form generated from reading UI file 'AdjustableParameterEditor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADJUSTABLEPARAMETEREDITOR_H
#define UI_ADJUSTABLEPARAMETEREDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AdjustableParameterEditor
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QGridLayout *gridLayout;
    QLineEdit *m_adjustmentDescriptionInput;
    QLabel *m_adjustmentSelectionLabel;
    QComboBox *m_adjustmentSelectionBox;
    QLabel *m_adjustmentDescriptionLabel;
    QLabel *label;
    QLabel *m_imageSourceLabel;
    QTableWidget *m_adjustableParameterTable;
    QFrame *line1;
    QHBoxLayout *hboxLayout;
    QPushButton *m_keepAdjustmentButton;
    QPushButton *m_copyAdjustmentButton;
    QPushButton *m_deleteAdjustmentButton;
    QPushButton *m_resetButton;
    QPushButton *m_saveButton;
    QPushButton *m_closeButton;

    void setupUi(QDialog *AdjustableParameterEditor)
    {
        if (AdjustableParameterEditor->objectName().isEmpty())
            AdjustableParameterEditor->setObjectName(QString::fromUtf8("AdjustableParameterEditor"));
        AdjustableParameterEditor->resize(730, 427);
        verticalLayout = new QVBoxLayout(AdjustableParameterEditor);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        widget = new QWidget(AdjustableParameterEditor);
        widget->setObjectName(QString::fromUtf8("widget"));
        gridLayout = new QGridLayout(widget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        m_adjustmentDescriptionInput = new QLineEdit(widget);
        m_adjustmentDescriptionInput->setObjectName(QString::fromUtf8("m_adjustmentDescriptionInput"));

        gridLayout->addWidget(m_adjustmentDescriptionInput, 2, 2, 1, 1);

        m_adjustmentSelectionLabel = new QLabel(widget);
        m_adjustmentSelectionLabel->setObjectName(QString::fromUtf8("m_adjustmentSelectionLabel"));
        m_adjustmentSelectionLabel->setWordWrap(false);

        gridLayout->addWidget(m_adjustmentSelectionLabel, 0, 0, 1, 1);

        m_adjustmentSelectionBox = new QComboBox(widget);
        m_adjustmentSelectionBox->setObjectName(QString::fromUtf8("m_adjustmentSelectionBox"));

        gridLayout->addWidget(m_adjustmentSelectionBox, 0, 2, 1, 1);

        m_adjustmentDescriptionLabel = new QLabel(widget);
        m_adjustmentDescriptionLabel->setObjectName(QString::fromUtf8("m_adjustmentDescriptionLabel"));
        m_adjustmentDescriptionLabel->setWordWrap(false);

        gridLayout->addWidget(m_adjustmentDescriptionLabel, 2, 0, 1, 1);

        label = new QLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 3, 0, 1, 1);

        m_imageSourceLabel = new QLabel(widget);
        m_imageSourceLabel->setObjectName(QString::fromUtf8("m_imageSourceLabel"));

        gridLayout->addWidget(m_imageSourceLabel, 3, 2, 1, 1);


        verticalLayout->addWidget(widget);

        m_adjustableParameterTable = new QTableWidget(AdjustableParameterEditor);
        if (m_adjustableParameterTable->columnCount() < 5)
            m_adjustableParameterTable->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        m_adjustableParameterTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        m_adjustableParameterTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        m_adjustableParameterTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        m_adjustableParameterTable->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        m_adjustableParameterTable->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        m_adjustableParameterTable->setObjectName(QString::fromUtf8("m_adjustableParameterTable"));

        verticalLayout->addWidget(m_adjustableParameterTable);

        line1 = new QFrame(AdjustableParameterEditor);
        line1->setObjectName(QString::fromUtf8("line1"));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line1);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        m_keepAdjustmentButton = new QPushButton(AdjustableParameterEditor);
        m_keepAdjustmentButton->setObjectName(QString::fromUtf8("m_keepAdjustmentButton"));
        m_keepAdjustmentButton->setAutoDefault(false);

        hboxLayout->addWidget(m_keepAdjustmentButton);

        m_copyAdjustmentButton = new QPushButton(AdjustableParameterEditor);
        m_copyAdjustmentButton->setObjectName(QString::fromUtf8("m_copyAdjustmentButton"));
        m_copyAdjustmentButton->setAutoDefault(false);

        hboxLayout->addWidget(m_copyAdjustmentButton);

        m_deleteAdjustmentButton = new QPushButton(AdjustableParameterEditor);
        m_deleteAdjustmentButton->setObjectName(QString::fromUtf8("m_deleteAdjustmentButton"));
        m_deleteAdjustmentButton->setAutoDefault(false);

        hboxLayout->addWidget(m_deleteAdjustmentButton);

        m_resetButton = new QPushButton(AdjustableParameterEditor);
        m_resetButton->setObjectName(QString::fromUtf8("m_resetButton"));
        m_resetButton->setAutoDefault(false);

        hboxLayout->addWidget(m_resetButton);

        m_saveButton = new QPushButton(AdjustableParameterEditor);
        m_saveButton->setObjectName(QString::fromUtf8("m_saveButton"));
        m_saveButton->setAutoDefault(false);

        hboxLayout->addWidget(m_saveButton);

        m_closeButton = new QPushButton(AdjustableParameterEditor);
        m_closeButton->setObjectName(QString::fromUtf8("m_closeButton"));
        m_closeButton->setAutoDefault(false);

        hboxLayout->addWidget(m_closeButton);


        verticalLayout->addLayout(hboxLayout);


        retranslateUi(AdjustableParameterEditor);

        QMetaObject::connectSlotsByName(AdjustableParameterEditor);
    } // setupUi

    void retranslateUi(QDialog *AdjustableParameterEditor)
    {
        AdjustableParameterEditor->setWindowTitle(QApplication::translate("AdjustableParameterEditor", "Parameter Adjustments", 0, QApplication::UnicodeUTF8));
        m_adjustmentSelectionLabel->setText(QApplication::translate("AdjustableParameterEditor", "Adjustment Selection:", 0, QApplication::UnicodeUTF8));
        m_adjustmentDescriptionLabel->setText(QApplication::translate("AdjustableParameterEditor", "Adjustment description:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("AdjustableParameterEditor", "Image Source", 0, QApplication::UnicodeUTF8));
        m_imageSourceLabel->setText(QString());
        QTableWidgetItem *___qtablewidgetitem = m_adjustableParameterTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("AdjustableParameterEditor", "Name", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = m_adjustableParameterTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("AdjustableParameterEditor", "Sigma", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = m_adjustableParameterTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("AdjustableParameterEditor", "Param", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = m_adjustableParameterTable->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("AdjustableParameterEditor", "Param adjust", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = m_adjustableParameterTable->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("AdjustableParameterEditor", "Value", 0, QApplication::UnicodeUTF8));
        m_keepAdjustmentButton->setText(QApplication::translate("AdjustableParameterEditor", "Keep", 0, QApplication::UnicodeUTF8));
        m_copyAdjustmentButton->setText(QApplication::translate("AdjustableParameterEditor", "Copy", 0, QApplication::UnicodeUTF8));
        m_deleteAdjustmentButton->setText(QApplication::translate("AdjustableParameterEditor", "Delete", 0, QApplication::UnicodeUTF8));
        m_resetButton->setText(QApplication::translate("AdjustableParameterEditor", "Reset", 0, QApplication::UnicodeUTF8));
        m_saveButton->setText(QApplication::translate("AdjustableParameterEditor", "Save...", 0, QApplication::UnicodeUTF8));
        m_closeButton->setText(QApplication::translate("AdjustableParameterEditor", "Close", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AdjustableParameterEditor: public Ui_AdjustableParameterEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADJUSTABLEPARAMETEREDITOR_H
