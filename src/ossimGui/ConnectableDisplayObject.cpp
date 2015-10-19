#include <ossimGui/ConnectableDisplayObject.h>
#include <ossimGui/MdiSubWindowBase.h>

namespace ossimGui {
   RTTI_DEF1(ConnectableDisplayObject, "ConnectableDisplayObject", ConnectableImageObject);
   
   ConnectableDisplayObject::ConnectableDisplayObject(MdiSubWindowBase* d)
   :ConnectableImageObject(), m_display(d)
   {
   }
   void ConnectableDisplayObject::close()
   {
      if(m_display) 
      {
         bool testAtt = m_display->testAttribute(Qt::WA_DeleteOnClose);
         m_display->close(); 
         if(testAtt)
         {
            m_display = 0;
         }
      }
   }

   bool ConnectableDisplayObject::saveState(ossimKeywordlist& kwl, const char* prefix)const
   {
      bool result = ConnectableImageObject::saveState(kwl, prefix);
      
      if(m_display)
      {
      }
      
      return result;
   }
   
   bool ConnectableDisplayObject::loadState(const ossimKeywordlist& kwl, const char* prefix)
   {
      bool result = ConnectableImageObject::loadState(kwl, prefix);
      
      
      if(m_display)
      {
      }
      
      return result;
   }
}
