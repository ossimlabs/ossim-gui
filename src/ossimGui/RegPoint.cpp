#include <ossimGui/RegPoint.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <QtGui/QToolTip>
#include <iostream>


ossimGui::RegPoint::RegPoint(const ossimDpt& scenePos,
                             const ossimDpt& imgPos,
                             const ossimString& overlayId,
                             const ossimString& id)
:
AnnotationItem(overlayId, id),
m_imgPos(imgPos),
m_len(12.0)
{
   setAcceptHoverEvents(true);

   setPos(scenePos.x, scenePos.y);

   // Set end points
   qreal xmin = -m_len/2;
   qreal xmax =  m_len/2;
   qreal ymin = -m_len/2;
   qreal ymax =  m_len/2;

   // Construct cross
   m_ver.setLine(0.0, ymax, 0.0, ymin);
   m_hor.setLine(xmin, 0.0, xmax, 0.0);

   m_pen.setColor(Qt::yellow);
   m_pen.setCapStyle(Qt::RoundCap);
   m_pen.setWidth(1);
   m_savedPen = m_pen;

   // QGraphicsItem::data
   setData(DATA_ITEM_ID, m_id.c_str());
   setData(DATA_OVERLAY_ID, m_overlayId.c_str());
}


void ossimGui::RegPoint::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
   QToolTip::showText(event->screenPos(), QString(this->m_id.data()));
   m_savedPen = m_pen;
   m_pen.setColor(Qt::cyan);
   update();
}

void ossimGui::RegPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
   QToolTip::hideText();
   m_pen = m_savedPen;
   update();
}

void ossimGui::RegPoint::setUsable(const bool& enable)
{
   m_isEnabled = enable;

   if (m_isEnabled)
   {
      m_pen.setColor(Qt::yellow);
   }
   else
   {
      m_pen.setColor(Qt::red);
   }

   update();
}

QRectF ossimGui::RegPoint::boundingRect() const
{
   return QRectF(-m_len/2-1, -m_len/2-1, m_len+2, m_len+2);
}

void ossimGui::RegPoint::paint(QPainter* painter,
                               const QStyleOptionGraphicsItem* option,
                               QWidget* widget)
{
   painter->setPen(m_pen);
   painter->drawLine(m_ver);
   painter->drawLine(m_hor);
}
