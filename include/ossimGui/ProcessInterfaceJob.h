#ifndef ossimGuiProecessInterfaceJob_HEADER
#define ossimGuiProecessInterfaceJob_HEADER
#include <ossimGui/Export.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/base/ossimProcessInterface.h>
#include <ossim/base/ossimProcessListener.h>

namespace ossimGui
{
   class OSSIMGUI_DLL ProcessInterfaceJob : public ossimJob
   {
   public:
      ProcessInterfaceJob():m_processInterface(0){}
      
      virtual void setState(int value, bool on=true);
      virtual void start();
      ossimObject* object(){return m_obj.get();}
      
   protected:
      class ProgressListener : public ossimProcessListener
      {
      public:
         ProgressListener(ossimJob* job):m_job(job){}
         virtual void processProgressEvent(ossimProcessProgressEvent& event);
         
         ossimJob* m_job;
         
      };
      friend class ProgressListener;
      void setPercentComplete(double value);

      
      mutable OpenThreads::Mutex m_processInterfaceMutex;
      ossimRefPtr<ossimObject> m_obj;
      ossimProcessInterface* m_processInterface;
   };
}   

#endif
