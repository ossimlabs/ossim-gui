//---
//
// License:  See top level LICENSE.txt file.
//
// Description:  Description: Dialog box for ProgressWidget.
//
//---
// $Id$

#include <ossimGui/ProgressDialog.h>
#include <ossimGui/ProgressWidget.h>
#include <ossim/base/ossimTrace.h>

#include <QVBoxLayout>
#include <QWidget>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

static ossimTrace traceDebug("ProgressDialog:debug");

ossimGui::ProgressDialog::ProgressDialog(QWidget* parent, Qt::WFlags f) 
   :
   QDialog( parent, f ),
   m_widget(0)
{
   m_widget = new ossimGui::ProgressWidget(this);
   m_widget->setValue(0);

   // setup vertical layout   
   QVBoxLayout* vbox0 = new QVBoxLayout();
    
   vbox0->addWidget( m_widget );

   setLayout( vbox0 );
}

ossimGui::ProgressDialog::~ProgressDialog()
{
   // m_annotator.removeListener((ossimROIEventListener*)this);

   // Pretty sure the widget is parented to "this" and will be deleted in the
   // QT code.  Need to verify...
   
   // delete m_widget;
   // m_widget = 0;
}

ossimGui::ProgressWidget* ossimGui::ProgressDialog::progressWidget()
{
   return m_widget;
}

