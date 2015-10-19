#ifndef ImageMdiSubWindow_HEADER
#define ImageMdiSubWindow_HEADER
#include <ossimGui/MdiSubWindowBase.h>
#include <ossimGui/ConnectableDisplayObject.h>
#include <ossimGui/Export.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimVisitor.h>
#include <QtGui/QMouseEvent>
class QMenu;
class QToolBar;
class QMenuBar;
class QMainWindow;
class ossimConnectableObject;
namespace ossimGui {
   //class ImageScrollWidget;
   class ImageScrollView;
   class OSSIMGUI_DLL ImageActions : public QObject
   {
      Q_OBJECT
   public:
      class OSSIMGUI_DLL Visitor : public ossimVisitor
      {
      public:
         Visitor();
         virtual void reset();
         virtual ossimRefPtr<ossimVisitor> dup()const{return new Visitor(*this);}
         virtual void visit(ossimConnectableObject* obj);
         
         ossimVisitor::ListRef m_imageAdjustments;
         bool                  m_isProjected;
         bool                  m_isAffine;
         ossimVisitor::ListRef m_imageHandlers;
         ossimVisitor::ListRef m_bandSelectors;
         ossimVisitor::ListRef m_histogramRemappers;
         ossimVisitor::ListRef m_imageRenderers;
         ossimVisitor::ListRef m_viewInterfaces;
         ossimVisitor::ListRef m_scalarRemappers;
         ossimVisitor::ListRef m_hsiRemappers;
         ossimVisitor::ListRef m_brightnessContrastSources;
         ossimVisitor::ListRef m_containers;
      };
      
      ImageActions();
      void addActions(QMainWindow* mainWindow);
      void removeActions(QMainWindow* mainWindow);
      void setWidget(ossimGui::ImageScrollView* widget){m_widget = widget;}
      void setupAndExecuteSyncing();
      Visitor& visitor(){return m_visitor;}
      const Visitor& visitor()const{return m_visitor;}
   public slots:
      void exportImage();
      void saveAs();      
      void exportKeywordlist();
      void editBandSelector();
      void editHsiAdjustments();
      void editBrightnessContrast();
      void editHistogramRemapper();
      void editGeometryAdjustments();
      void editView();
      void showPolygonRemapper();
      void showPositionInformation();
      
      void interpolationTypeChanged(const QString& value);
      void fitToWindow();
      void fullRes();
      void zoomIn(double factor=2.0);
      void zoomOut(double factor=2.0);
      void syncingOptionsChanged(const QString& value);      
      void layerOptionsChanged(int index);      
//      void mouseMove(QMouseEvent* event,   const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
//      void mousePress(QMouseEvent* event,   const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
//      void mouseRelease(QMouseEvent* event,   const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
//      void mouseDoubleClick(QMouseEvent* event,   const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
//      void wheel(QWheelEvent* event,  const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      void track(const ossimDpt& scenePoint);
   signals:
      void syncView(View&);
   protected:
      ossimImageGeometry* getView();
      
      Visitor m_visitor;
      ossimGui::ImageScrollView*   m_widget;
      ossim_uint32                 m_syncType;
      ossimDpt                     m_currentScenePoint;
      QString                      m_resamplerType;
      bool                         m_leftButtonPressed;
   };

   class OSSIMGUI_DLL ImageMdiSubWindow : public MdiSubWindowBase
   {
      Q_OBJECT
   public:
      ImageMdiSubWindow( QWidget * parent = 0, Qt::WindowFlags flags = 0);
      virtual ~ImageMdiSubWindow();
      
      ossimGui::ImageScrollView* scrollWidget();
      virtual void setJobQueue(ossimJobQueue* q);
      virtual void sync(View& syncInfo);
      virtual void setConnectableObject(ConnectableObject* connectable);
      ImageActions* getImageActions()const {return m_actions;}

   public slots:
      void stateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);
      void syncView(View& view);
      
   protected:
      friend class ConnectionListener;
      class ContainerListener : public ossimConnectableObjectListener
      {
      public:
         ContainerListener(ImageMdiSubWindow* w)
         :m_window(w)
         {
         }
         ImageMdiSubWindow* window(){return m_window;}
         void setWindow(ImageMdiSubWindow* w){m_window = w;}
         virtual void containerEvent(ossimContainerEvent& /* event */);
         
      protected:
         ImageMdiSubWindow* m_window;
      };
      void addItems();
      void removeItems();
      void addOrSetupToolbar();
      virtual void	closeEvent ( QCloseEvent * event ) ;    
      void addListeners();
      void removeListeners();
      virtual bool	event ( QEvent * event );      
      ossimImageGeometry* getView();
      
      ossimGui::ImageScrollView* m_imageScrollView;
      
      ImageActions* m_actions;
      ContainerListener* m_containerListener;
   };
}

#endif
