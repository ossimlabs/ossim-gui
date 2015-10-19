#ifndef ossimGuiMdiSubWindowBase_HEADER
#define ossimGuiMdiSubWindowBase_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/View.h>
#include <ossimGui/ConnectableDisplayObject.h>
#include <QtGui/QMdiSubWindow>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/parallel/ossimJobQueue.h>

class QMainWindow;
namespace ossimGui{

   class OSSIMGUI_DLL MdiSubWindowBase : public QMdiSubWindow
   {
   public:
      MdiSubWindowBase( QWidget * parent = 0, Qt::WindowFlags flags = 0);
      virtual ~MdiSubWindowBase();
      QMainWindow* mainWindow();
      
      virtual void sync(View& /* viewInfo */){}
      virtual void setJobQueue(ossimJobQueue* /* q */){}
      virtual ConnectableObject* connectableObject(){return m_connectableObject.get();}
      virtual const ConnectableObject* connectableObject()const{return m_connectableObject.get();}
      virtual void setConnectableObject(ConnectableObject* connectable);
      
   protected:
      ossimRefPtr<ConnectableObject> m_connectableObject;
   };
}
#endif
