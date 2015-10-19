#ifndef ossimGuiConnectableImageObject_HEADER
#define ossimGuiConnectableImageObject_HEADER
#include <ossimGui/ConnectableObject.h>
#include <ossim/base/ossimDrect.h>
namespace ossimGui {
   class OSSIMGUI_DLL ConnectableImageObject : public ConnectableObject
   {
   public:
      /**
       * required to be overriden by derived classes
       */
      virtual bool canConnectMyInputTo(ossim_int32 myInputIndex,
                                       const ossimConnectableObject* object)const;
      ossimDrect getBounds()const;
      void getBounds(ossimDrect& result)const;
      virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const
      {
         return ConnectableObject::saveState(kwl, prefix);
      }
      virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0)
      {
         return ConnectableObject::loadState(kwl, prefix);
      }
      TYPE_DATA;
   };
}

#endif
