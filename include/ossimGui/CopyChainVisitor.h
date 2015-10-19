#ifndef ossimGuiCopyChainVisitor_HEADER
#define ossimGuiCopyChainVisitor_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimKeywordlist.h>

namespace ossimGui 
{
   class CopyChainVisitor : public ossimVisitor
   {
   public:
      CopyChainVisitor():ossimVisitor(ossimVisitor::VISIT_INPUTS){reset();}
      virtual ossimRefPtr<ossimVisitor> dup()const{return new CopyChainVisitor(*this);}
      virtual void reset();
      virtual void visit(ossimObject* obj);
      
      ossimObject* newContainer()const;
      const ossimKeywordlist& kwl()const{return m_kwl;}
      
   protected:
      ossimKeywordlist m_kwl;
      ossim_uint32 m_currentIndex;
   };
}
#endif
