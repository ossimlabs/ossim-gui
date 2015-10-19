//*******************************************************************
// 
//
//
// Author: Garrett Potts 
//
//*************************************************************************
// $Id$
#include <ossimGui/StaticTileImageCache.h>
#include <QtGui/QImage>
#include <OpenThreads/ScopedLock>

ossimGui::StaticTileImageCache::StaticTileImageCache(const ossimIpt& tileSize)
   :m_tileSize(tileSize)
{
   if(m_tileSize.x <=0)
   {
      m_tileSize.x = 64;
   }
   if(m_tileSize.y <=0)
   {
      m_tileSize.y = 64;
   }
   m_cache = new QImage(m_tileSize.x,
                        m_tileSize.y,
                        QImage::Format_RGB32);
   m_validTileArray.resize(1);
   m_cache->fill(0);
   m_actualRect = ossimIrect(0,0,m_tileSize.x-1, m_tileSize.y-1);
   m_cacheRect = m_actualRect;
   m_numberOfTiles.x = 1;
   m_numberOfTiles.y = 1;
   m_validTileArray[0] = false;
}
ossimGui::StaticTileImageCache::~StaticTileImageCache()
{
   if(m_cache)
   {
      delete m_cache;
      m_cache = 0;
   }
}

QImage& ossimGui::StaticTileImageCache::getCache()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return *m_cache;
}

const QImage& ossimGui::StaticTileImageCache::getCache()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return *m_cache;
}

void ossimGui::StaticTileImageCache::flush()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   std::fill(m_validTileArray.begin(),
             m_validTileArray.end(),
             false);
}

void ossimGui::StaticTileImageCache::flush(const ossimIpt& pt)
{
   flush(ossimIrect(pt.x,
                    pt.y,
                    pt.x,
                    pt.y));
}


void ossimGui::StaticTileImageCache::flush(const ossimIrect& rect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   if(rect == getRect())
   {
      std::fill(m_validTileArray.begin(),
                m_validTileArray.end(),
                false);
   }
   else
   {
      int x = 0;
      int y = 0;
      int upperX = 0;
      int upperY = 0;
      ossimIrect tempRect = rect;
      tempRect.stretchToTileBoundary(m_tileSize);

      upperX = tempRect.lr().x;
      upperY = tempRect.lr().y;
      for(y = tempRect.ul().y; y < upperY; y+=m_tileSize.y)
      {
         for(x = tempRect.ul().x; x < upperX; x+=m_tileSize.x)
         {
            ossim_int32 idx = getTileIndex(m_cacheRect, m_numberOfTiles, x, y);
            if(idx >= 0)
            {
               m_validTileArray[idx] = false; 
            }
         }
      }
   }
}

const ossimIrect& ossimGui::StaticTileImageCache::getRect()const
{
   /*
   QPoint pt = m_cache->offset();
   
   return ossimIrect( pt.x(),
                      pt.y(),
                      pt.x() + m_cache->width() - 1,
                      pt.y() + m_cache->height() - 1);
    */
   
   return m_cacheRect;
}

const ossimIrect& ossimGui::StaticTileImageCache::getActualRect()const
{
   return m_actualRect;
}

void ossimGui::StaticTileImageCache::setRect(const ossimIrect& newRect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossimIrect tempRect = newRect;
   m_actualRect = newRect;
   tempRect.stretchToTileBoundary(m_tileSize);

   ossimIrect currentRect = m_cacheRect;

   m_cacheRect = tempRect;
   if(currentRect != tempRect)
   {
      if(!currentRect.intersects(tempRect))
      {
         if((currentRect.width()  != tempRect.width()) ||
            (currentRect.height() != tempRect.height()))
         {
            delete m_cache;
            m_cache = new QImage(tempRect.width(),
                                 tempRect.height(),
                                 QImage::Format_RGB32);
         }
         m_cache->fill(0);
         m_cache->setOffset(QPoint(tempRect.ul().x,
                                   tempRect.ul().y));
         currentRect = m_cacheRect;
         m_numberOfTiles.x = currentRect.width()/m_tileSize.x;
         m_numberOfTiles.y = currentRect.height()/m_tileSize.y;
         m_validTileArray.resize(m_numberOfTiles.x*m_numberOfTiles.y);
         std::fill(m_validTileArray.begin(),
                   m_validTileArray.end(),
                   false);
      }
      else
      {
         ossimIrect intersectionRect = currentRect.clipToRect(tempRect);
         ossimIpt offset = tempRect.ul() - currentRect.ul();
         
         ossimIpt oldNumberOfTiles = m_numberOfTiles;
         std::vector<bool> oldValidTileArray = m_validTileArray;
         ossimIrect  oldRect = currentRect;
         
         *m_cache = m_cache->copy(offset.x,
                                  offset.y,
                                  tempRect.width(),
                                  tempRect.height());
         
         m_cache->setOffset(QPoint(tempRect.ul().x,
                                   tempRect.ul().y));
         
         currentRect = m_cacheRect;
         m_numberOfTiles.x = currentRect.width()/m_tileSize.x;
         m_numberOfTiles.y = currentRect.height()/m_tileSize.y;

         
         m_validTileArray.resize(m_numberOfTiles.x*m_numberOfTiles.y);

         std::fill(m_validTileArray.begin(),
                   m_validTileArray.end(),
                   false);
         
         int x = 0;
         int y = 0;
         int urX = intersectionRect.ur().x;
         int lrY = intersectionRect.lr().y;
         
         for(x = intersectionRect.ul().x; x <= urX; x+=m_tileSize.x)
         {
            for(y = intersectionRect.ul().y; y <= lrY; y+=m_tileSize.y)
            {
               ossim_int32 idx    = getTileIndex(m_cacheRect, m_numberOfTiles, x, y);
               ossim_int32 oldIdx = getTileIndex(oldRect,
                                                 oldNumberOfTiles,
                                                 x,
                                                 y);
               if(idx > -1)
               {
                  if(oldIdx > -1)
                  {
                     m_validTileArray[idx] = oldValidTileArray[oldIdx];
                  }
               }
            }
         }
      }
   }
}

