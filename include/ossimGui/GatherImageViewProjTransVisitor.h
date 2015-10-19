#ifndef ossimGuiGatherImageViewProjTransVisitor_HEADER
#define ossimGuiGatherImageViewProjTransVisitor_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/IvtGeomTransform.h>
#include <ossim/base/ossimVisitor.h>

namespace ossimGui
{
  class OSSIMGUI_DLL GatherImageViewProjTransVisitor : public ossimVisitor
  {
  public:
    typedef std::vector<ossimRefPtr<IvtGeomTransform> > TransformList;

    GatherImageViewProjTransVisitor(int visitorType =(VISIT_INPUTS|VISIT_CHILDREN))
    :ossimVisitor(visitorType)
    {
    }
    virtual ossimRefPtr<ossimVisitor> dup()const{return new GatherImageViewProjTransVisitor(*this);}
    virtual void visit(ossimObject* obj);

    TransformList& getTransformList(){return m_transformList;}
    const TransformList& getTransformList()const{return m_transformList;}
  protected:
    TransformList m_transformList;
  };
}

#endif