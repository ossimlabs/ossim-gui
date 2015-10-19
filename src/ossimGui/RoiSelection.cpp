#include <ossimGui/RoiSelection.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <QtGui/QToolTip>
#include <iostream>


ossimGui::RoiSelection::RoiSelection(const ossimDpt& scenePos,
                                     const ossimDpt& imagePos,
                                     const ossimDpt& widHgt,
                                     const ossimString& overlayId,
                                     const ossimString& rid)
:
AnnotationItem(overlayId, rid)
{
   setAcceptHoverEvents(true);

   m_scnPos[0] = scenePos;
   m_imgPos[0] = imagePos;

   // Construct rectangle
   m_rect.setRect(scenePos.x, scenePos.y, widHgt.x, widHgt.y);

   m_pen.setColor(Qt::green);
   m_pen.setCapStyle(Qt::RoundCap);
   m_pen.setWidth(1);

   // QGraphicsItem::data
   setData(DATA_ITEM_ID, m_id.c_str());
   setData(DATA_OVERLAY_ID, m_overlayId.c_str());
}

void ossimGui::RoiSelection::drag(const ossimDpt& scenePos, const ossimDpt& imagePos)
{
   if (scenePos.y > m_scnPos[0].y)
      m_rect.setBottom(scenePos.y);
   else
      m_rect.setTop(scenePos.y);

   if (scenePos.x > m_scnPos[0].x)
      m_rect.setRight(scenePos.x);
   else
      m_rect.setLeft(scenePos.x);

   m_scnPos[1] = scenePos;
   m_imgPos[1] = imagePos;

   prepareGeometryChange();
   update();
}


void ossimGui::RoiSelection::redefine(ossimDpt* scenePos, ossimDpt* imagePos)
{
   if (scenePos[0].y > scenePos[1].y)
   {
      m_rect.setBottom(scenePos[0].y);
      m_rect.setTop(scenePos[1].y);
   }
   else
   {
      m_rect.setBottom(scenePos[1].y);
      m_rect.setTop(scenePos[0].y);
   }

   if (scenePos[0].x > scenePos[1].x)
   {
      m_rect.setRight(scenePos[0].x);
      m_rect.setLeft(scenePos[1].x);
   }
   else
   {
      m_rect.setRight(scenePos[1].x);
      m_rect.setLeft(scenePos[0].x);
   }

   m_scnPos[0] = scenePos[0];
   m_scnPos[1] = scenePos[1];
   m_imgPos[0] = imagePos[0];
   m_imgPos[1] = imagePos[1];

   prepareGeometryChange();
   update();
}


ossimIrect ossimGui::RoiSelection::getRectImg() const
{
   // Sort points
   ossim_int32 ulx = (m_imgPos[0].x < m_imgPos[1].x) ?
      m_imgPos[0].x : m_imgPos[1].x;
   
   ossim_int32 uly = (m_imgPos[0].y < m_imgPos[1].y) ?
      m_imgPos[0].y : m_imgPos[1].y;
   
   ossim_int32 lrx = (m_imgPos[1].x > m_imgPos[0].x) ?
      m_imgPos[1].x : m_imgPos[0].x;
   
   ossim_int32 lry = (m_imgPos[1].y > m_imgPos[0].y) ?
      m_imgPos[1].y : m_imgPos[0].y;
   
   return ossimIrect(ulx, uly, lrx, lry);
}


void ossimGui::RoiSelection::hoverEnterEvent( QGraphicsSceneHoverEvent* /* event */ )
{
   m_savedPen = m_pen;
   m_pen.setColor(Qt::cyan);
   update();
}

void ossimGui::RoiSelection::hoverLeaveEvent( QGraphicsSceneHoverEvent* /* event */ )
{
   m_pen = m_savedPen;
   update();
}

QRectF ossimGui::RoiSelection::boundingRect() const
{
   return m_rect;
}

void ossimGui::RoiSelection::paint(QPainter* painter,
                                   const QStyleOptionGraphicsItem* /* option */,
                                   QWidget* /* widget */)
{
   painter->setPen(m_pen);
   painter->drawRect(m_rect);
}
