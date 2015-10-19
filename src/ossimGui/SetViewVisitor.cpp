#include <ossimGui/SetViewVisitor.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimViewInterface.h>

namespace ossimGui
{
   void SetViewVisitor::setView()
   {
      int  refreshType = ossimRefreshEvent::REFRESH_NONE;
      ossim_uint32 nObjects =  m_collection.size();
      // bool viewChanged = false;
      ossim_uint32 collectionIdx = 0;
      
      if(m_obj.valid())
      {
         for(collectionIdx = 0; collectionIdx < nObjects; ++collectionIdx)
         {
            ossimViewInterface* viewInterface = getObjectAs<ossimViewInterface>(collectionIdx);
            ossimPropertyInterface* propertyInterface = getObjectAs<ossimPropertyInterface>(collectionIdx);
           if(viewInterface)
            {
               ossimObject* input = dynamic_cast<ossimObject*>(viewInterface->getView());
               if(input)
               {
                  if(!input->isEqualTo(*(m_obj.get())))
                  {
                     refreshType |= ossimRefreshEvent::REFRESH_GEOMETRY;
                     viewInterface->setView(m_obj->dup());
                  }
               }
            }
            if(!m_resamplerType.empty()&&propertyInterface)
            {
               if(propertyInterface->getPropertyValueAsString("filter_type") != m_resamplerType)
               {
                  refreshType |= ossimRefreshEvent::REFRESH_PIXELS;
                  propertyInterface->setProperty("filter_type", m_resamplerType);
               }
            }
         }
      }
      if(!m_viewPoint.hasNans())
      {
         refreshType|=ossimRefreshEvent::REFRESH_POSITION;
      }
      if(refreshType!=ossimRefreshEvent::REFRESH_NONE)
      {
         ossimRefreshEvent* event = new ossimRefreshEvent();
         
         if(m_obj.valid())
         {
            event->setRefreshType(refreshType);
         }
         event->setPosition(m_viewPoint);
         
         ossimEventVisitor eventVisitor(event);
         
         for(collectionIdx = 0; collectionIdx < nObjects; ++collectionIdx)
         {
            m_collection[collectionIdx]->accept(eventVisitor);
         }
      }
   }

}