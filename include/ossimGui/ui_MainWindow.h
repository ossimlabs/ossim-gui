/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>
#include "ossimGui/DataManagerWidget.h"

QT_BEGIN_NAMESPACE

class Ui_ossimGuiMainWindow
{
public:
    QAction *actionSubwindow_View;
    QAction *m_actionWindowTabbedView;
    QAction *m_actionCloseProject;
    QAction *m_actionNewProject;
    QAction *m_actionSaveSessionAs;
    QAction *m_actionSaveSession;
    QAction *m_actionDatumConverter;
    QWidget *m_centralWidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *m_splitter;
    ossimGui::DataManagerWidget *m_dataManagerWidget;
    QMdiArea *m_mdiArea;
    QMenuBar *m_menubar;
    QStatusBar *m_statusbar;

    void setupUi(QMainWindow *ossimGuiMainWindow)
    {
        if (ossimGuiMainWindow->objectName().isEmpty())
            ossimGuiMainWindow->setObjectName(QString::fromUtf8("ossimGuiMainWindow"));
        ossimGuiMainWindow->resize(1255, 876);
        ossimGuiMainWindow->setAcceptDrops(true);
        actionSubwindow_View = new QAction(ossimGuiMainWindow);
        actionSubwindow_View->setObjectName(QString::fromUtf8("actionSubwindow_View"));
        m_actionWindowTabbedView = new QAction(ossimGuiMainWindow);
        m_actionWindowTabbedView->setObjectName(QString::fromUtf8("m_actionWindowTabbedView"));
        m_actionWindowTabbedView->setCheckable(true);
        m_actionCloseProject = new QAction(ossimGuiMainWindow);
        m_actionCloseProject->setObjectName(QString::fromUtf8("m_actionCloseProject"));
        m_actionNewProject = new QAction(ossimGuiMainWindow);
        m_actionNewProject->setObjectName(QString::fromUtf8("m_actionNewProject"));
        m_actionSaveSessionAs = new QAction(ossimGuiMainWindow);
        m_actionSaveSessionAs->setObjectName(QString::fromUtf8("m_actionSaveSessionAs"));
        m_actionSaveSession = new QAction(ossimGuiMainWindow);
        m_actionSaveSession->setObjectName(QString::fromUtf8("m_actionSaveSession"));
        m_actionDatumConverter = new QAction(ossimGuiMainWindow);
        m_actionDatumConverter->setObjectName(QString::fromUtf8("m_actionDatumConverter"));
        m_centralWidget = new QWidget(ossimGuiMainWindow);
        m_centralWidget->setObjectName(QString::fromUtf8("m_centralWidget"));
        m_centralWidget->setAcceptDrops(false);
        horizontalLayout = new QHBoxLayout(m_centralWidget);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_splitter = new QSplitter(m_centralWidget);
        m_splitter->setObjectName(QString::fromUtf8("m_splitter"));
        m_splitter->setOrientation(Qt::Horizontal);
        m_dataManagerWidget = new ossimGui::DataManagerWidget(m_splitter);
        m_dataManagerWidget->setObjectName(QString::fromUtf8("m_dataManagerWidget"));
        m_dataManagerWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_splitter->addWidget(m_dataManagerWidget);
        m_mdiArea = new QMdiArea(m_splitter);
        m_mdiArea->setObjectName(QString::fromUtf8("m_mdiArea"));
        m_mdiArea->setAcceptDrops(false);
        m_mdiArea->setFrameShape(QFrame::StyledPanel);
        m_mdiArea->setFrameShadow(QFrame::Plain);
        m_mdiArea->setLineWidth(1);
        m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_mdiArea->setViewMode(QMdiArea::SubWindowView);
        m_mdiArea->setTabShape(QTabWidget::Rounded);
        m_splitter->addWidget(m_mdiArea);

        horizontalLayout->addWidget(m_splitter);

        ossimGuiMainWindow->setCentralWidget(m_centralWidget);
        m_menubar = new QMenuBar(ossimGuiMainWindow);
        m_menubar->setObjectName(QString::fromUtf8("m_menubar"));
        m_menubar->setGeometry(QRect(0, 0, 1255, 22));
        ossimGuiMainWindow->setMenuBar(m_menubar);
        m_statusbar = new QStatusBar(ossimGuiMainWindow);
        m_statusbar->setObjectName(QString::fromUtf8("m_statusbar"));
        ossimGuiMainWindow->setStatusBar(m_statusbar);

        retranslateUi(ossimGuiMainWindow);

        QMetaObject::connectSlotsByName(ossimGuiMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ossimGuiMainWindow)
    {
        ossimGuiMainWindow->setWindowTitle(QApplication::translate("ossimGuiMainWindow", "OSSIM Main WIndow", 0, QApplication::UnicodeUTF8));
        actionSubwindow_View->setText(QApplication::translate("ossimGuiMainWindow", "Subwindow View", 0, QApplication::UnicodeUTF8));
        m_actionWindowTabbedView->setText(QApplication::translate("ossimGuiMainWindow", "Tabbed View", 0, QApplication::UnicodeUTF8));
        m_actionCloseProject->setText(QApplication::translate("ossimGuiMainWindow", "Close Project", 0, QApplication::UnicodeUTF8));
        m_actionNewProject->setText(QApplication::translate("ossimGuiMainWindow", "New Project", 0, QApplication::UnicodeUTF8));
        m_actionSaveSessionAs->setText(QApplication::translate("ossimGuiMainWindow", "Save Project As", 0, QApplication::UnicodeUTF8));
        m_actionSaveSession->setText(QApplication::translate("ossimGuiMainWindow", "Save Project", 0, QApplication::UnicodeUTF8));
        m_actionDatumConverter->setText(QApplication::translate("ossimGuiMainWindow", "Datum Converter", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem = m_dataManagerWidget->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("ossimGuiMainWindow", "Data Manager", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ossimGuiMainWindow: public Ui_ossimGuiMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
