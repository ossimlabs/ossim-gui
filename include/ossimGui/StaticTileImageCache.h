//*******************************************************************
// Author: Garrett Potts (gpotts@imagelinks.com)
//*************************************************************************
// $Id$
#ifndef ossimGuiStaticTileImageCache_HEADER
#define ossimGuiStaticTileImageCache_HEADER
#include <QtGui/QImage>
#include <vector>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossimGui/Export.h>
#include <OpenThreads/Mutex>
namespace ossimGui
{
   class OSSIMGUI_DLL StaticTileImageCache :public ossimReferenced
   {
   public:
      StaticTileImageCache(const ossimIpt& tileSize=ossimIpt(0,0));
      virtual ~StaticTileImageCache();
      
      QImage& getCache();
      const QImage& getCache()const;
      const ossimIrect& getRect()const;
      const ossimIrect& getActualRect()const;
      
      /*!
       * Will invalidate the entire cache and set the image cache to blank
       */
      void flush();
      
      /*!
       * Will determine the til that is associated with the pt
       * and will invalidate that tile and blank out that portion of the image
       */
      void flush(const ossimIpt& pt);
      
      /*!
       * Will invalidate all tiles that fall within the defined rect
       */
      void flush(const ossimIrect& rect);
      
      /*!
       * This will do a non destructive resize or translate
       * of the cache.
       */
      void setRect(const ossimIrect& newRect);
      
      /*!
       * Will return true or false if the tile rectangle underneath
       * the passed in point is valid.
       */
      bool isValid(const ossimIpt& pt)const;
      
      /*!
       * Will return true or false if the tile rectangle underneath
       * the passed in point is valid.
       */
      ossim_int32 getTileIndex(const ossimIpt& origin)const;
      ossim_int32 getTileIndex(ossim_int32 x,
                               ossim_int32 y)const;
      /*!
       * will return the tile rectangle at the point.
       */
      ossimIrect getTileRect(const ossimIpt& pt)const;
      
      /*!
       * Given the passed in point it will resize QImage and copy the tile.
       * Note the image object will have its offset X, Y set to be the upper left
       * coordinate of the tile being copied.
       */
      bool getTile(const ossimIpt& pt,
                   QImage& image)const;
      
      ossimIpt getTileOrigin(const ossimIpt& pt)const;
      /*!
       * Will use the offset method of the passed in image and
       * copy a sub rect of the cache out.  Note:  This will not resize
       * the image.  It keeps it the same size as the passed in image.
       */
      void getSubImage(QImage& image)const;
      
      /*!
       * sets the new tile size.  The current implementation
       * will invalidate the entire cache.
       */
      void setTileSize(const ossimIpt& tileSize);
      
      /*!
       * Returns the current tile size used in the cache.
       */
      const ossimIpt& getTileSize()const;
      
      /*!
       * The passed in image can cover several tiles but this image object
       * must be on a tile boundary in order for it to succeed.
       */
      bool addTile(const QImage& image);
      
      /*!
       * Will use the clip rect to paint the tiles.  If one is not defined it will
       * check to see if there is a viewport rect and paint that rect.
       */
      void paintTiles(QPainter* p);
      
      static ossim_int32 computeTileId(const ossimIpt& origin,
                                       const ossimIrect& tileBounderyRect,
                                       const ossimIpt&   tileSize);
      
      bool nextInvalidTile(ossimIrect& rect)const;
      bool hasInvalidTiles()const;
      
   protected:
      QImage* m_cache;
      ossimIrect m_cacheRect;
      ossimIrect m_actualRect;
      ossimIpt m_tileSize;
      std::vector<bool> m_validTileArray;
      ossimIpt          m_numberOfTiles;
      mutable OpenThreads::Mutex m_mutex;
      
      ossim_int32 getTileIndex(const ossimIrect& rect,
                               const ossimIpt& numberOfTiles,
                               const ossimIpt& origin)const;
      
      ossim_int32 getTileIndex(const ossimIrect& rect,
                               const ossimIpt& numberOfTiles,
                               ossim_int32 x,
                               ossim_int32 y)const;
      
      
   };
}   
#endif
