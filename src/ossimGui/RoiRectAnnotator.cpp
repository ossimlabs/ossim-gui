//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:   Region Of Interest (ROI) rectangle annotator object.
//
// $Id$
//----------------------------------------------------------------------------

#include <ossimGui/RoiRectAnnotator.h>
#include <ossimGui/ImageScrollView.h>
#include <ossim/base/ossimROIEvent.h>
#include <ossim/base/ossimIrect.h>
#include <QtGui/QPainter>
#include <QtCore/QEvent>
#include <QPoint>
#include <QRect>
#include <QRectF>

ossimGui::RoiRectAnnotator::RoiRectAnnotator()
   : ossimListenerManager(),
     m_widget(0),
     m_enableFlag(true),
     m_roiLeftPressedFlag(false),
     m_roiMiddlePresedFlag(false),
     m_roiPressStart(0,0),
     m_penColor(Qt::white),
     m_points(2)
{
}

ossimGui::RoiRectAnnotator::~RoiRectAnnotator()
{
   // cout << "~RoiRectAnnotator() entered..." << endl;
}

void ossimGui::RoiRectAnnotator::setImageWidget(ossimGui::ImageScrollView* widget)
{
   if (m_widget)
   {
      disconnect(m_widget, 0, 0, 0);
   }
   m_widget = widget;
   
   // Connect the widget's paintYourGraphics signal to our paint slot.
   connect(widget, SIGNAL( paintYourGraphics(QPainter*, const QRectF&) ),
           this, SLOT( paint(QPainter*, const QRectF&) ) );

   connect( m_widget, SIGNAL( mousePress(QMouseEvent*) ),
            this, SLOT( mousePress(QMouseEvent*) ) );   

   connect( m_widget, SIGNAL( mouseMove(QMouseEvent*) ),
            this, SLOT( mouseMove(QMouseEvent*) ) );   

   connect( m_widget, SIGNAL( mouseRelease(QMouseEvent*) ),
            this, SLOT( mouseRelease(QMouseEvent*) ) ); 
}

void ossimGui::RoiRectAnnotator::paint(QPainter* p, const QRectF & rect)
{
   if (m_enableFlag)
   {
      // Pass it on to the derived class.
      paintAnnotation(p, rect);
   }
}

void ossimGui::RoiRectAnnotator::enablePaint()
{
   m_enableFlag = true;
}

void ossimGui::RoiRectAnnotator::disablePaint()
{
   m_enableFlag = false;
}

void ossimGui::RoiRectAnnotator::setRoiRect(const ossimIrect& rect)
{
   if (m_points.size() != 2)
   {
      m_points.resize(2);
   }

   m_points[0] = rect.ul();
   m_points[1] = rect.lr();
   refresh();
}

ossimIrect ossimGui::RoiRectAnnotator::getRoiRect() const
{
   if (m_points.size() == 2 && m_widget)
   {
      // Sort the points.
      ossim_int32 ulx = (m_points[0].x < m_points[1].x) ?
         m_points[0].x : m_points[1].x;
      
      ossim_int32 uly = (m_points[0].y < m_points[1].y) ?
         m_points[0].y : m_points[1].y;
      
      ossim_int32 lrx = (m_points[1].x > m_points[0].x) ?
         m_points[1].x : m_points[0].x;
      
      ossim_int32 lry = (m_points[1].y > m_points[0].y) ?
         m_points[1].y : m_points[0].y;
      
      return ossimIrect(ulx, uly, lrx, lry);
   }
   
   return ossimIrect(0,0,0,0);
}

void ossimGui::RoiRectAnnotator::mousePress(QMouseEvent* e)
{
   //---
   // On left click:
   // - start rectangle
   // 
   // On middle click:
   // - change rectangle color to green
   //---

   if ( e )
   {
      Qt::MouseButton button = e->button();
      
      if (button == Qt::LeftButton)
      {
         qPtToIpt( e->pos(), m_roiPressStart );
         m_roiLeftPressedFlag = true;
         // m_points[0] = m_roiPressStart;
      }
      else if (button == Qt::MidButton)
      {
         // Change the color from white to green.
         m_penColor = Qt::green;
         qPtToIpt( e->pos(), m_roiPressStart );
         m_roiMiddlePresedFlag = true;
      }
      // cout << "m_roiPressStart: " << m_roiPressStart << endl;
   }
   
} // End: mousePress( ... )

