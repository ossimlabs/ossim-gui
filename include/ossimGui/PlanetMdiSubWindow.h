#ifndef ossimGuiPlanetMdiSubWindow_HEADER
#define ossimGuiPlanetMdiSubWindow_HEADER
#include <ossimGui/MdiSubWindowBase.h>
#include <ossimGui/ConnectableDisplayObject.h>
#include <ossimGui/Export.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimVisitor.h>
#include <QtGui/QMouseEvent>
#include <ossimGui/GlWidget.h>
#include <ossimPlanet/ossimPlanetViewMatrixBuilder.h>
#include <ossimPlanet/ossimPlanetTextureLayerGroup.h>
#include <osg/ref_ptr>
#include <ossim/base/ossimViewInterface.h>



class QMenu;
class QToolBar;
class QMenuBar;
class QMainWindow;
class ossimConnectableObject;
namespace ossimGui {
   class ImageScrollWidget;

   class OSSIMGUI_DLL PlanetMdiSubWindow : public MdiSubWindowBase
   {
      Q_OBJECT
   public:
      typedef std::map<ossimRefPtr<ossimObject>, osg::ref_ptr<ossimPlanetTextureLayer> > ChainToTextureType;
      PlanetMdiSubWindow( QWidget * parent = 0, Qt::WindowFlags flags = 0);
      virtual ~PlanetMdiSubWindow();
      
      ossimGui::ImageScrollWidget* scrollWidget();
      virtual void sync(View& syncInfo);
      virtual void setConnectableObject(ConnectableObject* connectable);
      virtual void setPlanet(ossimPlanet* planet){m_planet = planet;}
      
   public slots:
      void stateChanged(Qt::WindowStates oldState, Qt::WindowStates newState);
      void syncView(View& view);
      
   protected:
      class OSSIMGUI_DLL InputListener : public ossimConnectableObjectListener
      {
      public:
         InputListener(PlanetMdiSubWindow* w)
         :m_window(w)
         {
         }
         PlanetMdiSubWindow* window(){return m_window;}
         void setWindow(PlanetMdiSubWindow* w){m_window = w;}
         virtual void disconnectInputEvent(ossimConnectionEvent& /* event */);
         virtual void connectInputEvent(ossimConnectionEvent& /* event */);
         
      protected:
         PlanetMdiSubWindow* m_window;
      };
      friend class ConnectionListener;
      
      // for now we will overide the standard close and make sure we only hide 
      // by doing the default accept.
      //
      void closeEvent ( QCloseEvent * event );

      virtual bool	event(QEvent* evt);
      
      GlViewer*                             m_planetViewer;
      osg::ref_ptr<ossimPlanet>             m_planet;
      osg::ref_ptr<ossimPlanetManipulator>  m_manipulator; 
      osg::ref_ptr<ossimPlanetTextureLayerGroup> m_textureLayers;
      ChainToTextureType                         m_chainToTextureMap;
      
      InputListener* m_inputListener;
   };
}

#endif
