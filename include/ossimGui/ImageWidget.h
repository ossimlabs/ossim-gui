#ifndef ossimGuiImageWidget_HEADER
#define ossimGuiImageWidget_HEADER
#include <QtGui/QScrollArea>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>
#include <QtGui/QBitmap>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimCacheTileSource.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDrect.h>
#include <ossimGui/Export.h>
#include <ossimGui/Image.h>
#include <ossimGui/ConnectableImageObject.h>
#include <ossimGui/StaticTileImageCache.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/parallel/ossimJobQueue.h>

class QGraphicsView;
class QGraphicsScene;

namespace ossimGui
{
   class ImageWidget;
   class ImageScrollWidget;
   class RegistrationOverlay;
   
   class OSSIMGUI_DLL ImageWidgetJob : public ossimJob
   {
   public:
      ImageWidgetJob();
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
      void setCacheToViewTransform(const QTransform& trans)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         m_cacheToView = trans;
      }
      void setViewToCacheTransform(const QTransform& trans)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_jobMutex);
         m_viewToCache = trans;
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
      QTransform                        m_cacheToView;
      QTransform                        m_viewToCache;
      ossimRefPtr<StaticTileImageCache> m_tileCache;
      ossimRefPtr<ossimImageSource>     m_inputSource;
      OpenThreads::Mutex                m_imageWidgetJobMutex;
   };
   
   class OSSIMGUI_DLL ImageScrollWidget : public QScrollArea
   {
      Q_OBJECT
   public:  
      enum MultiLayerAlgorithmType
      {
         NO_ALGORITHM=0,
         HORIZONTAL_SWIPE_ALGORITHM,
         VERTICAL_SWIPE_ALGORITHM,
         BOX_SWIPE_ALGORITHM,
         CIRCLE_SWIPE_ALGORITHM,
         ANIMATION_ALGORITHM
      };
      
      friend class ImageWidgetJob;
      
      class OSSIMGUI_DLL ImageWidget : public QFrame
      {
      public:
         ImageWidget(ImageScrollWidget* scrollWidget, QWidget* parent=0);
         
         virtual void paintEvent(QPaintEvent *event);
      protected:
         ImageScrollWidget* m_scrollWidget;


      // QGraphicsScene* m_scene;
      // QGraphicsView* m_view;


      };
      
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
      ImageScrollWidget(QWidget* parent=0);
      virtual ~ImageScrollWidget();
      void setJobQueue(ossimJobQueue* jobQueue);
      void refreshDisplay();
      ossimIpt getViewportSize();
      void inputConnected(ossim_int32 idx    = -1);
      void inputDisconnected(ossim_int32 idx = -1);
      Layers* layers(){return m_layers.get();}
      ImageWidget* imageWidget(){return m_widget;}
      const ImageWidget* imageWidget()const{return m_widget;}
      const ossimDrect& inputBounds()const{return m_inputBounds;}
      const ossimDpt& scrollOrigin()const{return m_scrollOrigin;}
      ossimDrect viewportBoundsInViewSpace()const;
     // ossimDrect viewportBoundsInLocalSpace()const;
      void setPositionGivenView(const ossimDpt& position);
      void setPositionGivenLocal(const ossimDpt& position);
      void setTrackPoint(const ossimDpt& viewPoint);

      
      const QTransform& viewToScroll()const{return m_viewToScroll;}
      const QTransform& scrollToView()const{return m_scrollToView;}
      const QTransform& localToView()const{return m_localToView;}
      const QTransform& viewToLocal()const{return m_viewToLocal;}
      
      ossimImageGeometry* getGeometry();
      
      void setConnectableObject(ConnectableImageObject* c);
      ConnectableImageObject* connectableObject();
      
      void setMultiLayerAlgorithm(int algorithm){m_multiLayerAlgorithm = static_cast<MultiLayerAlgorithmType> (algorithm);}
      ossim_int32 multiLayerAlgorithmType()const{return m_multiLayerAlgorithm;}

      // Temporay multi-image development
      const ossimDpt& measPnt()const{return m_measPoint;}
      void updateAnnotation();
      void setMarkPointPosition(const ossimDpt& setViewPoint, const ossimDpt& setImagePoint);
      void storeCurrentPosition(const ossimString& id);
      bool getPoint(const ossimString& id, ossimDpt& imgPt);
      // Temporay multi-image development

   signals:
      void wheel(QWheelEvent* event,  const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      void mouseMove(QMouseEvent* event,   const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      void mousePress(QMouseEvent* event,  const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      void mouseRelease(QMouseEvent* event, const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      void mouseDoubleClick(QMouseEvent* event, const ossimDrect& viewportRectInViewSpace, const ossimDpt& viewPoint);
      
   protected:
      
      
      class ConnectionListener : public ossimConnectableObjectListener
      {
      public:
         ConnectionListener(ImageScrollWidget* widget=0)
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
               m_widget->setPositionGivenView(event.getPosition());
            }
         }
         ImageScrollWidget* m_widget;
      };  
      
   protected:
      class Callback : public ossimJobCallback
      {
      public:
         Callback(ImageScrollWidget* w):m_imageScrollWidget(w){}
         virtual void started(ossimJob* job)
         {
            ImageWidgetJob* imageWidgetJob = dynamic_cast<ImageWidgetJob*>(job);
            if(imageWidgetJob)
            {
               ossimRefPtr<Layer> layer = m_imageScrollWidget->m_layers->findFirstDirtyLayer();
               if(layer.valid())
               {
                  imageWidgetJob->setTileCache(layer->tileCache());
                  imageWidgetJob->setInputSource(layer->chain());
               }
               imageWidgetJob->setCacheToViewTransform(m_imageScrollWidget->scrollToView());
               imageWidgetJob->setViewToCacheTransform(m_imageScrollWidget->viewToScroll());
            }
         }
         virtual void finished(ossimJob* job)
         {
            ImageWidgetJob* imageWidgetJob = dynamic_cast<ImageWidgetJob*>(job);
            if(imageWidgetJob)
            {
               m_imageScrollWidget->m_widget->update();
               if(m_imageScrollWidget)
               {
                  ossimRefPtr<Layer> layer = m_imageScrollWidget->m_layers->findFirstDirtyLayer();
                  if(layer.valid())
                  {
                     imageWidgetJob->ready();
                     m_imageScrollWidget->m_jobQueue->add(job);
                  }
               }
            }
         }

         ImageScrollWidget* m_imageScrollWidget;
      };
      friend class Callback;
      //virtual void timerEvent(QTimerEvent* event);
      virtual void resizeEvent(QResizeEvent* event);
      virtual void	scrollContentsBy ( int dx, int dy );     
      void updateScrollBars();
      void setCacheRect();
      virtual void	mouseDoubleClickEvent ( QMouseEvent * e );
      virtual void	mouseMoveEvent ( QMouseEvent * e );
      virtual void	mousePressEvent ( QMouseEvent * e );
      virtual void	mouseReleaseEvent ( QMouseEvent * e );
      virtual void	wheelEvent ( QWheelEvent * e );      
      virtual void	enterEvent ( QEvent * event );      
      virtual void	leaveEvent ( QEvent * event );     
      virtual void paintWidget(QPainter& painter);
      virtual void drawCursor(QPainter& painter);
      virtual void paintMultiLayer(QPainter& painter);
      virtual void updateTransforms();
      mutable ossimRefPtr<ConnectableImageObject> m_connectableObject;
      
      ConnectionListener* m_listener;
      ossimDrect m_inputBounds;
      ossimDpt m_scrollOrigin;
      ImageWidget* m_widget;
      ossimIpt m_tileSize;
      ossim_int32 m_timerId;
      ossimRefPtr<ossimJobQueue> m_jobQueue;
      ossimRefPtr<ImageWidgetJob> m_imageWidgetJob;
      
      ossimRefPtr<Layers> m_layers;

      // ossimGui::RegistrationOverlay* m_regOverlay;
   
      ossimDpt                  m_trackPoint;
      ossimDpt                  m_oldTrackPoint;
      bool                      m_trackingFlag;
      bool                      m_mouseInsideFlag;
      
      QPoint                    m_activePointStart;
      QPoint                    m_activePointEnd;
      
      MultiLayerAlgorithmType   m_multiLayerAlgorithm;
      ossim_int32               m_activeIndex;
      QTransform                m_viewToScroll;
      QTransform                m_scrollToView;
      QTransform                m_scrollToLocal;
      QTransform                m_localToScroll;
      
      QTransform                m_viewToLocal;
      QTransform                m_localToView;
      
      ossimDpt                  m_measPoint;
      ossimDpt                  m_measImgPoint;
      ossimDpt                  m_markPoint;
      ossimDpt                  m_markImgPoint;
      bool                      m_drawPts;

      // QGraphicsScene* m_scene;
      // QGraphicsView* m_view;
   };
   
}   
#endif
