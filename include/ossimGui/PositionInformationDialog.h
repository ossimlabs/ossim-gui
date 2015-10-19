//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for simple text data, e.g. position information.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimGuiPositionInformationDialog_HEADER
#define ossimGuiPositionInformationDialog_HEADER 1

#include <QDialog>

class ossimDpt;
class QString;
class QTextEdit;

namespace ossimGui
{
   class ImageScrollView;
   
   class PositionInformationDialog : public QDialog
   {
   Q_OBJECT

   public:
      
      /** @brief default constructor */
      PositionInformationDialog( QWidget* parent=0, Qt::WFlags f = 0 );

      void setWidget( ossimGui::ImageScrollView* widget );
      
   public slots:

      void track( const ossimDpt& scenePt );

   protected:
      
      QTextEdit*                 m_textEdit;
      ossimGui::ImageScrollView* m_widget;

   private:
      void sceneToImage( const ossimDpt& scenePt, ossimDpt& imagePt ) const;
   };
}

#endif /* #ifndef ossimGuiPositionInformationDialog_HEADER */
