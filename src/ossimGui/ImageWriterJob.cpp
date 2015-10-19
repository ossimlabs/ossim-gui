#include <ossimGui/ImageWriterJob.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimVisitor.h>

namespace ossimGui
{
   void ImageWriterJob::start()
   {
      if(isCanceled()) return;
         
      m_processInterfaceMutex.lock();
      m_containerObj = ossimObjectFactoryRegistry::instance()->createObject(m_kwl);
      if(m_containerObj.valid())
      {
         ossimTypeNameVisitor typeVisitor("ossimImageFileWriter");
         m_containerObj->accept(typeVisitor);
         m_obj = typeVisitor.getObject();
         m_processInterface = 0;
         m_processInterfaceMutex.unlock();
         typeVisitor.reset();
         if(m_obj.valid())
         {
            ProcessInterfaceJob::start();
         }
      }
      else 
      {
         m_processInterfaceMutex.unlock();
      }
   }
}