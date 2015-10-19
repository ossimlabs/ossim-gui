#include <ossimGui/OpenImageUrlJob.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
void ossimGui::OpenImageUrlJob::start()
{
   ossimFilename file = m_url.toString().toAscii().data();
   
   //test if uri is local file
   if(m_url.scheme().toLower() == "file")
   {
      file = m_url.toLocalFile().toAscii().data();
      if(!file.exists()) return;
   }
   ossimRefPtr<ossimImageHandler> ih = ossimImageHandlerRegistry::instance()->open(file);
   if(ih.valid())
   {
      m_handlers.push_back(ih.get());
      ossim_uint32 nEntries = ih->getNumberOfEntries();
      if(nEntries > 1)
      {
         m_handlers.push_back(ih.get());
         ossim_uint32 idx=0;
         do
         {
            ++idx;
            ih = static_cast<ossimImageHandler*>(ih->dup());
            if(ih->setCurrentEntry(idx))
            {
               m_handlers.push_back(ih.get());
            }
         }while(((idx+1) < nEntries)&&!isCanceled());
      }
   }
   
   if(isCanceled())
   {
      m_handlers.clear();
   }
}