bool ossimGui::StaticTileImageCache::isValid(const ossimIpt& pt)const
{
   ossim_int32 idx = getTileIndex(pt);

   if(idx >= 0)
   {
      return m_validTileArray[idx];
   }

   return false;
}

ossim_int32 ossimGui::StaticTileImageCache::getTileIndex(const ossimIpt& origin)const
{
   return getTileIndex(origin.x,
                       origin.y);
}

ossim_int32 ossimGui::StaticTileImageCache::getTileIndex(ossim_int32 x,
                                                      ossim_int32 y)const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return getTileIndex(m_cacheRect, m_numberOfTiles, x, y);
}

bool ossimGui::StaticTileImageCache::getTile(const ossimIpt& pt,
                                      QImage& image)const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   bool result = false;
   ossimIpt tileOrigin = getTileOrigin(pt);
   ossimIrect cacheRect = getRect();

   if((image.width() != m_tileSize.x)||
      (image.height() != m_tileSize.y))
   {
      image = QImage(m_tileSize.x,
                     m_tileSize.y,
                     QImage::Format_RGB32);
   }
   if(cacheRect.pointWithin(tileOrigin))
   {
      ossimIpt delta(tileOrigin.x - cacheRect.ul().x,
                     tileOrigin.y - cacheRect.ul().y);

      if((delta.x >= 0)&&(delta.y >= 0))
      {
         image = m_cache->copy(tileOrigin.x - cacheRect.ul().x,
                               tileOrigin.y - cacheRect.ul().y,
                               m_tileSize.x,
                               m_tileSize.y);
	 ossim_int32 idx = getTileIndex(m_cacheRect, m_numberOfTiles, pt);
	 if(idx >=0)
	   {
	     result = m_validTileArray[idx];
	   }
      }
      else
      {
         image.fill(0);
      }
   }
   else
   {
      image.fill(0);
   }

   return result;
}

ossimIpt ossimGui::StaticTileImageCache::getTileOrigin(const ossimIpt& pt)const
{
   ossimIpt tempPt;

   if(pt.x < 0)
   {
      tempPt.x = pt.x - (m_tileSize.x-1);
   }
   else
   {
      tempPt.x = pt.x + (m_tileSize.x-1);
   }
   if(pt.y < 0)
   {
      tempPt.y = pt.y - (m_tileSize.y-1);
   }
   else
   {
      tempPt.y = pt.y + (m_tileSize.y-1);
   }

   return ossimIpt((tempPt.x/m_tileSize.x)*m_tileSize.x,
                   (tempPt.y/m_tileSize.y)*m_tileSize.y);
}

void ossimGui::StaticTileImageCache::getSubImage(QImage& image)const
{
   QPoint ulSubImage(image.offset().x(),
                     image.offset().y());

   QPoint ulCache(m_cache->offset().x(),
                  m_cache->offset().y());

   image = m_cache->copy(ulSubImage.x() - ulCache.x(),
                         ulSubImage.y() - ulCache.y(),
                         image.width(),
                         image.height());
}

void ossimGui::StaticTileImageCache::setTileSize(const ossimIpt& tileSize)
{
   flush();
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   m_tileSize = tileSize;
   ossimIrect currentRect = m_cacheRect;
   m_numberOfTiles.x = currentRect.width()/m_tileSize.x;
   m_numberOfTiles.y = currentRect.height()/m_tileSize.y;
}

const ossimIpt& ossimGui::StaticTileImageCache::getTileSize()const
{
   return m_tileSize;
}

