#include <ossimGui/ImageViewManipulator.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/SetViewVisitor.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageRenderer.h>
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
         //ossimDpt center  = m_scrollView->getInputBounds().midPoint();
         //m_centerPoint = center;

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
      ossimTypeNameVisitor visitor("ossimImageRenderer", true);
      m_scrollView->connectableObject()->accept(visitor);
      ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*>(visitor.getObject());
      ossim_float64 viewportWidth  = m_scrollView->size().width()-16;
      ossim_float64 viewportHeight = m_scrollView->size().height()-16;
      ossimDpt saveCenter = m_centerPoint;
      if(connectable)
      {
         ossimImageSource* inputSource = dynamic_cast<ossimImageSource*>(connectable->getInput());
         ossimImageRenderer* renderer  = dynamic_cast<ossimImageRenderer*>(connectable);

         ossimImageViewAffineTransform* ivat = dynamic_cast<ossimImageViewAffineTransform*>(renderer->getImageViewTransform());
         ossimImageViewProjectionTransform* ivpt = dynamic_cast<ossimImageViewProjectionTransform*>(renderer->getImageViewTransform());

         if(ivpt)
         {
            ossimImageGeometry* iGeom = ivpt->getImageGeometry();
            ossimImageGeometry* vGeom = asGeometry();
            if(iGeom&&vGeom)
            {

               ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(vGeom->getProjection());

               if(mapProj)
               {
                  // ossimDpt savedMpp = mapProj->getMetersPerPixel();
                  ossimDpt mpp = m_fullResolutionScale;
                  mapProj->setMetersPerPixel(m_fullResolutionScale);
                  ossimDrect rect = inputSource->getBoundingRect();
                  std::vector<ossimGpt> gpoints(4);
                  std::vector<ossimDpt> ipoints(4);
                  iGeom->localToWorld(rect.ul(), gpoints[0]);
                  iGeom->localToWorld(rect.ur(), gpoints[1]);
                  iGeom->localToWorld(rect.lr(), gpoints[2]);
                  iGeom->localToWorld(rect.ll(), gpoints[3]);

                  vGeom->worldToLocal(gpoints[0], ipoints[0]);
                  vGeom->worldToLocal(gpoints[1], ipoints[1]);
                  vGeom->worldToLocal(gpoints[2], ipoints[2]);
                  vGeom->worldToLocal(gpoints[3], ipoints[3]);

                  ossimDrect fullBounds = ossimDrect(ipoints);

                  double scaleX = static_cast<double>(fullBounds.width())/static_cast<double>(viewportWidth);
                  double scaleY = static_cast<double>(fullBounds.height())/static_cast<double>(viewportHeight);
                  double largestScale = ossim::max(scaleX, scaleY);
                  mpp.x*=largestScale;
                  mpp.y*=largestScale;


                  mapProj->setMetersPerPixel(mpp);						
               }
            }
         }
         else if(ivat)
         {
            ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
            ossimDrect inputBounds = m_scrollView->getInputBounds();
            double scaleX = static_cast<double>(inputBounds.width())/static_cast<double>(viewportWidth);
            double scaleY = static_cast<double>(inputBounds.height())/static_cast<double>(viewportHeight);
            double largestScale = ossim::max(scaleX, scaleY);
            if(ivat)
            {
               ossimDpt tempCenter;

               double x = 1.0/largestScale;
               double y = x;
               ivat->scale(x,y);
            }

         }
      }

      m_centerPoint = saveCenter;
      setViewToChains();
   }
   void ImageViewManipulator::setFullResScale(const ossimDpt& scale)
   {
      m_fullResolutionScale = scale;
   }
   
   void ImageViewManipulator::fullRes()
   {
      ossimImageGeometry* geom = asGeometry();
	   
      if(geom)
      {
         ossimGpt tempCenter;
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
            ossimDpt tempCenter;
            ivat->scale(m_fullResolutionScale.x, m_fullResolutionScale.y);
         }
      }
      setViewToChains();

      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::zoomIn(double factor)
   {
      bool modified = false;
      if(!m_scrollView) return;
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
                     if(mapProj)
                     {
                        mapProj->setMetersPerPixel(mpp);
                        modified = true;
                     }
                  }
               }
               else
               {
                  mapProj->setMetersPerPixel(mpp);
                  modified = false;
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
      }

      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::zoomOut(double factor)
   {
      if(!m_scrollView) return;
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
      }

      m_scrollView->zoomAnnotation();
   }

   void ImageViewManipulator::fit(const ossimIrect& inputRect,
                                  const ossimIrect& targetRect)
   {
      ossimImageGeometry* geom = asGeometry();
      double scaleX = static_cast<double>(inputRect.width())/static_cast<double>(targetRect.width());
      double scaleY = static_cast<double>(inputRect.height())/static_cast<double>(targetRect.height());
      double largestScale = ossim::max(scaleX, scaleY);
      ossimDpt saveCenter = m_centerPoint;

      if(geom)
      {
         if(geom->getProjection())
         {
            ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
	         
            mpp.x*=largestScale;
            mpp.y*=largestScale;
            ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
            if(mapProj)
            {
               mapProj->setMetersPerPixel(mpp);
            }

         }
      }
      else 
      {
         ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
         if(ivat)
         {
            ossimDpt tempCenter;

            double x = 1.0/largestScale;
            double y = x;
            ivat->scale(x,y);
         }
      }
      m_centerPoint = saveCenter;
      setViewToChains();
		      
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
            double factor = 1.0 + fabs(event->delta()/500.0);
		    
            if(event->delta() > 0)
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
                  ivat->imageToView(m_centerPoint,center);
               }
            }
         }
         SetViewVisitor viewVisitor(m_obj.get());
         viewVisitor.setViewPoint(center);
         m_scrollView->connectableObject()->accept(viewVisitor);

         // just in case if an update causes a change in center let's keep our locked 
         // center point for zooming in and out.
         ossimDpt saveCenter = m_centerPoint;
         viewVisitor.setView();
         m_centerPoint = saveCenter;

         m_scrollView->emitViewChanged();
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
            }
         }
      }

      return result;
   }
}
