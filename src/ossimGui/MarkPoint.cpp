#include <ossimGui/MarkPoint.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <QtGui/QToolTip>
#include <iostream>


ossimGui::MarkPoint::MarkPoint(const ossimDpt& scenePos,
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

   // Construct symbol
   m_ver.setLine(0.0, ymax, 0.0, ymin);
   m_hor.setLine(xmin, 0.0, xmax, 0.0);
   m_rect.setRect(xmin+2, ymin+2, m_len-4, m_len-4);

   m_pen.setColor(Qt::green);
   m_pen.setCapStyle(Qt::RoundCap);
   m_pen.setWidth(0);

   // QGraphicsItem::data
   setData(DATA_ITEM_ID, m_id.c_str());
   setData(DATA_OVERLAY_ID, m_overlayId.c_str());
}

void ossimGui::MarkPoint::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
   m_savedPen = m_pen;
   m_pen.setColor(Qt::cyan);
   update();
}

void ossimGui::MarkPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
   m_pen = m_savedPen;
   update();
}

QRectF ossimGui::MarkPoint::boundingRect() const
{
   return QRectF(-m_len/2-1, -m_len/2-1, m_len+2, m_len+2);
}

void ossimGui::MarkPoint::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
   painter->setPen(m_pen);
   painter->drawLine(m_ver);
   painter->drawLine(m_hor);

   painter->setPen(m_annPen);
   painter->drawRect(m_rect);

   painter->setPen(m_pen);
}