bool ossimGui::StaticTileImageCache::addTile(const QImage& image)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   bool result = false;
   
   ossimIrect tileRect(image.offset().x(),
                       image.offset().y(),
                       image.offset().x() + (image.width()-1),
                       image.offset().y() + (image.height()-1));
   ossimIrect cacheRect = m_cacheRect;
   
   if(tileRect.intersects(cacheRect))
   {
      ossimIrect clipRect = tileRect.clipToRect(cacheRect);
      
      // now fill the clipped rect
      ossim_uint32 srcY = clipRect.ul().y-tileRect.ul().y;
      ossim_uint32 srcX = clipRect.ul().x-tileRect.ul().x;
      ossim_uint32 destY = clipRect.ul().y-cacheRect.ul().y;
      ossim_uint32 destX = clipRect.ul().x-cacheRect.ul().x;
      ossimIpt offset = tileRect.ul() - cacheRect.ul();
      ossim_uint32 x,y;
      for(y = 0; y < clipRect.height(); ++y)
      {
         ossim_uint32* cachePtr = ((ossim_uint32*)m_cache->scanLine(y+destY))+destX;
         ossim_uint32* tilePtr  = ((ossim_uint32*)image.scanLine(y+srcY))+srcX;
         for(x = 0; x < clipRect.width(); ++x)
         {
            *cachePtr = *tilePtr;
            ++cachePtr;
            ++tilePtr;
         }
      }
      ossimIpt tilePoint(tileRect.ul());

      for(y = 0; y < tileRect.height(); y+=m_tileSize.y)
      {
         tilePoint.x = tileRect.ul().x;
         for(x = 0; x < tileRect.width(); x+=m_tileSize.x)
         {
            ossim_int32 idx = getTileIndex(m_cacheRect, m_numberOfTiles, tilePoint);
            if(idx>=0)
            {
               m_validTileArray[idx] = true;
            }
            tilePoint.x+= m_tileSize.x;
         }
         tilePoint.y+=m_tileSize.y;
      }
      result = true;      
   }
   
   return result;
}

void ossimGui::StaticTileImageCache::paintTiles(QPainter* /*p*/)
{
   
}

ossim_int32 ossimGui::StaticTileImageCache::getTileIndex(const ossimIrect& rect,
                                                      const ossimIpt& numberOfTiles,
                                                      const ossimIpt& origin)const
{
   return getTileIndex(rect,
                       numberOfTiles,
                       origin.x,
                       origin.y);
}

ossim_int32 ossimGui::StaticTileImageCache::getTileIndex(const ossimIrect& rect,
                                                      const ossimIpt& numberOfTiles,
                                                      ossim_int32 x,
                                                      ossim_int32 y)const
{
   ossimIpt ul = rect.ul();
   ossimIpt delta = ossimIpt(x,y) - ul;

   if((delta.x < 0) ||
      (delta.y < 0) ||
      (delta.x >= (int)rect.width())||
      (delta.y >= (int)rect.height()))
   {
      return -1;
   }
   delta.x /= m_tileSize.x;
   delta.y /= m_tileSize.y;

   return delta.y*numberOfTiles.x + delta.x;
}

ossim_int32 ossimGui::StaticTileImageCache::computeTileId(const ossimIpt& origin,
						       const ossimIrect& tileBounderyRect,
						       const ossimIpt&   tileSize)
{
  ossim_uint32 numberOfTilesX = tileBounderyRect.width()/tileSize.x;
  
  ossimIpt ul = tileBounderyRect.ul();
  ossimIpt delta = origin - ul;
  
  if((delta.x < 0) ||
     (delta.y < 0) ||
     (delta.x >= (int)tileBounderyRect.width())||
     (delta.y >= (int)tileBounderyRect.height()))
    {
      return -1;
    }
  delta.x /= tileSize.x;
  delta.y /= tileSize.y;
  
  return delta.y*numberOfTilesX + delta.x;
}

bool ossimGui::StaticTileImageCache::nextInvalidTile(ossimIrect& rect)const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   ossimIpt origin = m_cacheRect.ul();
   for(idx = 0; idx < m_validTileArray.size(); ++idx)
   {
      if(!m_validTileArray[idx])
      {
         ossim_uint32 yOffset = idx/m_numberOfTiles.x;
         ossim_uint32 xOffset = idx%m_numberOfTiles.x;
         
         ossimIpt ul(origin.x + xOffset*m_tileSize.x, origin.y + yOffset*m_tileSize.y);
         rect = ossimIrect(ul.x, ul.y, ul.x + m_tileSize.x-1, ul.y + m_tileSize.y -1);
         
         return true;
      }
   }
   return false;
}

bool ossimGui::StaticTileImageCache::hasInvalidTiles()const
{
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_validTileArray.size(); ++idx)
   {
      if(!m_validTileArray[idx])
      {
         return true;
      }
   }
   
   return false;
}


