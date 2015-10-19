#include <ossimGui/ProcessInterfaceJob.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimConnectableObject.h>

namespace ossimGui
{
   void ProcessInterfaceJob::ProgressListener::processProgressEvent(ossimProcessProgressEvent& event)
   {
      if(m_job) m_job->setPercentComplete(event.getPercentComplete());
   }

   void ProcessInterfaceJob::setState(int value, bool on)
   {
      m_processInterfaceMutex.lock();
      if(value&ossimJob::ossimJob_CANCEL)
      {
         if(m_processInterface) m_processInterface->abort();
      }
      m_processInterfaceMutex.unlock();
      ossimJob::setState(value, on);
   }
   
   void ProcessInterfaceJob::start()
   {
      m_processInterfaceMutex.lock();
      m_processInterface = dynamic_cast<ossimProcessInterface*> (m_obj.get());
      if(m_processInterface)
      {
         m_processInterfaceMutex.unlock();
         ProgressListener* listener = new ProgressListener(this);
         ossimConnectableObject* obj = dynamic_cast<ossimConnectableObject*> (m_obj.get());
         if(obj) 
         {
            obj->addListener(listener);
         }
         m_processInterface->execute();
         if(obj) 
         {
            obj->removeListener(listener);
         }
         delete listener; listener = 0;
      }
      else 
      {
         m_processInterfaceMutex.unlock();
      }
   }
   void ProcessInterfaceJob::setPercentComplete(double value)
   {
      m_processInterfaceMutex.lock();
      ossimRefPtr<ossimJobCallback> c = callback();
      m_processInterfaceMutex.unlock();

      if(c.valid())
      {
         c->percentCompleteChanged(value, this);
      }
   }
}