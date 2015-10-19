#include <ossimGui/ProgressWidget.h>
#include <ossimGui/Event.h>
#include <ossim/base/ossimListenerManager.h>
#include <QtCore/QEvent>
#include <QtGui/QApplication>

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
      // cout << "ProgressWidget::~ProgressWidget entered..." << endl;
      if(m_listener)
      {
         removeListener();
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
      bool result = false;
      
      if ( e->type() == (QEvent::Type)PROGRESS_EVENT_ID )
      {
         ProgressEvent* evt = dynamic_cast<ProgressEvent*>(e);
         if(evt)
         {
            setValue(static_cast<int>(evt->percentComplete()));
            e->accept();
            result = true;
         }
      }
      if ( !result )
      {
         result = QProgressBar::event(e);
      }
      return result;
   }
   
   void ProgressWidget::Listener::processProgressEvent(ossimProcessProgressEvent& event)
   {
      QApplication::postEvent(m_widget, new ProgressEvent(event.getPercentComplete()));
      QApplication::processEvents();
   }
}
