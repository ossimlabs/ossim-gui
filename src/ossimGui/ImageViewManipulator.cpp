#include <ossimGui/ImageViewManipulator.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/SetViewVisitor.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimRLevelFilter.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <QtCore/QRectF>

namespace ossimGui
{
   ImageViewManipulator::ImageViewManipulator(ImageScrollView* scrollView)
      :m_scrollView(0),
       m_fullResolutionScale(1.0,1.0)
   {
      //m_sceneItemUpdate = new SceneItemUpdate();
      setImageScrollView(scrollView);

   }
   void ImageViewManipulator::setObject(ossimObject* obj)
   {
      m_obj = obj;	
      ossimImageViewTransform* ivt = getObjectAs<ossimImageViewTransform>();
      if(ivt)
      {
         m_fullResolutionScale = ivt->getInputMetersPerPixel();
      }
      if(m_fullResolutionScale.hasNans())
      {
         m_fullResolutionScale = ossimDpt(1.0,1.0);
      }
   }

   void ImageViewManipulator::setImageScrollView(ImageScrollView* scrollView)
   {
      m_fullResolutionScale = ossimDpt(1.0,1.0);
      m_scrollView = scrollView;
      if(m_scrollView)
      {
         m_scrollView->setMouseTracking(true);
      }
   }

   void ImageViewManipulator::initializeToCurrentView()
   {
      if(m_scrollView)
      {
         m_fullResolutionScale = ossimDpt(1.0,1.0);

         ossimTypeNameVisitor visitor("ossimImageRenderer", true);
         m_scrollView->connectableObject()->accept(visitor);


         ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*>(visitor.getObject());
         ossimViewInterface* geomSource = connectable?dynamic_cast<ossimViewInterface*>(connectable):0;
         ossimImageSource* is = connectable?dynamic_cast<ossimImageSource*>(connectable->getInput()):0;

         if(geomSource)
         {
            if(geomSource->getView())
            {
               m_obj = (ossimObject*)(geomSource->getView()->dup());
            }
         }
         if(!is)
         {
            visitor.reset();
            visitor.setTypeName("ossimImageHandler");
            m_scrollView->connectableObject()->accept(visitor);
            is = dynamic_cast<ossimImageSource*>(visitor.getObject());
         }
         bool affineFlag = isAffine();
         if(is)
         {
            ossim_uint32 nLevels = is->getNumberOfDecimationLevels();
            nLevels = nLevels?nLevels:1;
            ossim_float64 nLevelsPower2 = 1<<(nLevels-1);
            ossim_float64 zoomInFactor  = 1<<7;
            ossimRefPtr<ossimImageGeometry> geom = is->getImageGeometry();
            if(!affineFlag&&geom.valid()&&geom->getProjection())
            {
               m_fullResolutionScale = geom->getMetersPerPixel();
               m_fullResolutionScale.x = m_fullResolutionScale.y;
               m_scaleRange.m_min = m_fullResolutionScale.y*(1.0/zoomInFactor);
               m_scaleRange.m_max = m_fullResolutionScale.y*nLevelsPower2;
            }
            else
            {
               m_scaleRange.m_min =1.0/nLevelsPower2;
               m_scaleRange.m_max = zoomInFactor;
            }
         }
         setCommonCenter();
      }
   }
   bool ImageViewManipulator::isAffine()const
   {
      return (getObjectAs<const ossimImageViewAffineTransform>()!=0);
   }

   void ImageViewManipulator::fit()
   {
      ossimDrect inputBounds = m_scrollView->getInputBounds();
      QRect viewportRect = m_scrollView->viewport()->rect();
      ossim_float64 viewportWidth  = viewportRect.width();
      ossim_float64 viewportHeight = viewportRect.height();
      ossimIrect inputRect(inputBounds);
      ossimIrect targetRect(0, 0, static_cast<ossim_int32>(viewportWidth), static_cast<ossim_int32>(viewportHeight));

      fit(inputRect, targetRect);
   }
   void ImageViewManipulator::setFullResScale(const ossimDpt& scale)
   {
      m_fullResolutionScale = scale;
   }
   
