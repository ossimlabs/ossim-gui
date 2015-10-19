#include <ossimGui/IvtGeomTransform.h>


void ossimGui::IvtGeomTransform::viewToImage(const ossimDpt& viewPt, ossimDpt& ipt)
{
  ipt.makeNan();
  if(m_ivt.valid())
  {
    m_ivt->viewToImage(viewPt, ipt);
  }
}

void ossimGui::IvtGeomTransform::imageToView(const ossimDpt& ipt, ossimDpt& viewPt)
{
  viewPt.makeNan();
  if(m_ivt.valid())
  {
    m_ivt->imageToView(ipt, viewPt);
  }
}
void ossimGui::IvtGeomTransform::imageToGround(const ossimDpt& ipt, ossimGpt& gpt)
{
  gpt.makeNan();
  if(m_geom.valid())
  {
    m_geom->localToWorld(ipt, gpt);
  }
}
void ossimGui::IvtGeomTransform::groundToImage(const ossimGpt& gpt, ossimDpt& ipt)
{
  ipt.makeNan();
  if(m_geom.valid())
  {
    m_geom->worldToLocal(gpt, ipt);
  }
}

void ossimGui::IvtGeomTransform::viewToGround(const ossimDpt& viewPt, ossimGpt& gpt)
{
  ossimDpt ipt;
  gpt.makeNan();
  viewToImage(viewPt, ipt);
  if(!ipt.hasNans())
  {
    imageToGround(ipt, gpt);
  }
}

void ossimGui::IvtGeomTransform::groundToView(const ossimGpt& gpt, ossimDpt& viewPt)
{
  ossimDpt ipt;
  viewPt.makeNan();

  groundToImage(gpt, ipt);
  if(!ipt.hasNans())
  {
    imageToView(ipt, viewPt);
  }
}

