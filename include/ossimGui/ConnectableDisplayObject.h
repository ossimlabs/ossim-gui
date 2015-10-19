#ifndef ossimGuiConnectableDisplayObject_HEADER
#define ossimGuiConnectableDisplayObject_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/ConnectableImageObject.h>

namespace ossimGui {
   class MdiSubWindowBase;
   class OSSIMGUI_DLL ConnectableDisplayObject : public ConnectableImageObject
   {
   public:
      ConnectableDisplayObject(MdiSubWindowBase* m_display=0);
      
      void setDisplay(MdiSubWindowBase* d){m_display = d;}
      MdiSubWindowBase* display(){return m_display;}
      virtual void close();
      virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
      virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
   protected:
      MdiSubWindowBase* m_display;
      TYPE_DATA;
   };
   
}
#endif
