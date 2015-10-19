#include <ossimGui/MdiSubWindowBase.h>
#include <QtGui/QMdiArea>
#include <QtGui/QMainWindow>

namespace ossimGui {
   MdiSubWindowBase::MdiSubWindowBase( QWidget * parent, Qt::WindowFlags flags)
   :QMdiSubWindow(parent, flags)
   {
   }
   MdiSubWindowBase::~MdiSubWindowBase()
   {
      if(m_connectableObject.valid()) m_connectableObject->disconnect();
      m_connectableObject = 0;
   }

   QMainWindow* MdiSubWindowBase::mainWindow()
   {
      QMainWindow* result = 0;
      QMdiArea* area = mdiArea();
      
      if(area)
      {
         QWidget* parent = area->parentWidget();
         while(parent && ! result)
         {
            result = dynamic_cast<QMainWindow*>(parent);
            parent = parent->parentWidget();
         }
      }
      
      return result;
   }
   
   void MdiSubWindowBase::setConnectableObject(ConnectableObject* connectable)
   {
      m_connectableObject = connectable;
      ConnectableDisplayObject* connectableDisplay = dynamic_cast<ConnectableDisplayObject*> (connectable);
      if(connectableDisplay) connectableDisplay->setDisplay(this);
   }
}

