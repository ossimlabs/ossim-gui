#include <ossimGui/GlWidget.h>
#include <QtGui/QCursor>
#include <QtCore/QTimer>
#include <QtCore/QRect>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QLayout>
#include <QtGui/QFrame>
#include <OpenThreads/ScopedLock>
#include <osgUtil/LineSegmentIntersector>
#include <osg/io_utils>
#include <osgGA/GUIEventAdapter>
#include <ossimPlanet/ul.h>

namespace ossimGui
{
   GlWidget::GlWidget( QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
   :QGLWidget(parent, shareWidget, f),
   m_mouseNavigationFlag(true),
   m_passAllUnhandledEventsFlag(true)
   {
      init();
   }
   
   GlWidget::GlWidget( const QGLFormat & format, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f )
   :QGLWidget(format, parent, shareWidget, f),
   m_mouseNavigationFlag(true),
   m_passAllUnhandledEventsFlag(true)
   {
      init();
   }
   
   GlWidget::~GlWidget()
   {
      std::cout << "GlWidget::~GlWidget()\n";
   }
   
   void GlWidget::init()
   {
      m_graphicsWindow = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
      setAcceptDrops(false);
   }
   
   void GlWidget::doIdleAnimationFrame()
   {
      frameIfNeeded();
   }
   
   void GlWidget::setMouseNavigationFlag(bool flag)
   {
      m_mouseNavigationFlag = flag;
   }
   
   bool GlWidget::getMouseNavigationFlag()const
   {
      return m_mouseNavigationFlag;
   }
   
   void GlWidget::resizeGL( int width, int height )
   {
      width = ossim::max(64, width);
      height = ossim::max(64, height);
      m_graphicsWindow->getEventQueue()->windowResize(0, 0, width, height );
      m_graphicsWindow->resized(0,0,width,height);
   }
   
   void GlWidget::keyPressEvent( QKeyEvent* event )
   {
      m_graphicsWindow->getEventQueue()->keyPress( qtKeyToOsg(event) );
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
   }
   
   void GlWidget::keyReleaseEvent( QKeyEvent* event )
   {
      m_graphicsWindow->getEventQueue()->keyRelease( qtKeyToOsg(event) );
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
   }
   
   void GlWidget::mousePressEvent( QMouseEvent* event )
   {
      int button = 0;
      switch(event->button())
      {
         case(Qt::LeftButton): button = 1; break;
         case(Qt::MidButton): button = 2; break;
         case(Qt::RightButton): button = 3; break;
         case(Qt::NoButton): button = 0; break;
         default: button = 0; break;
      }
      m_graphicsWindow->getEventQueue()->mouseButtonPress(event->x(), event->y(), button);
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
      emit signalMousePressEvent(event);
   }
   
   void GlWidget::mouseReleaseEvent( QMouseEvent* event )
   {
      int button = 0;
      switch(event->button())
      {
         case(Qt::LeftButton): button = 1; break;
         case(Qt::MidButton): button = 2; break;
         case(Qt::RightButton): button = 3; break;
         case(Qt::NoButton): button = 0; break;
         default: button = 0; break;
      }
      m_graphicsWindow->getEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
      emit signalMouseReleaseEvent(event);
   }
   
   void GlWidget::mouseMoveEvent( QMouseEvent* event )
   {
      m_graphicsWindow->getEventQueue()->mouseMotion(event->x(), event->y());
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      emit signalMouseMoveEvent(event);
      
   }
   
   void GlWidget::mouseDoubleClickEvent( QMouseEvent * event )
   {
      int button = 0;
      switch(event->button())
      {
         case(Qt::LeftButton): button = 1; break;
         case(Qt::MidButton): button = 2; break;
         case(Qt::RightButton): button = 3; break;
         case(Qt::NoButton): button = 0; break;
         default: button = 0; break;
      }
      m_graphicsWindow->getEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
   }
   
   void GlWidget::wheelEvent(QWheelEvent * event)
   {
      m_graphicsWindow->getEventQueue()->mouseScroll(event->delta()>0?osgGA::GUIEventAdapter::SCROLL_UP:osgGA::GUIEventAdapter::SCROLL_DOWN);
      addModifiers(m_graphicsWindow->getEventQueue(), event->modifiers());
      if(m_passAllUnhandledEventsFlag)
      {
         event->ignore();
      }
   }
 
#if 0
   void GlWidget::dropEvent ( QDropEvent * event )
   {
      emit signalDropEvent(event);
   }
   
   void GlWidget::dragEnterEvent(QDragEnterEvent *event)
   {
      emit signalDragEnterEvent(event);
   }
#endif
   osgGA::GUIEventAdapter::KeySymbol GlWidget::qtKeyToOsg(QKeyEvent * e)const
   {
      int qtKey = e->key();
      switch(qtKey)
      {
         case Qt::Key_Up:
         {
            return(osgGA::GUIEventAdapter::KEY_Up);
         }
         case Qt::Key_Down:
         {
            return (osgGA::GUIEventAdapter::KEY_Down);
         }
         case Qt::Key_Left:
         {
            return (osgGA::GUIEventAdapter::KEY_Left);
         }
         case Qt::Key_Right:
         {
            return (osgGA::GUIEventAdapter::KEY_Right);
         }
         case Qt::Key_Return:
         {
            return (osgGA::GUIEventAdapter::KEY_Return);
         }
         default:
         {
            if((qtKey >= Qt::Key_A)&&(qtKey <= Qt::Key_Z))
            {
               QString s = e->text();
               std::string stdString = s.toStdString();
               char c = *stdString.begin();
               return (osgGA::GUIEventAdapter::KeySymbol)(c);  
            }
         }
      }
      
      return (osgGA::GUIEventAdapter::KeySymbol)(qtKey);
   }
   
   void GlWidget::addModifiers(osg::ref_ptr<osgGA::EventQueue> eventQueue, Qt::KeyboardModifiers modifier)
   {
      if(!eventQueue.valid()) return;
      unsigned int modKeyMask = 0;
      osgGA::GUIEventAdapter* adapter = eventQueue->getCurrentEventState();
      if(!adapter) return;
      if(modifier & Qt::ShiftModifier)
      {
         modKeyMask|=osgGA::GUIEventAdapter::MODKEY_SHIFT;
      }
      if(modifier & Qt::ControlModifier)
      {
         modKeyMask|=osgGA::GUIEventAdapter::MODKEY_CTRL;
      }
      if(modifier & Qt::AltModifier)
      {
         modKeyMask|=osgGA::GUIEventAdapter::MODKEY_ALT;
      }
      if(modifier & Qt::MetaModifier)
      {
         modKeyMask|=osgGA::GUIEventAdapter::MODKEY_META;
      }
      adapter->setModKeyMask(modKeyMask);
   }
   
   GlViewer::GlViewer(QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f)
   :GlWidget( parent, shareWidget, f ),
   m_timerInterval(0),
   m_timer(0)
   {
      setViewer(new ossimPlanetViewer);
      setCurrentSimulationTime();
      setTargetFramesPerSecond(60);
      m_currentSimulationTime = USE_REFERENCE_TIME;
      timer()->setInterval(m_timerInterval);
      timer()->setSingleShot(false);
      timer()->start();
   }
   
   GlViewer::GlViewer( const QGLFormat & format, QWidget * parent, const QGLWidget * shareWidget, Qt::WindowFlags f )
   :GlWidget(format, parent, shareWidget, f),
   m_timerInterval(0),
   m_timer(0)
   {
      setViewer(new ossimPlanetViewer);
      setCurrentSimulationTime();
      setTargetFramesPerSecond(60);
      m_currentSimulationTime = USE_REFERENCE_TIME;
      timer()->setInterval(m_timerInterval);
      timer()->setSingleShot(false);
      timer()->start();
   }
   
   GlViewer::~GlViewer()
   {
      clearPointersInViewer(); 
   }
   
   void GlViewer::setViewer(ossimPlanetViewer* viewer)
   {
      clearPointersInViewer();
      ossim_uint32 idx = 0;
      m_viewer = viewer;
      if(m_viewer.valid())
      {
         m_viewer->getCamera()->setGraphicsContext(getGraphicsWindow());
         osgViewer::ViewerBase::Cameras cameraList;
         m_viewer->getCameras(cameraList);
         for(idx = 0; idx < cameraList.size();++idx)
         {
            cameraList[idx]->setGraphicsContext(getGraphicsWindow());
         }
         // initialize some size for the view port for this canvas if it doesn't have any
         //
         int w=width(), h=height();
         
         // let's default to something
         w = w>0?w:10;
         h = h>0?h:10;
         if((w > 0)&&(h>0))
         {
            m_viewer->getCamera()->setViewport(new osg::Viewport(0,0,w,h));
            m_viewer->getCamera()->setProjectionMatrixAsPerspective(45.0, 
                                                                    static_cast<double>(w)/
                                                                    static_cast<double>(h), 
                                                                    1.0, 
                                                                    50.0);
         }
      }
   }
   
   void GlViewer::resizeGL( int width, int height )
   {
      width = ossim::max(64, width);
      height = ossim::max(64, height);
      GlWidget::resizeGL(width, height);
      if(m_viewer.valid()&&m_viewer->getCamera())
      {
         m_viewer->getCamera()->getViewport()->setViewport(0,0,width,height);
         double fovy, aspectRatio, znear, zfar;
         m_viewer->getCamera()->getProjectionMatrixAsPerspective(fovy, aspectRatio, znear, zfar);
         aspectRatio = static_cast<double>(width)/static_cast<double>(height);
         m_viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, 
                                                                  aspectRatio, 
                                                                  znear, 
                                                                  zfar);
      }
   }
   
