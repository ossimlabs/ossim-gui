//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Region Of Interest (ROI) rectangle annotator object.
//
// $Id$
//----------------------------------------------------------------------------
#ifndef ossimGuiRoiRectAnnotator_HEADER
#define ossimGuiRoiRectAnnotator_HEADER 1

#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimIpt.h>
#include <QObject>
#include <QtGui/QColor>
#include <vector>

class ossimIrect;
class QMouseEvent;
class QPainter;
class QPoint;
class QRectF;

namespace ossimGui
{
   class ImageScrollView;
   
   class RoiRectAnnotator 
      : public QObject, public ossimListenerManager
   {
      Q_OBJECT
      
   public:
      
      RoiRectAnnotator();
      virtual ~RoiRectAnnotator();

      /**
       * @param widget The widget to paint to.
       * 
       * @note This will connect the widgets paintYourGraphics
       * signal to slot paint.
       */
      void setImageWidget(ossimGui::ImageScrollView* widget);

      /** Set theEnableFlag to true enabling the paint method. */
      void enablePaint();
      
      /** Set theEnableFlag to false short circuiting the paint method. */
      void disablePaint();   

      /**
       * @param pts The points to draw.
       *
       * @note Points should be in QT space.
       * 0,0 = upper left, x positive right, y positive down.
       */
      void setPoints(const std::vector<ossimIpt>& pts);
      
      /** Clears thePoints. */
      void clear();
      
      /**
       * @param pts Vector of ossimIpts to initialize.
       * 
       * @note Points are in QT space.
       * 0,0 = upper left, x positive right, y positive down.
       */
      virtual void getPoints(std::vector<ossimIpt>& pts) const;
      
      /**
       * Set the roi rectangle.
       * 
       * @param rect Rectangle to set.
       *
       * @note This method takes a rectangle that's in native QImage space.
       * 
       * */
      void setRoiRect(const ossimIrect& rect);
      
      /**
       * @return the current roi rectangle.
       *
       * @note This method returns a rectangle that's in QImage space.
       */
      ossimIrect getRoiRect() const;

   public slots:

      void mousePress(QMouseEvent* e);
      void mouseMove(QMouseEvent* e);
      void mouseRelease(QMouseEvent* e);

      /**
       * Slot to hook up to the widget's paintYourGraphics signal.
       */
      void paint(QPainter* p, const QRectF& rect); 
      
   private:

      /** Force viewport update. */
      void refresh();

      /** @brief Copy QPoint to ossimIpt. */
      void qPtToIpt( const QPoint& qpt, ossimIpt& ipt ) const;
      
      /**
       * Called by base class slot paint.
       */
      virtual void paintAnnotation(QPainter* p, const QRectF& rect);
      
   private:
      /** Hidden from use copy constructor. */
      RoiRectAnnotator( const ossimGui::RoiRectAnnotator&);
      
      /** Hidden from use assignment operator. */
      RoiRectAnnotator& operator=( const ossimGui::RoiRectAnnotator&);

      /** The widget to paint to. */
      ImageScrollView* m_widget;
      
      /**
       * Enables/disables call to paintAnnotation method.
       * Defaulted to true in constructor.
       */
      bool m_enableFlag;

      bool     m_roiLeftPressedFlag;
      bool     m_roiMiddlePresedFlag;
      ossimIpt m_roiPressStart;
      QColor   m_penColor;

      /**
       * Points which drawing to widget will be based on.
       * 
       * @note These points are stored non shifted in QT widget space.
       * 0,0 = upper left, x positive right, y positive down.
       */
      std::vector<ossimIpt> m_points;   
   };
}

#endif /* #ifndef ossimGuiRoiRectAnnotator_HEADER */