void ossimGui::RoiRectAnnotator::mouseMove(QMouseEvent* e)
{
   //---
   // On left move:
   // - grow rectangle
   // 
   // On middle move:
   // - shift existing rectangle by the movement.
   //---

   if ( e )
   {
      // Qt::MouseButton button = e->button();

      if (m_roiLeftPressedFlag)
      {
         // Check for some travel to avoid resizing roi on a simple click.
         ossimIpt ipt;
         qPtToIpt(e->pos(), ipt );
         ossim_int32 x_delta = abs(m_roiPressStart.x - ipt.x);
         ossim_int32 y_delta = abs(m_roiPressStart.y - ipt.y);
         if (x_delta > 4 || y_delta > 4)
         {
            if (m_points[0] != m_roiPressStart)
            {
               m_points[0] = m_roiPressStart;
            }
            qPtToIpt( e->pos(), m_points[1]);
            refresh();
         }
      }
      else if (m_roiMiddlePresedFlag)
      {
         // move the existing rectangle.
         ossimIpt pt;
         qPtToIpt( e->pos(), pt );
         ossimIpt shift = (pt - m_roiPressStart);
         m_points[0] += shift;
         m_points[1] += shift;
         m_roiPressStart = pt; // evt->pos();
         refresh();
      }
   }
   
} // End: mouseMove( ... )

void ossimGui::RoiRectAnnotator::mouseRelease(QMouseEvent* e)
{
   //--
   // On middle release:
   // - change rectangle color back to white
   //---

   if ( e )
   {
      Qt::MouseButton button = e->button();
      
      ossimROIEvent roiEvent;
      roiEvent.setEventType(ossimROIEvent::OSSIM_RECTANGLE_ROI);
      
      if (button == Qt::LeftButton)
      {
         // Check for some travel to avoid resizing roi on a simple click.
         ossimIpt ipt;
         qPtToIpt(e->pos(), ipt );
         ossim_int32 x_delta = abs(m_roiPressStart.x - ipt.x);
         ossim_int32 y_delta = abs(m_roiPressStart.y - ipt.y);
         if (x_delta > 4 || y_delta > 4)
         {
            qPtToIpt( e->pos(), m_points[1] ); //  = evt->getPoint();
            refresh();
         }
         else
         {
            m_points[1] = m_points[0];
         }
         m_roiLeftPressedFlag = false;
      }
      else if (button == Qt::MidButton)
      {
         // Change the color from green to white.
         m_penColor = Qt::white;
         
         // move the existing rectangle.
         ossimIpt ipt;
         qPtToIpt( e->pos(), ipt );
         ossimIpt shift = ( ipt - m_roiPressStart );
         m_points[0] += shift;
         m_points[1] += shift;
         m_roiMiddlePresedFlag = false;
         refresh();
         
         // Set the moving flag in the event.
         roiEvent.setMovingFlag(true);
      }
      
      ossimIrect r(m_points[0], m_points[1]);
      roiEvent.setRect(r);
      fireEvent(roiEvent);
   }
}

void ossimGui::RoiRectAnnotator::paintAnnotation( QPainter* p, const QRectF& rect )
{
   if ( p && (m_points.size() >= 2) )
   {
      QRect r = rect.toRect();
      
      ossimIrect clipRect(r.x(), r.y(), r.x() + r.width() - 1, r.y() + r.height() - 1);
      if (clipRect.intersects(getRoiRect()))
      {
         p->save();
         
         QRect r;
         r.setCoords(m_points[0].x, m_points[0].y,
                     m_points[1].x, m_points[1].y);
         p->setPen(m_penColor);
         p->drawRect(r);

         p->restore();  // Every QPainter::save must have corresponding restore.
      }
   }
}

void ossimGui::RoiRectAnnotator::setPoints(const std::vector<ossimIpt>& pts)
{
   m_points = pts;
}

void ossimGui::RoiRectAnnotator::clear()
{
   m_points.clear();
}

void ossimGui::RoiRectAnnotator::getPoints(std::vector<ossimIpt>& pts) const
{
   pts = m_points;
}

void ossimGui::RoiRectAnnotator::refresh()
{
   if ( m_widget )
   {
      QWidget* viewport = m_widget->viewport();
      if ( viewport )
      {
         viewport->update();
      }
   }
}

void ossimGui::RoiRectAnnotator::qPtToIpt( const QPoint& qpt, ossimIpt& ipt ) const
{
   if ( m_widget )
   {
      QPointF scene = m_widget->mapToScene(qpt);
      ipt.x = scene.x();
      ipt.y = scene.y();
   }
}

// Hidden from use.
ossimGui::RoiRectAnnotator::RoiRectAnnotator(
   const ossimGui::RoiRectAnnotator&)
   : ossimListenerManager()
{}

// Hidden from use.
ossimGui::RoiRectAnnotator& ossimGui::RoiRectAnnotator::operator=(
   const ossimGui::RoiRectAnnotator&)
{
   return *this;
}
