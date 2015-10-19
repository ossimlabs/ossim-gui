#ifndef ossimGuiImageScrollView_HEADER
#define ossimGuiImageScrollView_HEADER 1
#include <QtGui/QGraphicsView>
#include <QtGui/QScrollArea>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>
#include <QtGui/QBitmap>
#include <QtGui/QRubberBand>
#include <ossimGui/Export.h>
#include <ossimGui/ConnectableImageObject.h>
#include <ossimGui/Image.h>
#include <ossimGui/StaticTileImageCache.h>
#include <ossimGui/ImageViewManipulator.h>
#include <ossimGui/DataManager.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimCacheTileSource.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/parallel/ossimJobQueue.h>
#include <vector>

namespace ossimGui
{
   class RegistrationOverlay;
   class MetricOverlay;

   class OSSIMGUI_DLL ImageViewJob : public ossimJob
   {
   public:
      ImageViewJob();
      virtual void start();
      void setMaxProcessingTimeInMillis(ossim_float64 t)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         m_maxProcessingTime = t;
      }
      ossim_float64 maxProcessingTimeInMillis()const
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         return m_maxProcessingTime;
      }
      void setTileCache(StaticTileImageCache* cache)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         m_tileCache = cache;
      }
      StaticTileImageCache* tileCache()
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         return m_tileCache.get();
      }
      void setInputSource(ossimImageSource* input)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         m_inputSource = input;
      }
      
   protected:
      ossim_float64                     m_maxProcessingTime;
      ossimRefPtr<StaticTileImageCache> m_tileCache;
      ossimRefPtr<ossimImageSource>     m_inputSource;
      OpenThreads::Mutex                m_imageViewJobMutex;
   };
   
   class OSSIMGUI_DLL ImageScrollView : public QGraphicsView
   {
   Q_OBJECT
   public:
      friend class ImageViewManipulator;
      enum MultiLayerAlgorithmType
      {
         NO_ALGORITHM=0,
         HORIZONTAL_SWIPE_ALGORITHM,
         VERTICAL_SWIPE_ALGORITHM,
         BOX_SWIPE_ALGORITHM,
         CIRCLE_SWIPE_ALGORITHM,
         ANIMATION_ALGORITHM
      };
      
      friend class ImageViewJob;
      class ConnectionListener : public ossimConnectableObjectListener
      {
      public:
         ConnectionListener(ImageScrollView* widget=0)
            :m_widget(widget)
         {
         }
         virtual void objectDestructingEvent(ossimObjectDestructingEvent& /*event*/)
         {
         }
         virtual void disconnectInputEvent(ossimConnectionEvent& /* event */)
         {
            if(m_widget)
            {
               m_widget->inputDisconnected();
            }
            
         }
         virtual void disconnectOutputEvent(ossimConnectionEvent& /* event */)
         {
         }
         virtual void connectInputEvent(ossimConnectionEvent& /* event */)
         {
            if(m_widget)
            {
               m_widget->inputConnected();
            }
         }
         virtual void connectOutputEvent(ossimConnectionEvent& /* event */)
         {
         }
         
         virtual void propertyEvent(ossimPropertyEvent& /* event */)
         {
            m_widget->refreshDisplay();
         }
         
         /*!
          * Typically isued by objects that contain children.  If anyone is
          * interested, can latch on to this event.  Other objects within the
          * system might be interest in this event even 
          */
         virtual void addObjectEvent(ossimContainerEvent& /* event */)
         {}
         
         virtual void removeObjectEvent(ossimContainerEvent& /* event */)
         {}
         
         virtual void refreshEvent(ossimRefreshEvent& event)
         {
            int refreshType = event.getRefreshType();
            if((refreshType & ossimRefreshEvent::REFRESH_PIXELS)||
               (refreshType & ossimRefreshEvent::REFRESH_GEOMETRY))
            {
               m_widget->refreshDisplay();
            }
            if(refreshType & ossimRefreshEvent::REFRESH_POSITION)
            {
               m_widget->centerOn(event.getPosition().x, event.getPosition().y);
            }
         }
         ImageScrollView* m_widget;
      };  
      class Callback : public ossimJobCallback
      {
      public:
         Callback(ImageScrollView* w):m_imageScrollWidget(w){}
         virtual void started(ossimJob* job)
         {
            ImageViewJob* imageViewJob = dynamic_cast<ImageViewJob*>(job);
            if(imageViewJob)
            {
               ossimRefPtr<Layer> layer = m_imageScrollWidget->m_layers->findFirstDirtyLayer();
               if(layer.valid())
               {
                  imageViewJob->setTileCache(layer->tileCache());
                  imageViewJob->setInputSource(layer->chain());
               }
            }
         }
         virtual void finished(ossimJob* job)
         {
            ImageViewJob* imageViewJob = dynamic_cast<ImageViewJob*>(job);
            if(imageViewJob)
            {
               m_imageScrollWidget->viewport()->update();
               if(m_imageScrollWidget)
               {
                  ossimRefPtr<Layer> layer = m_imageScrollWidget->m_layers->findFirstDirtyLayer();
                  if(layer.valid())
                  {
                     imageViewJob->ready();
                     m_imageScrollWidget->m_jobQueue->add(job);
                  }
               }
            }
         }
         
         ImageScrollView* m_imageScrollWidget;
      };
      friend class Callback;
      
      class OSSIMGUI_DLL Layer : public ossimReferenced
      {
      public:
         Layer(ossimConnectableObject* obj=0);
         virtual ~Layer();
         void clear()
         {
            m_inputObject = 0;
            m_tileCache = 0;
            if(m_scalarRemapperChain.valid()) m_scalarRemapperChain->disconnect();
            m_scalarRemapperChain = 0;
         }
         StaticTileImageCache* tileCache(){return m_tileCache.get();}
         const StaticTileImageCache* tileCache()const{return m_tileCache.get();}
         
         ossimConnectableObject* inputSource(){return m_inputObject.get();}
         const ossimConnectableObject* inputSource()const{return m_inputObject.get();}
         
         ossimImageSource* chain(){return m_scalarRemapperChain.get();}
         const ossimImageSource* chain()const{return m_scalarRemapperChain.get();}
         
         ossimRefPtr<ossimConnectableObject> m_inputObject;
         ossimRefPtr<ossimImageChain>        m_scalarRemapperChain;
         ossimRefPtr<StaticTileImageCache>   m_tileCache;
      };
      class Layers : public ossimReferenced
      {
      public:
         typedef std::vector<ossimRefPtr<Layer> > LayerListType;
         class OSSIMGUI_DLL FindConnectable
         {
         public:
            FindConnectable(ossimConnectableObject* obj):m_connectable(obj){}
            bool operator()(const ossimRefPtr<Layer>& layer)
            {
               return (layer->m_inputObject.get() == m_connectable);
            }
            ossimConnectableObject* m_connectable;
         };
         
         Layers();
         virtual ~Layers();
         Layer* layer(ossim_uint32 idx);
         Layer* layer(ossimConnectableObject* input);
         bool isEmpty()const;
         void adjustLayers(ossimConnectableObject* connectable);
         Layer* findFirstDirtyLayer();
         void setCacheRect(const ossimDrect& rect);
         void flushDisplayCaches();
         ossim_uint32 numberOfLayers()const
         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
            return m_layers.size();
         }
         
      protected:
         Layer* layerNoMutex(ossim_uint32 idx);
         Layer* layerNoMutex(ossimConnectableObject* input);
         LayerListType m_layers;
         
         mutable OpenThreads::Mutex m_mutex;
      };
      
      ImageScrollView ( QWidget * parent = 0 );
      ImageScrollView ( QGraphicsScene * scene, QWidget * parent = 0 );
      ~ImageScrollView ();	
      void setManipulator(ImageViewManipulator* manipulator);
      ImageViewManipulator* manipulator();
      
      void setConnectableObject(ConnectableImageObject* c);
      ConnectableImageObject* connectableObject();

      ossimImageGeometry* getGeometry();

      /**
       * @brief Gets rgb values from viewport.
       */
      void getRgb(const ossimIpt& location,
                  ossim_uint8& r,
                  ossim_uint8& g,
                  ossim_uint8& b);

      /**
       * @brief Gets the raw pixel values at the image handler level.
       * Only works on single image chains.  If the input fed from multiple
       * inputs the call will do nothing.
       */
      void getRaw(const ossimIpt& location,
                  std::vector<ossim_float64>& values);
      
      void setShowTrackCursor(bool flag){m_showTrackingCursorFlag=flag;}
      bool showTrackCursor()const{return m_showTrackingCursorFlag;}
      const ossimDpt& trackPoint()const;
      void setTrackPoint(const ossimDpt& position);
      void setJobQueue(ossimJobQueue* jobQueue);
      void refreshDisplay();
      void setMultiLayerAlgorithm(int algorithm){m_multiLayerAlgorithm = static_cast<MultiLayerAlgorithmType> (algorithm);}
      ossim_int32 multiLayerAlgorithmType()const{return m_multiLayerAlgorithm;}
      void setExploitationMode(int expMode);
      void setAutoMeasActive(const bool state);
      ossim_int32 exploitationMode()const{return m_exploitationMode;}
      ossimDrect viewportBoundsInSceneSpace()const;
      Layers* layers(){return m_layers.get();}
      const ossimDrect& getInputBounds()const{return m_inputBounds;}
      const ossimDpt& getLastClickedPoint()const{return m_lastClickedPoint;}
      void setLastClickedPoint(const ossimDpt& position) {m_lastClickedPoint = position;}
      void setPositionGivenView(const ossimDpt& position);
      virtual void mouseDoubleClickEvent ( QMouseEvent * e );
      virtual void mouseMoveEvent ( QMouseEvent * e );
      virtual void mousePressEvent ( QMouseEvent * e );
      virtual void mouseReleaseEvent ( QMouseEvent * e );
      virtual void wheelEvent ( QWheelEvent * e );      
      virtual void enterEvent ( QEvent * event );      
      virtual void leaveEvent ( QEvent * event );     
      virtual void keyPressEvent ( QKeyEvent * event );
      virtual void keyReleaseEvent ( QKeyEvent * event );
      
      // Currently called by ImageViewManipulator zoom functions
      void zoomAnnotation();
      
      // Overlay access
      ossimGui::RegistrationOverlay* regOverlay()const{return m_regOverlay;}
      ossimGui::MetricOverlay* metOverlay()const{return m_metricOverlay;}

      void emitViewChanged();
      
   signals:
      
      void wheel(QWheelEvent* event,  const ossimDrect& viewSceneRect, const ossimDpt& scenePoint);
      void mouseMove(QMouseEvent* event,   const ossimDrect& viewSceneRect, const ossimDpt& scenePoint);
      void mouseMove(QMouseEvent* event);
      void mouseDoubleClick(QMouseEvent* event, const ossimDrect& viewSceneRect, const ossimDpt& scenePoint);
      void mouseRelease(QMouseEvent* event, const ossimDrect& viewSceneRect, const ossimDpt& scenePoint);
      void mouseRelease(QMouseEvent* event);
      void track(const ossimDpt& scenePoint);
      void mousePress(QMouseEvent* event,  const ossimDrect& viewSceneRect, const ossimDpt& scenePoint);
      void mousePress(QMouseEvent* event, const ossimDpt& scenePoint);
      void mousePress(ImageScrollView* sptr, const ossimDpt& scenePoint);
      void mousePress(QMouseEvent* event);      
      void mouseBox(ImageScrollView* sptr, const ossimDpt& startPoint, const ossimDpt& stopPoint);

      void paintYourGraphics(QPainter* p, const QRectF& rect);

      void viewChanged();

   protected:
      
      virtual void resizeEvent(QResizeEvent* event);
      virtual void scrollContentsBy( int dx, int dy );
      void setCacheRect();
      void inputConnected(ossim_int32 idx    = -1);
      void inputDisconnected(ossim_int32 idx = -1);
      virtual void drawBackground ( QPainter * painter, const QRectF & rect );
      virtual void drawForeground ( QPainter * painter, const QRectF & rect );
      void paintMultiLayer(QPainter& painter, const QRectF & rect);
      void updateSceneRect();
      void emitTracking(const ossimDpt& pt);
      
      ossimDpt                          m_lastClickedPoint;
      ossimDpt                          m_trackPoint;
      ossimDpt                          m_oldTrackPoint;
      bool                              m_trackingFlag;
      bool                              m_mouseInsideFlag;
      bool                              m_showTrackingCursorFlag;
      bool                              m_amDialogActive;
      
      QPoint                            m_mouseStartPoint;
      QPointF                           m_activePointStart;
      QPointF                           m_activePointEnd;
      ossimRefPtr<ImageViewJob>         m_imageViewJob;
      ossimRefPtr<Layers>               m_layers;
      ConnectionListener*               m_listener;
      ossimRefPtr<ossimJobQueue>        m_jobQueue;
      ossimDrect                        m_inputBounds;
      MultiLayerAlgorithmType           m_multiLayerAlgorithm;
      DataManager::ExploitationModeType m_exploitationMode;
      
      ossimRefPtr<ImageViewManipulator> m_manipulator;
      mutable ossimRefPtr<ConnectableImageObject> m_connectableObject;
      
      RegistrationOverlay*              m_regOverlay;
      MetricOverlay*                    m_metricOverlay;
      ossimString                       m_roiId;
      
   }; // End: class ImageScrollView
}

#endif /* #ifndef ossimGuiImageScrollView_HEADER */
