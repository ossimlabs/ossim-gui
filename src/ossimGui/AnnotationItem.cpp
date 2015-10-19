#include <ossimGui/AnnotationItem.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <QtGui/QToolTip>
#include <iostream>


ossimGui::AnnotationItem::AnnotationItem(const ossimString& overlayId, const ossimString& id)
:
m_id(id),
m_isEnabled(true),
m_overlayId(overlayId)
{
   m_annPen.setColor(Qt::blue);
   m_annPen.setCapStyle(Qt::RoundCap);
   m_annPen.setWidth(0);
}

void ossimGui::AnnotationItem::setID(const ossimString& id)
{
   m_id = id;
}

void ossimGui::AnnotationItem::setUsable(const bool& enable)
{
   m_isEnabled = enable;
   update();
}

void ossimGui::AnnotationItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
   m_annPen.setColor(Qt::cyan);
   update();
}

void ossimGui::AnnotationItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
   m_annPen.setColor(Qt::blue);
   update();
}
