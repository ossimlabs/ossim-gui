#ifndef  ossimGuiQtGlWidget_HEADER
#define ossimGuiQtGlWidget_HEADER
#include <QtOpenGL/QGLWidget>
#include <ossimGui/Export.h>
#include <iostream>
#include <osg/Node>
#include <osg/Timer>
#include <osg/Matrixd>
#include <osg/Material>
#include <osg/FrameStamp>
#include <osgDB/DatabasePager>
#include <osgGA/GUIEventHandler>
#include <osgGA/EventQueue>
#include <osgGA/EventVisitor>
#include <osgDB/ReadFile>
#include <osgUtil/SceneView>
//#include "ossimPlanetQtActionAdapter.h"
#include <ossimPlanet/ossimPlanet.h>
#include <ossimPlanet/ossimPlanetSceneView.h>
#include <ossimPlanet/ossimPlanetManipulator.h>
#include <osgDB/DatabasePager>
//#include <ossimPlanet/ossimPlanetDatabasePager.h>
#include <ossimPlanet/ossimPlanetLookAt.h>
#include <ossimPlanet/ossimPlanetViewer.h>
//#include <osgGA/MatrixManipulator>
#include <osgGA/CameraManipulator>
#include <osgGA/StateSetManipulator>
#include <ossimPlanet/ossimPlanetVisitors.h>
/* #include <ossimPlanet/ossimPlanet.h> */
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <OpenThreads/ReentrantMutex>
//class ossimOsgMainFormController;
namespace ossimGui
{
   class OSSIMGUI_DLL GlWidget : public QGLWidget
   {
      Q_OBJECT
   public:
      GlWidget( QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0 );
      GlWidget( const QGLFormat & format, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0 );
      virtual ~GlWidget();
      
      osgViewer::GraphicsWindow* getGraphicsWindow() { return m_graphicsWindow.get(); }
      const osgViewer::GraphicsWindow* getGraphicsWindow() const { return m_graphicsWindow.get(); }
      
      void setMouseNavigationFlag(bool flag);
      bool getMouseNavigationFlag()const;
      
      protected slots:
      
      void doIdleAnimationFrame();
      
   signals:
      void signalMouseMoveEvent(QMouseEvent* event);
      void signalMousePressEvent(QMouseEvent* event);
      void signalMouseReleaseEvent(QMouseEvent* event);
      void signalMouseDoubleClickEvent(QMouseEvent* event);
      void signalViewPositionChangedLatLonHgtHPR(double lat, double lon, double height,
                                                 double heading, double pitch, double roll);
    //  void signalDropEvent(QDropEvent * event);
    //  void signalDragEnterEvent(QDragEnterEvent *event);
      
   protected:
      void init();
      virtual void frameIfNeeded()=0;
      virtual void resizeGL( int width, int height );
      virtual void keyPressEvent( QKeyEvent* event );
      virtual void keyReleaseEvent( QKeyEvent* event );
      virtual void mousePressEvent( QMouseEvent* event );
      virtual void mouseReleaseEvent( QMouseEvent* event );
      virtual void mouseMoveEvent( QMouseEvent* event );
      virtual void mouseDoubleClickEvent ( QMouseEvent * event );
      virtual void wheelEvent ( QWheelEvent * event );
      osgGA::GUIEventAdapter::KeySymbol qtKeyToOsg(QKeyEvent * e)const;
      void addModifiers(osg::ref_ptr<osgGA::EventQueue> eventQueue, Qt::KeyboardModifiers modifier);
      
      osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_graphicsWindow;
      bool m_mouseNavigationFlag;
      bool m_passAllUnhandledEventsFlag;
   };
   
   class OSSIMGUI_DLL GlViewer : public GlWidget
   {
   public:
      GlViewer(QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);
      GlViewer( const QGLFormat & format, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0 );
      virtual ~GlViewer();
      void setViewer(ossimPlanetViewer* viewer);
      virtual void paintGL();
      
      virtual void frameIfNeeded();
      virtual void mouseMoveEvent( QMouseEvent* event );
      ossimPlanetViewer* viewer();
      void setCurrentSimulationTime(double simtime = USE_REFERENCE_TIME);
      void setTargetFrameTimeInMilliseconds(float millisecondRate);
      void setTargetFramesPerSecond(float framesPerSecond);
   protected:
      virtual void resizeGL( int width, int height );
      void clearPointersInViewer();
      QTimer *timer();    // Ensures that qtimer_ exists
      void noRenderCycle();
      OpenThreads::ReentrantMutex theDrawMutex;
      //   bool theRequestRedrawFlag;
      //   bool theRequestContinuousUpdateFlag;
      int m_timerInterval;
      osg::Matrixd m_currentViewMatrix;
      osg::Matrixd m_currentViewMatrixInverse;
      osg::Vec3d   m_currentViewLlh;
      osg::Vec3d   m_currentViewHpr;
      osg::ref_ptr<ossimPlanetViewer> m_viewer;
      double m_currentSimulationTime;
      float m_frameRateLimiter;
   private:
      QTimer *m_timer;	// This  should always be accessed via the timer() method
   };
}
#endif
