#include <ossimGui/ConnectableObject.h>
namespace ossimGui{
   
RTTI_DEF1(ConnectableObject, "ConnectableObject", ossimConnectableObject);

   
   bool ConnectableObject::saveState(ossimKeywordlist& kwl, const char* prefix)const
   {
      bool result = ossimConnectableObject::saveState(kwl, prefix);
      
      kwl.add(ossimString(prefix)+"auto_delete", m_autoDelete);
      
      return result;
   }
   bool ConnectableObject::loadState(const ossimKeywordlist& kwl, const char* prefix)
   {
      bool result = ossimConnectableObject::loadState(kwl, prefix);
      
      ossimString auto_delete = kwl.find(ossimString(prefix), "auto_delete");
      
      if(!auto_delete.empty())
      {
         m_autoDelete = auto_delete.toBool();
      }
      
      return result;
   }

}