#include <ossimGui/MetricOverlay.h>
#include <ossimGui/MarkPoint.h>
#include <ossimGui/IvtGeomTransform.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QtGui>
#include <QtGui/QGraphicsItem>
#include <iostream>


ossimGui::MetricOverlay::MetricOverlay(const ossimString& overlayId, QGraphicsScene* scene)
: OverlayBase(overlayId, scene),
  m_currentId("NS")
{}

void ossimGui::MetricOverlay::reset()
{
   QList<QGraphicsItem*> items = m_scene->items();
   for (int i=0; i<items.size(); ++i)
   {
         m_scene->removeItem(items[i]);
   }

   m_isActive = false;
   m_currentId = "NS";
}

ossim_uint32 ossimGui::MetricOverlay::getNumPoints()const
{
   return m_scene->items().size();
}

void ossimGui::MetricOverlay::addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt)
{
   addPoint(scenePt, imagePt, m_currentId);
}

void ossimGui::MetricOverlay::addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimString& id)
{
   // Check for duplicate (changing position of point already added)
   removePoint(id);

   // Add point to scene
   ossimGui::MarkPoint* pt = new ossimGui::MarkPoint(scenePt, imagePt, m_overlayId, id);
   m_scene->addItem(pt);

   // Notify MultiImageDialog
   emit pointActivated(id);
}

// TODO not really needed for this class
void ossimGui::MetricOverlay::togglePointActive(const ossimString& id)
{
   ossimGui::MarkPoint* point = getMarkPoint(id);
   bool currentlyActive = false;

   if (point != 0)
   {
      // Toggle point in layer map
      currentlyActive = point->isUsable();
      point->setUsable(!currentlyActive);
   }

   // Notify MultiImageDialog
   if (currentlyActive)
      emit pointDeactivated(id);
   else
      emit pointActivated(id);

}


void ossimGui::MetricOverlay::removePoint(const ossimString& id)
{
   QList<QGraphicsItem*> items = m_scene->items();
   for (int i=0; i<items.size(); ++i)
   {
      ossimString idFromScene = items[i]->data(AnnotationItem::DATA_ITEM_ID).toString().toUtf8().constData();
      ossimString ovidFromScene = items[i]->data(AnnotationItem::DATA_OVERLAY_ID).toString().toUtf8().constData();
      if (id == idFromScene && ovidFromScene == m_overlayId)
      {
         m_scene->removeItem(items[i]);
      }
   }

   emit pointRemoved(id);
}


bool ossimGui::MetricOverlay::getImgPoint(const ossimString& id,
                                                      ossimDpt& imgPt,
                                                      bool& isActive)
{
   bool isFound = false;

   ossimGui::MarkPoint* point = getMarkPoint(id);

   if (point != 0)
   {
      imgPt = point->getImgPos();
      isActive = point->isUsable();
      isFound = true;
   }

   return isFound;
}


ossimGui::MarkPoint* ossimGui::MetricOverlay::getMarkPoint(const ossimString& id)
{
   ossimGui::MarkPoint* point = 0;

   QGraphicsItem* item = getItem(id);
   if (item != 0)
   {
      point = dynamic_cast<ossimGui::MarkPoint*>(item);
   }

   return point;
}


void ossimGui::MetricOverlay::setVisible(const bool& visible)
{
   QList<QGraphicsItem*> items = m_scene->items();
   for (int i=0; i<items.size(); ++i)
   {
      items[i]->setVisible(visible);
   }
}


void ossimGui::MetricOverlay::setView(ossimRefPtr<IvtGeomTransform> ivtg)
{
   QList<QGraphicsItem*> items = m_scene->items();
   for (int i=0; i<items.size(); ++i)
   {
      ossimGui::MarkPoint* thisPoint = dynamic_cast<ossimGui::MarkPoint*>(items[i]);
      if (thisPoint)
      {
         ossimDpt imgPos = thisPoint->getImgPos();
         ossimDpt viewPos;
         ivtg->imageToView(imgPos, viewPos);
         thisPoint->setPos(viewPos.x, viewPos.y);
      }
   }
}
