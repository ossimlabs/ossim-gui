/********************************************************************************
** Form generated from reading UI file 'About.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_About
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *m_aboutText;
    QSpacerItem *verticalSpacer;
    QGroupBox *m_sponsors;
    QHBoxLayout *horizontalLayout;
    QLabel *m_logo;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *m_okButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *About)
    {
        if (About->objectName().isEmpty())
            About->setObjectName(QString::fromUtf8("About"));
        About->resize(505, 304);
        verticalLayout = new QVBoxLayout(About);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_aboutText = new QLabel(About);
        m_aboutText->setObjectName(QString::fromUtf8("m_aboutText"));
        m_aboutText->setFrameShape(QFrame::StyledPanel);
        m_aboutText->setFrameShadow(QFrame::Plain);
        m_aboutText->setTextFormat(Qt::AutoText);
        m_aboutText->setScaledContents(false);
        m_aboutText->setAlignment(Qt::AlignCenter);
        m_aboutText->setWordWrap(true);

        verticalLayout->addWidget(m_aboutText);

        verticalSpacer = new QSpacerItem(20, 121, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        m_sponsors = new QGroupBox(About);
        m_sponsors->setObjectName(QString::fromUtf8("m_sponsors"));
        QFont font;
        font.setPointSize(18);
        m_sponsors->setFont(font);
        horizontalLayout = new QHBoxLayout(m_sponsors);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_logo = new QLabel(m_sponsors);
        m_logo->setObjectName(QString::fromUtf8("m_logo"));
        m_logo->setFrameShape(QFrame::Box);
        m_logo->setFrameShadow(QFrame::Sunken);
        m_logo->setScaledContents(false);
        m_logo->setOpenExternalLinks(true);

        horizontalLayout->addWidget(m_logo);


        verticalLayout->addWidget(m_sponsors);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        m_okButton = new QPushButton(About);
        m_okButton->setObjectName(QString::fromUtf8("m_okButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_okButton->sizePolicy().hasHeightForWidth());
        m_okButton->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(m_okButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(About);

        QMetaObject::connectSlotsByName(About);
    } // setupUi

    void retranslateUi(QDialog *About)
    {
        About->setWindowTitle(QApplication::translate("About", "About GeoCell", 0, QApplication::UnicodeUTF8));
        m_aboutText->setText(QString());
        m_sponsors->setTitle(QApplication::translate("About", "Support", 0, QApplication::UnicodeUTF8));
        m_logo->setText(QString());
        m_okButton->setText(QApplication::translate("About", "Ok", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class About: public Ui_About {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
