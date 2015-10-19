#ifndef ossimGuiProgressWidget_HEADER
#define ossimGuiProgressWidget_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimProcessProgressEvent.h>
#include <ossim/base/ossimProcessListener.h>
#include <ossim/base/ossimRefPtr.h>

#include <QtGui/QProgressBar>

namespace ossimGui 
{
   class OSSIMGUI_DLL ProgressWidget : public QProgressBar
   {
      Q_OBJECT
   public:
      ProgressWidget(QWidget* parent);
      virtual ~ProgressWidget();
      virtual void setObject(ossimObject* obj);
      
      virtual bool event(QEvent * e );
   protected:
      class Listener : public ossimProcessListener
      {
      public:
         Listener(ProgressWidget* w):m_widget(w){}
         virtual void processProgressEvent(ossimProcessProgressEvent& event);
         
         ProgressWidget* m_widget;
         
      };
      void removeListener();
      void addListener();
      
      ossimRefPtr<ossimObject> m_object;
      Listener*        m_listener;
   };
}
#endif
