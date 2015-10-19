#include <ossimGui/Util.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>

namespace ossimGui 
{
   void Util::imageWriterTypes(QStringList& result)
   {
      std::vector<ossimString> writers;
      imageWriterTypes(writers);
      if(!writers.empty())
      {
         ossim_uint32 idx = 0;
         for(idx = 0; idx < writers.size(); ++idx)
         {
            result.push_back(writers[idx].c_str());
         }
      }
   }
   
   void Util::imageWriterTypes(std::vector<ossimString>& result)
   {
      ossimImageWriterFactoryRegistry::instance()->getTypeNameList(result);
   }
}
