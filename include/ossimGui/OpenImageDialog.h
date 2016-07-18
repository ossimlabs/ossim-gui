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

#ifndef ossimGuiOpenImageDialog_HEADER
#define ossimGuiOpenImageDialog_HEADER 1

#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossimGui/Common.h>
#include <ossimGui/Export.h>
#include <QDialog>

// Forward class declarations:
class QButtonGroup;

namespace ossimGui
{
   class OSSIMGUI_DLL OpenImageDialog : public QDialog
   {
   Q_OBJECT

   public:
   
      /** @brief default constructor */
      OpenImageDialog( ossimImageHandler* ih,
                       QWidget* parent=0,
                       Qt::WindowFlags f = 0 );

      /**
       * @brief Adds selected handlers to the list.
       *
       * This does not clear or resize "handlers" if no entries are selected.
       * 
       * @param handlers Initialized by this.
       */
      void handlerList(ossimGui::HandlerList& handlers);

   public slots:

      /**
       * @brief Connected to "cancel" button.
       * Override QDialog::reject to uncheck all entries on cancel.
       */
      void reject();

      /**
       * @brief Connected to "all" check box.
       */
      void allStateChanged( int state );

   private:

      // Hold group of check boxes.  One for each entry.
      QButtonGroup* m_entryButtonGroup;

      // Holds pointer to open image handler.
      ossimRefPtr<ossimImageHandler> m_ih;
   };
}

#endif /* #ifndef ossimGuiOpenImageDialog_HEADER */
