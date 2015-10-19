#include <ossimGui/ConnectableImageObject.h>
#include <ossim/imaging/ossimImageSource.h>
namespace ossimGui {
   RTTI_DEF1(ConnectableImageObject, "ConnectableImageObject", ConnectableObject);
}

bool ossimGui::ConnectableImageObject::canConnectMyInputTo(ossim_int32 myInputIndex,
                                                    const ossimConnectableObject* object)const
{
   return dynamic_cast<const ossimImageSource*>(object);
}

ossimDrect ossimGui::ConnectableImageObject::getBounds()const
{
   ossimDrect result;
   getBounds(result);
   return result;
}

void ossimGui::ConnectableImageObject::getBounds(ossimDrect& result)const
{
   result.makeNan();
   
   ossim_uint32 idx = 0;
   for(idx = 0; idx < getNumberOfInputs(); ++idx)
   {
      const ossimImageSource* is = dynamic_cast<const ossimImageSource*> (getInput(idx));
      if(is)
      {
         ossimDrect rect = is->getBoundingRect();
         if(result.hasNans())
         {
            result = rect;
         }
         else 
         {
            if(!rect.hasNans())
            {
               result.combine(rect);
            }
         }
         
      }
   }
}
