//---
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for property editor.
//
//---
// $Id$

#include <ossimGui/PropertyEditorDialog.h>
#include "ossimGui/DataManagerPropertyView.h"

#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

ossimGui::PropertyEditorDialog::PropertyEditorDialog(QWidget* parent, Qt::WFlags f) 
   :
   QDialog( parent, f ),
   m_propertyView(0)
{
   // set title in Polygon Remapper Dialog
   this->setWindowTitle( tr("Property Editor") );

   this->setModal( false );
   
   // setup vertical layout   
   QVBoxLayout* vbox0 = new QVBoxLayout();

   // Row 1: property editor:
   QGroupBox* propertyEditorGroupBox = new QGroupBox(tr("properties"));
   QHBoxLayout* hboxR1 = new QHBoxLayout();
   m_propertyView = new ossimGui::DataManagerPropertyView();
   hboxR1->addWidget( m_propertyView );
   propertyEditorGroupBox->setLayout( hboxR1 );
   vbox0->addWidget(propertyEditorGroupBox);

   // Row 2:
   QHBoxLayout* hboxR2 = new QHBoxLayout();

   // close:
   QPushButton* closePushButton = new QPushButton( tr("close") );
   hboxR2->addWidget( closePushButton );

   vbox0->addLayout(hboxR2);

   setLayout(vbox0);

   // Signals and slots connections:
   connect(closePushButton, SIGNAL(clicked()),
           this, SLOT(close()));
}


void ossimGui::PropertyEditorDialog::setObject( ossimObject* object )
{
   if ( m_propertyView )
   {
      m_propertyView->setObject( object );
   }
}
