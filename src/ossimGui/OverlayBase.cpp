#include <ossimGui/OverlayBase.h>
#include <ossimGui/AnnotationItem.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <iostream>


ossimGui::OverlayBase::OverlayBase(const ossimString& overlayId, QGraphicsScene* scene)
: m_scene(scene),
  m_isActive(false),
  m_overlayId(overlayId)
{}

QGraphicsItem* ossimGui::OverlayBase::getItem(const ossimString& id)
{
   QGraphicsItem* foundItem = 0;

   QList<QGraphicsItem*> items = m_scene->items();
   for (int i=0; i<items.size(); ++i)
   {
      ossimString idFromScene = items[i]->data(AnnotationItem::DATA_ITEM_ID).toString().toUtf8().constData();
      ossimString ovidFromScene = items[i]->data(AnnotationItem::DATA_OVERLAY_ID).toString().toUtf8().constData();
      if (id == idFromScene && ovidFromScene == m_overlayId)
      {
         foundItem = items[i];
      }
   }

   return foundItem;
}