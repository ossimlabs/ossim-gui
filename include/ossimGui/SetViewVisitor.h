#ifndef ossimGuiSetViewVisitor_HEADER
#define ossimGuiSetViewVisitor_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageGeometry.h>

namespace ossimGui
{
class OSSIMGUI_DLL SetViewVisitor : public ossimTypeNameVisitor
{
public:
   SetViewVisitor(ossimObject* view,
                  int visitorType =(VISIT_INPUTS|VISIT_CHILDREN))
   :ossimTypeNameVisitor("ossimViewInterface", false,visitorType),
   m_obj(view)
   {
      m_viewPoint.makeNan();
   }
   SetViewVisitor(const SetViewVisitor& src)
   :ossimTypeNameVisitor(src),
   m_obj(src.m_obj)
   {
      m_viewPoint.makeNan();
   }
   virtual ossimRefPtr<ossimVisitor> dup()const{return new SetViewVisitor(*this);}
   
   void setView();
   void setGeometry(ossimImageGeometry* obj){m_obj = obj;}
   void setViewPoint(const ossimDpt& pt){m_viewPoint = pt;}
   void setResamplerType(const ossimString& value){m_resamplerType = value;}
protected:
   ossimDpt m_viewPoint;
   ossimRefPtr<ossimObject> m_obj;
   ossimString m_resamplerType;
};

}


#endif
