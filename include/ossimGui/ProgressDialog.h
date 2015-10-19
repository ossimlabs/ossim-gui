//---
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for progress widget.
//
//---
// $Id$

#ifndef ossimGuiProgressDialog_HEADER
#define ossimGuiProgressDialog_HEADER 1

#include <QDialog>

namespace ossimGui
{
   class ProgressWidget;
   
   class ProgressDialog : public QDialog
   {
   Q_OBJECT

   public:
      
      /** @brief default constructor */
      ProgressDialog( QWidget* parent=0, Qt::WFlags f = 0 );

      virtual ~ProgressDialog();

      ossimGui::ProgressWidget* progressWidget(); 

   public slots:

   private:

      ossimGui::ProgressWidget* m_widget;
   };
}

#endif /* #ifndef ossimGuiProgressDialog_HEADER */
