#ifndef ossimGuiConnection_HEADER
#define ossimGuiConnection_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimConnectableObject.h>

namespace ossimGui {
   class OSSIMGUI_DLL ConnectableObject : public ossimConnectableObject
   {
   public:
      ConnectableObject():ossimConnectableObject(0,0,false,false),m_autoDelete(true){}
      ConnectableObject(ossimObject* owner,
                 ossim_int32 inputListSize,
                 ossim_int32 outputListSize,
                 bool inputListIsFixedFlag=true,
                 bool outputListIsFixedFlag=true):ossimConnectableObject(owner, inputListSize, outputListSize, inputListIsFixedFlag, outputListIsFixedFlag){}

      virtual bool saveState(ossimKeywordlist& kwl, const char* prefix=0)const;
      virtual bool loadState(const ossimKeywordlist& kwl, const char* prefix=0);
      /**
       * Specifies if the connectable display auto deletes on disconnect
       *
       * Note, if the display attribute is not delete on close it
       * will not delete but just hide and keep it alive in the datamanager
       */ 
      bool autoDelete()const{return m_autoDelete;}
      /**
       * Turns on and off the ability to auto delete when the node is disconnected
       * from all inputs.
       */
      void setAutoDelete(bool flag){m_autoDelete = flag;}
      
   protected:
      bool m_autoDelete;
      TYPE_DATA;
   };
   
}
#endif
