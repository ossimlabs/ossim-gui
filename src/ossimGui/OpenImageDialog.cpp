//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Open image dialog for handling multiple entry images.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossimGui/OpenImageDialog.h>
#include <ossimGui/Common.h>

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QString>
#include <QVBoxLayout>
#include <QCheckBox>

#include <vector>

ossimGui::OpenImageDialog::OpenImageDialog(ossimImageHandler* ih,
                                           QWidget* parent,
                                           Qt::WFlags f) 
   :
   QDialog( parent, f ),
   m_entryButtonGroup(0),
   m_ih(0)
{
   if ( ih )
   {
      // Capture image handler pointer.
      m_ih = ih;
      
      // set title in Polygon Remapper Dialog
      
      this->setWindowTitle( ih->getFilename().c_str() );

      // Make modal.  User must respond.
      this->setModal( true );

      // Top level vertical layout:
      QVBoxLayout* mainVbox = new QVBoxLayout();

      // Group box for entry check boxes.
      QGroupBox* entryGroupBox = new QGroupBox(tr("Image Entry Selection:"));
      
      // Vertical layout for entries:   
      QVBoxLayout* entryVbox = new QVBoxLayout();

      // Button group to hold/manage check boxes.
      m_entryButtonGroup = new QButtonGroup( this );

      // Not exclusive, i.e. can select any number of entries.
      m_entryButtonGroup->setExclusive( false );

      // Add the "all button":
      std::string name = "all entries";
      QString qs = name.c_str();
      QCheckBox* cb = new QCheckBox( qs );
      m_entryButtonGroup->addButton( cb, 0 );
      entryVbox->addWidget( cb );
      connect( cb, SIGNAL(stateChanged(int)), this, SLOT(allStateChanged(int)) );
      
      ossim_int32 nEntries = (ossim_int32)ih->getNumberOfEntries();
      for ( ossim_int32 entry = 0; entry < nEntries; ++entry )
      {
         ih->getEntryName( entry, name );
         if ( name.empty() )
         {
            name = "entry ";
            name += ossimString::toString(entry).string();
         }
         qs = name.c_str();

         // Create:
         cb = new QCheckBox( qs );

         // Mark checked:
         // cb->setCheckState(Qt::Checked);

         // Add to button group.  Note +1 to compensate for "all" button.
         m_entryButtonGroup->addButton( cb, (entry+1) );

         // Add to vertical box:
         entryVbox->addWidget( cb );
      }

      entryGroupBox->setLayout( entryVbox );

      mainVbox->addWidget( entryGroupBox );
      
      QDialogButtonBox* buttonBox = new QDialogButtonBox(
         QDialogButtonBox::Open|QDialogButtonBox::Cancel, Qt::Horizontal);
      connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
      connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
      
      mainVbox->addWidget( buttonBox );
      
      setLayout( mainVbox );
   }
   
} // End: ossimGui::OpenImageDialog::OpenImageDialog(...)

void ossimGui::OpenImageDialog::handlerList( ossimGui::HandlerList& handlers )
{
   if ( m_ih.valid() )
   {
      // Note, start at one as the "all" button is 0.
      QList<QAbstractButton*> buttons = m_entryButtonGroup->buttons();
      for ( ossim_int32 i = 1; i < buttons.length(); ++i )
      {
         QAbstractButton* button = buttons.at( i );
         if ( button )
         {
            if ( button->isChecked() )
            {
               ossimRefPtr<ossimImageHandler> ih =
                  static_cast<ossimImageHandler*>( m_ih->dup() );

               // Note, subtract one, "all" button again.
               if ( ih->setCurrentEntry( (ossim_uint32)(i-1) ) )
               {
                  handlers.push_back( ih.get() );
               }
            }
         }
      }
   }
}

// Slot connect
void ossimGui::OpenImageDialog::reject()
{
   // Uncheck all the buttons:
   if ( m_entryButtonGroup )
   {
      QList<QAbstractButton*> buttons = m_entryButtonGroup->buttons();
      for ( ossim_int32 i = 0; i < buttons.length(); ++i )
      {
         QAbstractButton* button = buttons.at( i );
         if ( button )
         {
            button->setChecked( false );
         }
      }
   }
   QDialog::reject();
}

void ossimGui::OpenImageDialog::allStateChanged( int state )
{
   // Uncheck all the buttons:
   if ( m_entryButtonGroup )
   {
      QList<QAbstractButton*> buttons = m_entryButtonGroup->buttons();
      for ( ossim_int32 i = 1; i < buttons.length(); ++i )
      {
         QAbstractButton* button = buttons.at( i );
         if ( button )
         {
            QCheckBox* cb = dynamic_cast<QCheckBox*>( button );
            if ( cb )
            {
               cb->setCheckState( (Qt::CheckState)state );
            }
         }
      }
   }
}
