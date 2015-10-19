#include <ossimGui/ViewManipulator.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
ossimGui::ViewManipulator::ViewManipulator()
:m_fullResolutionScale(1.0,1.0)
{
}

ossimGui::ViewManipulator::ViewManipulator(ossimObject* geom)
{
   setObject(geom);
}

void ossimGui::ViewManipulator::setObject(ossimObject* obj)
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

void ossimGui::ViewManipulator::setFullResScale(const ossimDpt& scale)
{
   m_fullResolutionScale = scale;
}

void ossimGui::ViewManipulator::fullRes(ossimDpt& center)
{
   ossimImageGeometry* geom = asGeometry();
   
   if(geom)
   {
      ossimGpt tempCenter;
      if(!center.hasNans())
      {
         geom->localToWorld(center, tempCenter);
      }
      if(geom->getProjection())
      {
         ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
         if(mapProj)
         {
            mapProj->setMetersPerPixel(m_fullResolutionScale);
         }
      }
      if(!center.hasNans())
      {
         geom->worldToLocal(tempCenter, center);
      }
   }
   else 
   {
      ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
      if(ivat)
      {
         ossimDpt tempCenter;
         if(!center.hasNans())
         {
            ivat->viewToImage(center, tempCenter);
         }
         ivat->scale(m_fullResolutionScale.x, m_fullResolutionScale.y);
         if(!center.hasNans())
         {
            ivat->imageToView(tempCenter,center );
         }
      }
   }
}

void ossimGui::ViewManipulator::fit(const ossimDrect& inputRect,
                                    const ossimDrect& targetRect)
{
   ossimImageGeometry* geom = asGeometry();
   double scaleX = inputRect.width()/static_cast<double>(targetRect.width());
   double scaleY = inputRect.height()/static_cast<double>(targetRect.height());
   double largestScale = ossim::max(scaleX, scaleY);
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
         double x = 1.0/largestScale;
         double y = x;
         ivat->scale(x,y);
      }
   }

}

void ossimGui::ViewManipulator::zoomIn(ossimDpt& center, double factor)
{
   ossimImageGeometry* geom = asGeometry();
   if(geom)
   {      
      if(geom->getProjection())
      {
         ossimGpt tempCenter;
         if(!center.hasNans())
         {
            geom->localToWorld(center, tempCenter);
         }
         ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
         mpp.x/=factor;
         mpp.y/=factor;
         ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
         if(mapProj)
         {
            mapProj->setMetersPerPixel(mpp);
         }
         if(!center.hasNans())
         {
            geom->worldToLocal(tempCenter, center);
         }
      }
   }
   else 
   {
      ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
      if(ivat)
      {
         ossimDpt scale = ivat->getScale();
         ossimDpt tempCenter;
         if(!center.hasNans())
         {
            ivat->viewToImage(center, tempCenter);
         }
         
         ivat->scale(scale.x*factor,scale.y*factor);
         if(!center.hasNans())
         {
            ivat->imageToView(tempCenter,center);
         }
      }
   }
}

void ossimGui::ViewManipulator::zoomOut(ossimDpt& center, double factor)
{
   ossimImageGeometry* geom = asGeometry();
   if(geom)
   {      
      if(geom->getProjection())
      {
         ossimGpt tempCenter;
         if(!center.hasNans())
         {
            geom->localToWorld(center, tempCenter);
         }
         ossimDpt mpp = geom->getProjection()->getMetersPerPixel();
         mpp.x*=factor;
         mpp.y*=factor;
         ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
         if(mapProj)
         {
            mapProj->setMetersPerPixel(mpp);
         }
         if(!center.hasNans())
         {
            geom->worldToLocal(tempCenter, center);
         }
      }
   }
   else 
   {
      ossimImageViewAffineTransform* ivat = getObjectAs<ossimImageViewAffineTransform>();
      if(ivat)
      {
         ossimDpt scale = ivat->getScale();
         ossimDpt tempCenter;
         if(!center.hasNans())
         {
            ivat->viewToImage(center, tempCenter);
         }
         
         ivat->scale(scale.x/factor,scale.y/factor);
         if(!center.hasNans())
         {
            ivat->imageToView(tempCenter,center);
         }
      }
   }
}

ossimImageGeometry* ossimGui::ViewManipulator::asGeometry()
{
   ossimImageViewProjectionTransform* ivpt = getObjectAs<ossimImageViewProjectionTransform>();
   if(ivpt)
   {
      return dynamic_cast<ossimImageGeometry*>(ivpt->getView());
   }
      
   return getObjectAs<ossimImageGeometry>();

}
