/********************************************************************************
** Form generated from reading UI file 'ExportImageDialog.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORTIMAGEDIALOG_H
#define UI_EXPORTIMAGEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include "ossimGui/DataManagerPropertyView.h"
#include "ossimGui/ProgressWidget.h"

QT_BEGIN_NAMESPACE

class Ui_ExportImageDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *m_generalInformationBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QGridLayout *gridLayout_2;
    QLabel *m_widthLabel;
    QLineEdit *m_width;
    QLabel *m_heightLabel;
    QLineEdit *m_height;
    QGridLayout *gridLayout;
    QLabel *m_scalarTypeLabel;
    QLineEdit *m_scalarType;
    QLabel *m_bandsLabel;
    QLineEdit *m_bands;
    QHBoxLayout *horizontalLayout_3;
    QLabel *m_sizeLabel;
    QLineEdit *m_size;
    QHBoxLayout *m_fileLayout;
    QComboBox *m_fileTypes;
    QPushButton *m_fileButton;
    QGroupBox *m_propertiesGroupBox;
    QVBoxLayout *verticalLayout;
    ossimGui::DataManagerPropertyView *m_propertyView;
    ossimGui::ProgressWidget *m_progressBar;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *m_leftHorizontalSpacer;
    QCheckBox *m_exportInBackground;
    QPushButton *m_exportAbortButton;
    QPushButton *m_closeButton;
    QSpacerItem *m_rightHorizontalSpacer;

    void setupUi(QDialog *ExportImageDialog)
    {
        if (ExportImageDialog->objectName().isEmpty())
            ExportImageDialog->setObjectName(QString::fromUtf8("ExportImageDialog"));
        ExportImageDialog->resize(681, 557);
        verticalLayout_3 = new QVBoxLayout(ExportImageDialog);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        m_generalInformationBox = new QGroupBox(ExportImageDialog);
        m_generalInformationBox->setObjectName(QString::fromUtf8("m_generalInformationBox"));
        verticalLayout_2 = new QVBoxLayout(m_generalInformationBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        m_widthLabel = new QLabel(m_generalInformationBox);
        m_widthLabel->setObjectName(QString::fromUtf8("m_widthLabel"));

        gridLayout_2->addWidget(m_widthLabel, 0, 0, 1, 1);

        m_width = new QLineEdit(m_generalInformationBox);
        m_width->setObjectName(QString::fromUtf8("m_width"));
        m_width->setReadOnly(true);

        gridLayout_2->addWidget(m_width, 0, 1, 1, 1);

        m_heightLabel = new QLabel(m_generalInformationBox);
        m_heightLabel->setObjectName(QString::fromUtf8("m_heightLabel"));

        gridLayout_2->addWidget(m_heightLabel, 1, 0, 1, 1);

        m_height = new QLineEdit(m_generalInformationBox);
        m_height->setObjectName(QString::fromUtf8("m_height"));
        m_height->setReadOnly(true);

        gridLayout_2->addWidget(m_height, 1, 1, 1, 1);


        horizontalLayout_2->addLayout(gridLayout_2);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        m_scalarTypeLabel = new QLabel(m_generalInformationBox);
        m_scalarTypeLabel->setObjectName(QString::fromUtf8("m_scalarTypeLabel"));

        gridLayout->addWidget(m_scalarTypeLabel, 0, 0, 1, 1);

        m_scalarType = new QLineEdit(m_generalInformationBox);
        m_scalarType->setObjectName(QString::fromUtf8("m_scalarType"));
        m_scalarType->setReadOnly(true);

        gridLayout->addWidget(m_scalarType, 0, 1, 2, 1);

        m_bandsLabel = new QLabel(m_generalInformationBox);
        m_bandsLabel->setObjectName(QString::fromUtf8("m_bandsLabel"));

        gridLayout->addWidget(m_bandsLabel, 1, 0, 2, 1);

        m_bands = new QLineEdit(m_generalInformationBox);
        m_bands->setObjectName(QString::fromUtf8("m_bands"));
        m_bands->setReadOnly(true);

        gridLayout->addWidget(m_bands, 2, 1, 1, 1);


        horizontalLayout_2->addLayout(gridLayout);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        m_sizeLabel = new QLabel(m_generalInformationBox);
        m_sizeLabel->setObjectName(QString::fromUtf8("m_sizeLabel"));

        horizontalLayout_3->addWidget(m_sizeLabel);

        m_size = new QLineEdit(m_generalInformationBox);
        m_size->setObjectName(QString::fromUtf8("m_size"));
        m_size->setReadOnly(true);

        horizontalLayout_3->addWidget(m_size);


        verticalLayout_2->addLayout(horizontalLayout_3);


        verticalLayout_3->addWidget(m_generalInformationBox);

        m_fileLayout = new QHBoxLayout();
        m_fileLayout->setObjectName(QString::fromUtf8("m_fileLayout"));
        m_fileTypes = new QComboBox(ExportImageDialog);
        m_fileTypes->setObjectName(QString::fromUtf8("m_fileTypes"));

        m_fileLayout->addWidget(m_fileTypes);

        m_fileButton = new QPushButton(ExportImageDialog);
        m_fileButton->setObjectName(QString::fromUtf8("m_fileButton"));

        m_fileLayout->addWidget(m_fileButton);


        verticalLayout_3->addLayout(m_fileLayout);

        m_propertiesGroupBox = new QGroupBox(ExportImageDialog);
        m_propertiesGroupBox->setObjectName(QString::fromUtf8("m_propertiesGroupBox"));
        QFont font;
        font.setPointSize(16);
        m_propertiesGroupBox->setFont(font);
        m_propertiesGroupBox->setAlignment(Qt::AlignCenter);
        verticalLayout = new QVBoxLayout(m_propertiesGroupBox);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_propertyView = new ossimGui::DataManagerPropertyView(m_propertiesGroupBox);
        m_propertyView->setObjectName(QString::fromUtf8("m_propertyView"));
        m_propertyView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        verticalLayout->addWidget(m_propertyView);


        verticalLayout_3->addWidget(m_propertiesGroupBox);

        m_progressBar = new ossimGui::ProgressWidget(ExportImageDialog);
        m_progressBar->setObjectName(QString::fromUtf8("m_progressBar"));
        m_progressBar->setValue(0);

        verticalLayout_3->addWidget(m_progressBar);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_leftHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(m_leftHorizontalSpacer);

        m_exportInBackground = new QCheckBox(ExportImageDialog);
        m_exportInBackground->setObjectName(QString::fromUtf8("m_exportInBackground"));
        m_exportInBackground->setChecked(true);

        horizontalLayout->addWidget(m_exportInBackground);

        m_exportAbortButton = new QPushButton(ExportImageDialog);
        m_exportAbortButton->setObjectName(QString::fromUtf8("m_exportAbortButton"));

        horizontalLayout->addWidget(m_exportAbortButton);

        m_closeButton = new QPushButton(ExportImageDialog);
        m_closeButton->setObjectName(QString::fromUtf8("m_closeButton"));

        horizontalLayout->addWidget(m_closeButton);

        m_rightHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(m_rightHorizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(ExportImageDialog);

        QMetaObject::connectSlotsByName(ExportImageDialog);
    } // setupUi

    void retranslateUi(QDialog *ExportImageDialog)
    {
        ExportImageDialog->setWindowTitle(QApplication::translate("ExportImageDialog", "Export Image", 0, QApplication::UnicodeUTF8));
        m_generalInformationBox->setTitle(QApplication::translate("ExportImageDialog", "General Export Info", 0, QApplication::UnicodeUTF8));
        m_widthLabel->setText(QApplication::translate("ExportImageDialog", "Width:", 0, QApplication::UnicodeUTF8));
        m_heightLabel->setText(QApplication::translate("ExportImageDialog", "Height:", 0, QApplication::UnicodeUTF8));
        m_scalarTypeLabel->setText(QApplication::translate("ExportImageDialog", "Scalar Type:", 0, QApplication::UnicodeUTF8));
        m_bandsLabel->setText(QApplication::translate("ExportImageDialog", "Bands:", 0, QApplication::UnicodeUTF8));
        m_sizeLabel->setText(QApplication::translate("ExportImageDialog", "Approximate Uncompressed Size:", 0, QApplication::UnicodeUTF8));
        m_fileButton->setText(QApplication::translate("ExportImageDialog", "File", 0, QApplication::UnicodeUTF8));
        m_propertiesGroupBox->setTitle(QApplication::translate("ExportImageDialog", "Writer Properties", 0, QApplication::UnicodeUTF8));
        m_exportInBackground->setText(QApplication::translate("ExportImageDialog", "Export in Background", 0, QApplication::UnicodeUTF8));
        m_exportAbortButton->setText(QApplication::translate("ExportImageDialog", "Export", 0, QApplication::UnicodeUTF8));
        m_closeButton->setText(QApplication::translate("ExportImageDialog", "Close", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ExportImageDialog: public Ui_ExportImageDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORTIMAGEDIALOG_H
