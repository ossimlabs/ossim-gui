#include <ossimGui/ProgressWidget.h>
#include <ossimGui/Event.h>
#include <ossim/base/ossimListenerManager.h>
#include <QEvent>
#include <QApplication>

namespace ossimGui
{
   ProgressWidget::ProgressWidget(QWidget* parent)
   :QProgressBar(parent)
   {
      m_listener = new Listener(this);
      setMaximum(100);
      setMinimum(0);
   }
   
   ProgressWidget::~ProgressWidget()
   {
      if(m_listener)
      {
         delete m_listener;
         m_listener = 0;
      }
   }
   
   void ProgressWidget::setObject(ossimObject* obj)
   {
      removeListener();
      reset();
      m_object = obj;
      addListener();
   }
   
   void ProgressWidget::removeListener()
   {
      ossimListenerManager* manager = dynamic_cast<ossimListenerManager*> (m_object.get());
      if(manager)
      {
         manager->removeListener(m_listener);
      }
   }
   void ProgressWidget::addListener()
   {
      ossimListenerManager* manager = dynamic_cast<ossimListenerManager*> (m_object.get());
      if(manager)
      {
         manager->addListener(m_listener);
      }
   }
   bool ProgressWidget::event(QEvent * e )
   {
      switch(e->type())
      {
         case PROGRESS_EVENT_ID:
         {
            ProgressEvent* evt = dynamic_cast<ProgressEvent*>(e);
            if(evt)
            {
               setValue(static_cast<int>(evt->percentComplete()));
               e->accept();
               return true;
            }
            break;
         }
         default:
         {
            break;
         }
      }
      return QProgressBar::event(e);
   }
   void ProgressWidget::Listener::processProgressEvent(ossimProcessProgressEvent& event)
   {
      QApplication::postEvent(m_widget, new ProgressEvent(event.getPercentComplete()));
      QApplication::processEvents();
   }

}