   void GlViewer::clearPointersInViewer()
   {
      if(m_viewer.valid())
      {
         ossim_uint32 idx = 0;
         osgViewer::ViewerBase::Cameras cameraList;
         m_viewer->getCameras(cameraList);
         for(idx = 0; idx < cameraList.size();++idx)
         {
            cameraList[idx]->setGraphicsContext(0);
         }
      }
   }
   void GlViewer::setTargetFrameTimeInMilliseconds(float millisecondRate)
   {
      m_frameRateLimiter = millisecondRate;
   }
   
   void GlViewer::setTargetFramesPerSecond(float framesPerSecond)
   {
      setTargetFrameTimeInMilliseconds(1000.0/framesPerSecond);
   }
   
   QTimer *GlViewer::timer()
   {
      if(!m_timer)
      {
         m_timer=new QTimer(this);
         connect(m_timer,SIGNAL(timeout()),this,SLOT(doIdleAnimationFrame()));
      }
      return m_timer;
   }
   
#if 0
   void GlViewer::requestRedraw()
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theDrawMutex);
      theRequestRedrawFlag = true;
      if(!timer()->isActive())
      {
         timer()->setInterval(theTimerInterval);
         timer()->start();
      }
   }
#endif
#if 0
   void GlViewer::requestContinuousUpdate(bool needed)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theDrawMutex);
      theRequestContinuousUpdateFlag = needed;
      if(needed)
      {
         if(!timer()->isActive())
         {
            timer()->setInterval(theTimerInterval);
            timer()->start();
         }
      }
   }
