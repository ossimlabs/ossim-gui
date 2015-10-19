#ifndef ossimGuiImageWriterJob_HEADER
#define ossimGuiImageWriterJob_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/ProcessInterfaceJob.h>
#include <ossim/base/ossimKeywordlist.h>
#include <OpenThreads/Mutex>

namespace ossimGui
{
   class OSSIMGUI_DLL ImageWriterJob : public ProcessInterfaceJob
   {
   public:
      ImageWriterJob(const ossimKeywordlist& kwl)
      :ProcessInterfaceJob(),m_kwl(kwl)
      {
         m_processInterface = 0;
      }
      
      virtual void start();
      
   protected:
      ossimKeywordlist m_kwl;
      ossimRefPtr<ossimObject> m_containerObj;
   };
}

#endif
