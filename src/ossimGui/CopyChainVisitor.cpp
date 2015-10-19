#include <ossimGui/CopyChainVisitor.h>

namespace ossimGui
{
   void CopyChainVisitor::reset()
   {
      m_currentIndex = 0;
      m_kwl.clear();
      ossimVisitor::reset();
      m_kwl.add("type", "ossimConnectableContainer", true);
   }
   void CopyChainVisitor::visit(ossimObject* obj)
   {
      if(!hasVisited(obj))
      {
         ossimVisitor::visit(obj);
         ossimString prefix = ossimString("object")+ossimString::toString(m_currentIndex)+".";
         obj->saveState(m_kwl, prefix);
         ++m_currentIndex;
      }
   }
   
   ossimObject* CopyChainVisitor::newContainer()const
   {
      return ossimObjectFactoryRegistry::instance()->createObject(m_kwl);
   }
   
}