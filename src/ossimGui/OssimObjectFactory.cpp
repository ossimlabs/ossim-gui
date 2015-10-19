#include <ossimGui/OssimObjectFactory.h>
#include <ossimGui/ConnectableDisplayObject.h>

ossimGui::OssimObjectFactory* ossimGui::OssimObjectFactory::m_instance = 0;
ossimGui::OssimObjectFactory::OssimObjectFactory()
{
   m_instance = this;
}

ossimGui::OssimObjectFactory* ossimGui::OssimObjectFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new OssimObjectFactory();
   }
   
   return m_instance;
}

ossimObject*  ossimGui::OssimObjectFactory::createObject(const ossimString& typeName)const
{
   ossimObject* result = 0;
   if(typeName == "ConnectableDisplayObject")
   {
      result = new ConnectableDisplayObject();
   }
   
   return result;
}

ossimObject*  ossimGui::OssimObjectFactory::createObject(const ossimKeywordlist& kwl,
                                                         const char* prefix)const
{
   ossimRefPtr<ossimObject> result = 0;
   ossimString type = kwl.find(prefix, "type");
   if(!type.empty())
   {
      result = createObject(type);
      if(result.valid())
      {
         if(!result->loadState(kwl, prefix))
         {
            result = 0;
         }
      }
   }
   
   return result.release();
}

void  ossimGui::OssimObjectFactory::getTypeNameList(std::vector<ossimString>& typeList)const
{
   typeList.push_back("ConnectableDisplayObject");
}
