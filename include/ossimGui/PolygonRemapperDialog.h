//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for simple text data, e.g. position information.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimGuiPolygonRemapperDialog_HEADER
#define ossimGuiPolygonRemapperDialog_HEADER 1

#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimGeoPolyCutter.h>

#include <QDialog>
class ossimDpt;
class ossimDrect;
class ossimGeoPolyCutter;
class QMouseEvent;
class QString;
class QTextEdit;
class QCheckBox;
class QLineEdit;
class QLabel;

namespace ossimGui
{
   class ImageScrollView;
   
   class PolygonRemapperDialog : public QDialog
   {
   Q_OBJECT

   public:
      
      /** @brief default constructor */
      PolygonRemapperDialog( QWidget* parent=0, Qt::WFlags f = 0 );

      void setWidget( ossimGui::ImageScrollView* widget );
      void setPolyCutter( ossimGeoPolyCutter* polygon );
      
      enum Mode
      {
         UNKNOWN,
         ACCEPTING_POINTS
      };

   public slots:

      void track( const ossimDpt& scenePt );
      void mousePress(QMouseEvent* e, const ossimDpt& scenePoint);
      void getPolyPoint();
      void toggleEnableDisable();
      void toggleCutType();
      void setFillType();
      
   protected:

   private:
      void fireRefreshEvent();
      void sceneToImage( const ossimDpt& scenePt, ossimDpt& imagePt ) const;

      QTextEdit*                      m_textEdit;
      ossimGui::ImageScrollView*      m_widget;
      ossimRefPtr<ossimGeoPolyCutter> m_polyCutter;
      QPushButton*                    m_addPolygonBt;
      QCheckBox*                      m_enableDisableCb;
      QCheckBox*                      m_toggleCutTypeCb;
      QLineEdit*                      m_fillValueLe;
      QPushButton*                    m_setfillBt;
      std::vector<ossimGpt>           m_polygon;
      Mode                            m_mode;
   };
}

#endif /* #ifndef ossimGuiPolygonRemapperDialog_HEADER */
