//---
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for property editor.
//
//---
// $Id$

#ifndef ossimGuiPropertyEditorDialog_HEADER
#define ossimGuiPropertyEditorDialog_HEADER 1

#include <QDialog>

class ossimObject;

namespace ossimGui
{
   class DataManagerPropertyView;
   
   class PropertyEditorDialog : public QDialog
   {
   Q_OBJECT

   public:
      
      /** @brief default constructor */
      PropertyEditorDialog( QWidget* parent=0, Qt::WFlags f = 0 );

      void setObject( ossimObject* input );

   public slots:
      
   protected:

   private:

      ossimGui::DataManagerPropertyView* m_propertyView;
   };
}

#endif /* #ifndef ossimGuiPropertyEditorDialog_HEADER */