#endif
   
   ossimPlanetViewer* GlViewer::viewer()
   {
      return m_viewer.get();
   }
   
   void GlViewer::setCurrentSimulationTime(double simtime)
   {
      m_currentSimulationTime = simtime; 
   }
   
   void GlViewer::frameIfNeeded()
   {
      //OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theDrawMutex);
      bool doVBlankLimit = false;
      osg::Timer_t beginT = osg::Timer::instance()->tick();
      osg::Timer_t endT;
      if((m_viewer->continuousUpdateFlag()||
          m_viewer->getAndSetRedrawFlag(false))&&isVisible())
      {
         updateGL();
         doVBlankLimit = format().swapInterval() < 0;
         endT = osg::Timer::instance()->tick();
      }
      else
      {
         noRenderCycle();
         doVBlankLimit = true;
         endT = osg::Timer::instance()->tick();
      }
      if(doVBlankLimit)
      {
         double test = osg::Timer::instance()->delta_m(beginT, endT);
         if(test < m_frameRateLimiter)
         {
            ulMilliSecondSleep((int)(m_frameRateLimiter-test));
         }
      }
#if 0
      std::cout << "---" << std::endl;
      std::cout << (getDatabasePager()->requiresUpdateSceneGraph()||getDatabasePager()->requiresCompileGLObjects()) << std::endl;
      std::cout << getDatabasePager()->getFileRequestListSize() << std::endl
      << getDatabasePager()->getDataToCompileListSize() << std::endl;
#endif
   }
   
   void GlViewer::noRenderCycle()//double simtime)
   {
      if(!m_viewer.valid())return;
      m_viewer->advance(m_currentSimulationTime);
      getGraphicsWindow()->getEventQueue()->frame(m_viewer->getViewerFrameStamp()->getReferenceTime());
      m_viewer->eventTraversal();
      m_viewer->updateTraversal();
   }
   
   void GlViewer::paintGL()
   {
      if(m_viewer.valid())
      {
         m_viewer->frame(m_currentSimulationTime);
      }
   }
   
   void GlViewer::mouseMoveEvent( QMouseEvent* event )
   {
      GlWidget::mouseMoveEvent(event);
   }
}