   void ImageViewManipulator::fullRes()
   {
      ossimRLevelFilter* rlevelFilter = findRLevelFilter();
      if(isImageMode())
      {
         setImageRLevel(0);
         m_scrollView->zoomAnnotation();
         return;
      }

      ossimImageGeometry* geom = asGeometry();
      if(geom)
      {
         if(geom->getProjection())
         {
            ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
            if(mapProj)
            {
               mapProj->setMetersPerPixel(m_fullResolutionScale);
            }
         }
      }
      else
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            ivat->scale(m_fullResolutionScale.x, m_fullResolutionScale.y);
         }
      }
      setViewToChains();
      m_scrollView->refreshDisplay();
      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::zoomIn(double factor)
   {
      ossimRLevelFilter* rlevelFilter = findRLevelFilter();
      bool modified = false;
      if(!m_scrollView) return;
      if(isImageMode())
      {
         if(rlevelFilter)
         {
            ossim_uint32 current = rlevelFilter->getCurrentRLevel();
            if(current > 0)
            {
               setImageRLevel(current-1);
            }
         }
         m_scrollView->zoomAnnotation();
         return;
      }

      ossimImageGeometry* geom = asGeometry();
      ossimDpt saveCenter = m_centerPoint;
      if(geom)
      {
         if(geom->getProjection())
         {
            ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
            mpp.x/=factor;
            mpp.y/=factor;
            ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
            if(mapProj)
            {
               if(m_scaleRange.isValid())
               {
                  if(mpp.y>m_scaleRange.m_min)
                  {
                     mapProj->setMetersPerPixel(mpp);
                     modified = true;
                  }
               }
               else
               {
                  mapProj->setMetersPerPixel(mpp);
                  modified = true;
               }
            }
         }
      }
      else
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            ossimDpt scale = ivat->getScale();
            ossimDpt factorScale(scale.x*factor,scale.y*factor);
            if(m_scaleRange.isValid())
            {
               if(factorScale.x < m_scaleRange.m_max&&
                  factorScale.y < m_scaleRange.m_max)
               {
                  ivat->scale(factorScale.x,factorScale.y);
                  modified = true;
               }
            }
            else
            {
               ivat->scale(factorScale.x,factorScale.y);
               modified = true;
            }
         }
      }
      m_centerPoint = saveCenter;
      if(modified)
      {
         setViewToChains();
         m_scrollView->refreshDisplay();
      }
      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::zoomOut(double factor)
   {
      ossimRLevelFilter* rlevelFilter = findRLevelFilter();
      if(!m_scrollView) return;
      if(isImageMode())
      {
         if(rlevelFilter)
         {
            ossim_uint32 current = rlevelFilter->getCurrentRLevel();
            ossim_uint32 maximum = maxImageRLevel();
            if(current < maximum)
            {
               setImageRLevel(current+1);
            }
         }
         m_scrollView->zoomAnnotation();
         return;
      }

      bool modified = false;
      ossimImageGeometry* geom = asGeometry();
      ossimDpt saveCenter = m_centerPoint;
      if(geom)
      {
         if(geom->getProjection())
         {
            ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
            mpp.x*=factor;
            mpp.y*=factor;
            ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
            if(mapProj)
            {
               if(m_scaleRange.isValid())
               {
                  if(mpp.y <= m_scaleRange.m_max)
                  {
                     mapProj->setMetersPerPixel(mpp);
                     modified = true;
                  }
               }
               else
               {
                  mapProj->setMetersPerPixel(mpp);
                  modified = true;
               }
            }
         }
      }
      else
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            ossimDpt scale = ivat->getScale();
            ossimDpt factorScale(scale.x/factor,scale.y/factor);
            if(m_scaleRange.isValid())
            {
               if( factorScale.x >= m_scaleRange.m_min &&
                   factorScale.y >= m_scaleRange.m_min )
               {
                  ivat->scale(factorScale.x,factorScale.y);
                  modified = true;
               }
            }
            else
            {
               ivat->scale(factorScale.x,factorScale.y);
               modified = true;
            }
         }
      }
      m_centerPoint = saveCenter;
      if(modified)
      {
         setViewToChains();
         m_scrollView->refreshDisplay();
      }
      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::fit(const ossimIrect& inputRect,
                                  const ossimIrect& targetRect)
   {
      if(isImageMode())
      {
         ossimImageHandler* handler = findImageHandler();
         ossimRLevelFilter* rlevelFilter = findRLevelFilter();
         if(handler && rlevelFilter)
         {
            const ossimIrect fullResRect = handler->getBoundingRect(0);
            ossim_uint32 fitRLevel = 0;
            const ossim_uint32 maximum = maxImageRLevel();
            for(ossim_uint32 level = 0; level <= maximum; ++level)
            {
               ossimIrect levelRect = handler->getBoundingRect(level);
               if(levelRect.hasNans())
               {
                  continue;
               }

               if((levelRect.width() <= targetRect.width()) &&
                  (levelRect.height() <= targetRect.height()))
               {
                  fitRLevel = level;
                  break;
               }

               fitRLevel = level;
            }

            m_centerPoint = fullResRect.midPoint();
            setImageRLevel(fitRLevel);
            m_scrollView->zoomAnnotation();
            return;
         }
      }

      ossimImageGeometry* geom = asGeometry();
      double scaleX = static_cast<double>(inputRect.width())/static_cast<double>(targetRect.width());
      double scaleY = static_cast<double>(inputRect.height())/static_cast<double>(targetRect.height());
      double largestScale = ossim::max(scaleX, scaleY);
      if(geom)
      {
         ossimGpt imageCenterGround;
         geom->localToWorld(inputRect.midPoint(), imageCenterGround);
         if(geom->getProjection())
         {
            ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
            mpp.x *= largestScale;
            mpp.y *= largestScale;
            ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
            if(mapProj)
            {
               mapProj->setMetersPerPixel(mpp);
            }
         }
         m_centerPoint = imageCenterGround;
      }
      else
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            double x = 1.0/largestScale;
            ivat->scale(x, x);
         }
         m_centerPoint = inputRect.midPoint();
      }
      setViewToChains();
      m_scrollView->refreshDisplay();
      m_scrollView->zoomAnnotation();
   }

   ImageScrollView* ImageViewManipulator::getImageScrollView()
   {
      return m_scrollView;
   }
	
   void   ImageViewManipulator::resizeEvent(QResizeEvent* /*event*/)
   {

   }

   void   ImageViewManipulator::scrollContentsBy( int /*dx*/, int /*dy*/ )
   {
      if(m_scrollView)
      {
         setCommonCenter();
      }
   }

   void ImageViewManipulator::keyPressEvent(QKeyEvent* /*event*/, 
                                            bool& consumeEvent)
   {
      consumeEvent = false;
   }

   void ImageViewManipulator::keyReleaseEvent ( QKeyEvent * event, 
                                                bool& consumeEvent )
   {
      switch(event->key())
      {
         case Qt::Key_Plus:
         {
            zoomIn();
            break;
         }
         case Qt::Key_Minus:
         {
            zoomOut();
            break;
         }
         default:
         {
            break;
         }
      }
      consumeEvent = false;
   }

   void ImageViewManipulator::mouseDoubleClickEvent ( QMouseEvent * /*event*/, 
                                                      bool& consumeEvent )
   {
      consumeEvent = false;
   }

   void ImageViewManipulator::mouseMoveEvent ( QMouseEvent * event, 
                                               bool& consumeEvent )
   {
      QPointF pt = m_scrollView->mapToScene(event->pos());
      if(m_leftButtonPressed)
      {
         m_scrollView->emitTracking(ossimDpt(pt.x(), pt.y()));
      }
      consumeEvent = false;
   }

   void ImageViewManipulator::mousePressEvent (QMouseEvent* event,
                                               bool& consumeEvent)
   {
      m_leftButtonPressed = false;
      if(event->buttons() & Qt::LeftButton)
      {
         m_leftButtonPressed = true;
      }
      consumeEvent = false;
      if(m_leftButtonPressed)
      {
         QPointF p = m_scrollView->mapToScene(event->pos());
         m_scrollView->emitTracking(ossimDpt(p.x(), p.y()));
      }
		
   }

   void ImageViewManipulator::mouseReleaseEvent (QMouseEvent* event, 
                                                 bool& consumeEvent )
   {

      QPointF pt = m_scrollView->mapToScene(event->pos());
		
      if(m_scrollView	&& m_leftButtonPressed)
      {
         if(event->modifiers()&Qt::ShiftModifier)
         {
            m_centerPoint = sceneToLocal(ossimDpt(pt.x(), pt.y()));
            m_scrollView->centerOn(pt);
         }
         m_leftButtonPressed = false;
         m_scrollView->emitTracking(ossimDpt(pt.x(), pt.y()));

      } 

      consumeEvent = false;
   }

   void ImageViewManipulator::resizeEvent (QResizeEvent * /*event*/, 
                                           bool& consumeEvent )
   {
      consumeEvent = false;
   }

   void ImageViewManipulator::wheelEvent (QWheelEvent * event, 
                                          bool& consumeEvent )
   {
      consumeEvent = false;

      switch(event->modifiers())
      {
         case Qt::ShiftModifier:
         {
            double factor = 1.0 + fabs(event->angleDelta().y()/500.0);
		    
            if(event->angleDelta().y() > 0)
            {
               zoomIn(factor);
            }
            else 
            {
               zoomOut(factor);
            }
            consumeEvent = true;
            break;
         }
         default:
         {
            consumeEvent = false;
            break;
         }
      }
      QPointF p = m_scrollView->mapToScene(event->pos());
      m_scrollView->emitTracking(ossimDpt(p.x(), p.y()));
   }

   void ImageViewManipulator::enterEvent ( QEvent * /*event*/, 
                                           bool& consumeEvent )
   {
      m_scrollView->setShowTrackCursor(false);
      consumeEvent = false;
   }   

   void ImageViewManipulator::leaveEvent ( QEvent * /*event*/, 
                                           bool& consumeEvent )
   {
      m_scrollView->setShowTrackCursor(true);
      consumeEvent = false;
   }     
   ossimImageGeometry* ImageViewManipulator::asGeometry()
   {
      ossimImageViewProjectionTransform* ivpt = getObjectAs<ossimImageViewProjectionTransform>();
      if(ivpt)
      {
         return dynamic_cast<ossimImageGeometry*>(ivpt->getView());
      }
      
      return getObjectAs<ossimImageGeometry>();
   }

   bool ImageViewManipulator::isImageMode() const
   {
      return (getObjectAs<const ossimImageViewAffineTransform>() != 0) &&
             (findRLevelFilter() != 0);
   }

   ossimImageHandler* ImageViewManipulator::findImageHandler() const
   {
      if(!m_scrollView || !m_scrollView->connectableObject())
      {
         return 0;
      }

      ossimConnectableObject* input =
         dynamic_cast<ossimConnectableObject*>(m_scrollView->connectableObject()->getInput());
      if(!input)
      {
         return 0;
      }

      ossimTypeNameVisitor visitor("ossimImageHandler",
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
      input->accept(visitor);

      return visitor.getObjectAs<ossimImageHandler>(0);
   }

   ossimRLevelFilter* ImageViewManipulator::findRLevelFilter() const
   {
      if(!m_scrollView || !m_scrollView->connectableObject())
      {
         return 0;
      }

      ossimConnectableObject* input =
         dynamic_cast<ossimConnectableObject*>(m_scrollView->connectableObject()->getInput());
      if(!input)
      {
         return 0;
      }

      ossimTypeNameVisitor visitor("ossimRLevelFilter",
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
      input->accept(visitor);

      return visitor.getObjectAs<ossimRLevelFilter>(0);
   }

   ossim_uint32 ImageViewManipulator::maxImageRLevel() const
   {
      ossimImageHandler* handler = findImageHandler();
      if(!handler)
      {
         return 0;
      }

      const ossim_uint32 levelCount = handler->getNumberOfDecimationLevels();
      return levelCount ? (levelCount-1) : 0;
   }

   ossimDpt ImageViewManipulator::imageDecimation(ossim_uint32 rlevel) const
   {
      ossimDpt result(1.0, 1.0);
      ossimImageHandler* handler = findImageHandler();
      if(handler)
      {
         handler->getDecimationFactor(rlevel, result);
      }

      if(result.hasNans() || (result.x == 0.0) || (result.y == 0.0))
      {
         result = ossimDpt(1.0, 1.0);
      }

      return result;
   }

   void ImageViewManipulator::normalizeImageModeTransform()
   {
      ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
      if(!ivat)
      {
         return;
      }

      ossimDpt scale = ivat->getScale();
      if(scale.hasNans() || (scale.x == 0.0) || (scale.y == 0.0))
      {
         return;
      }

      if((scale.x != 1.0) || (scale.y != 1.0))
      {
         ivat->scale(1.0, 1.0);
      }
   }

   void ImageViewManipulator::setImageRLevel(ossim_uint32 rlevel)
   {
      ossimRLevelFilter* rlevelFilter = findRLevelFilter();
      if(!rlevelFilter)
      {
         return;
      }

      normalizeImageModeTransform();
      const ossim_uint32 maximum = maxImageRLevel();
      if(rlevel > maximum)
      {
         rlevel = maximum;
      }

      const ossim_uint32 previous = rlevelFilter->getCurrentRLevel();
      rlevelFilter->setCurrentRLevel(rlevel);
      setViewToChains();
      m_scrollView->refreshDisplay();
   }

   void ImageViewManipulator::setViewToChains()
   {
      if(m_scrollView&&m_scrollView->connectableObject())
      {
         ossimDpt center;
         ossimImageGeometry* geom = asGeometry();
         if(geom)
         {
            geom->worldToLocal(ossimGpt(m_centerPoint.lat, m_centerPoint.lon), center);
         }
         else
         {
            ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
            if(ivat)
            {
               if(!m_centerPoint.hasNans())
               {
                  ossimDpt imagePoint = m_centerPoint;
                  if(isImageMode())
                  {
                     ossimRLevelFilter* rlevelFilter = findRLevelFilter();
                     if(rlevelFilter)
                     {
                        ossimDpt decimation = imageDecimation(rlevelFilter->getCurrentRLevel());
                        imagePoint.x *= decimation.x;
                        imagePoint.y *= decimation.y;
                     }
                  }
                  ivat->imageToView(imagePoint,center);
               }
            }
         }
         SetViewVisitor viewVisitor(m_obj.get());
         viewVisitor.setViewPoint(center);
         m_scrollView->connectableObject()->accept(viewVisitor);

         // keep our locked center point for zooming in and out.
         ossimDpt saveCenter = m_centerPoint;
         viewVisitor.setView();
         m_centerPoint = saveCenter;
      }
   }

   void ImageViewManipulator::setCommonCenter()
   {
      ossimDpt center = m_scrollView->viewportBoundsInSceneSpace().midPoint();

      m_centerPoint = sceneToLocal(center);
   }

   ossimDpt ImageViewManipulator::sceneToLocal(const ossimDpt& scenePoint)
   {
      ossimDpt result;
      result.makeNan();
      ossimImageGeometry* geom = asGeometry();
      if(geom)
      {
         ossimGpt wpt;
         geom->localToWorld(scenePoint, wpt);
         result = wpt;
      }
      else
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            if(!scenePoint.hasNans())
            {
               ivat->viewToImage(scenePoint, result);
               if(isImageMode())
               {
                  ossimRLevelFilter* rlevelFilter = findRLevelFilter();
                  if(rlevelFilter)
                  {
                     ossimDpt decimation = imageDecimation(rlevelFilter->getCurrentRLevel());
                     if((decimation.x != 0.0) && (decimation.y != 0.0))
                     {
                        result.x /= decimation.x;
                        result.y /= decimation.y;
                     }
                  }
               }
            }
         }
      }

      return result;
   }
}
