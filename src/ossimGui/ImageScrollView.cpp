
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/DisplayTimerJobQueue.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>
#include <ossimGui/MetricOverlay.h>
#include <ossimGui/RegistrationOverlay.h>
#include <ossimGui/RegPoint.h>
#include <ossimGui/RoiSelection.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageData.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageSource.h>
#include <QtCore/QTime>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QGraphicsRectItem>

namespace ossimGui
{
ImageViewJob::ImageViewJob()
   : m_maxProcessingTime(20),
     m_tileCache(0),
     m_inputSource(0),
     m_imageViewJobMutex()
{
}

void ImageViewJob::start()
{
   if(m_inputSource.valid())
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_imageViewJobMutex);
      QTime start = QTime::currentTime();
      ossimDrect cacheRect(m_tileCache->getRect());
      // ossimDpt ulCachePt = cacheRect.ul();
      ossimIrect rect;
      //ossimDpt ul;      
      // Because the cache rect is a sub rect of the scroll we need the upper left which starts at offset 0,0 in scroll space
      //m_cacheToView.map(0.0, 0.0, &ul.x, &ul.y);
      while(m_tileCache->nextInvalidTile(rect) && (!isCanceled()))
      {
         // shift to zero based rectangle and then set back for opying purposes.
         ossimRefPtr<ossimImageData> data =m_inputSource->getTile(rect);
         data->setImageRectangle(rect);
         ossimGui::Image img(data.get());
         if(data.valid())
         {
            m_tileCache->addTile(ossimGui::Image(data.get(), true));
         }
         else 
         {
            img = QImage(rect.width(), rect.height(),  QImage::Format_RGB32);
            img.fill(0);
            img.setOffset(QPoint(rect.ul().x, rect.ul().y));
            
            m_tileCache->addTile(img);
         }
         
         QTime end = QTime::currentTime();
         if(start.msecsTo(end) >= m_maxProcessingTime)
         {
            break;
         }
      }
   }
}

ImageScrollView::Layer::Layer(ossimConnectableObject* obj)
   : m_inputObject(obj),
     m_scalarRemapperChain( new ossimImageChain() ),
     m_tileCache( new StaticTileImageCache() )
{
   m_scalarRemapperChain->addFirst(new ossimScalarRemapper());
   m_scalarRemapperChain->addFirst(new ossimCacheTileSource());
   if(obj) m_scalarRemapperChain->connectMyInputTo(0, obj);
}

ImageScrollView::Layer::~Layer()
{
   clear();
}

ImageScrollView::Layers::Layers()
   : m_layers(0),
     m_mutex()
{
}

ImageScrollView::Layers::~Layers()
{
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      m_layers[idx]->clear();
      m_layers[idx] = 0;
   }
   m_layers.clear();
}

ImageScrollView::Layer* ImageScrollView::Layers::layer(ossim_uint32 idx)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return layerNoMutex(idx);
}

ImageScrollView::Layer* ImageScrollView::Layers::layer(ossimConnectableObject* input)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return layerNoMutex(input);
}

void  ImageScrollView::Layers::setCacheRect(const ossimDrect& rect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      m_layers[idx]->tileCache()->setRect(rect);
   }
}

ImageScrollView::Layer* ImageScrollView::Layers::layerNoMutex(ossim_uint32 idx)
{
   Layer* result = 0;
   if(idx < m_layers.size())
   {
      return m_layers[idx].get();
   }
   
   return result;
}

ImageScrollView::Layer* ImageScrollView::Layers::layerNoMutex(ossimConnectableObject* input)
{
   Layer* result = 0;
   LayerListType::iterator iter = std::find_if(m_layers.begin(), m_layers.end(), FindConnectable(input));
   if(iter != m_layers.end())
   {
      result = (*iter).get();
   }
   
   return result;
}

ImageScrollView::Layer* ImageScrollView::Layers::findFirstDirtyLayer()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size();++idx)
   {
      if(m_layers[idx]->tileCache()->hasInvalidTiles())
      {
         return m_layers[idx].get();
      }
   }
   
   return 0;
}

bool ImageScrollView::Layers::isEmpty()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return m_layers.empty();
}

void ImageScrollView::Layers::adjustLayers(ossimConnectableObject* connectable)
{
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
      LayerListType layers;
      ossim_uint32 nInputs = connectable->getNumberOfInputs();
      for(ossim_uint32 inputIdx = 0; inputIdx<nInputs;++inputIdx)
      {
         ossimRefPtr<ossimConnectableObject> inputObj = connectable->getInput(inputIdx);
         ossimRefPtr<Layer> tempLayer = layerNoMutex(inputObj.get());
         if(tempLayer.valid())
         {
            layers.push_back(tempLayer.get());
         }
         else // allocate a new display layer
         {
            tempLayer = new Layer(inputObj.get());
            layers.push_back(tempLayer.get());
         }
      }
      // Now any old layers that were removed lets fully remove
      //
      LayerListType::iterator iter = m_layers.begin();
      while(iter!=m_layers.end())
      {
         if(std::find(layers.begin(), layers.end(), (*iter).get())==layers.end())
         {
            (*iter)->clear();
         }
         ++iter;
      }
      
      m_layers = layers;
      // ossim_uint32 idx = 0;
   }
}

void ImageScrollView::Layers::flushDisplayCaches()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      if(m_layers[idx]->tileCache()) m_layers[idx]->tileCache()->flush();
   }
}

ImageScrollView::ImageScrollView (QWidget* parent)
   : QGraphicsView(parent),
     m_lastClickedPoint(),
     m_trackPoint(),
     m_oldTrackPoint(),
     m_trackingFlag( true ),
     m_mouseInsideFlag( false ),
     m_showTrackingCursorFlag( false ),
     m_amDialogActive( false ),
     m_mouseStartPoint(),
     m_activePointStart(),
     m_activePointEnd(),
     m_imageViewJob( new ImageViewJob() ),
     m_layers( new Layers() ),
     m_listener( new ConnectionListener(this) ),
     m_jobQueue( new DisplayTimerJobQueue() ),
     m_inputBounds(),
     m_multiLayerAlgorithm( BOX_SWIPE_ALGORITHM ),
     m_exploitationMode( DataManager::NO_MODE ),
     m_manipulator(0),
     m_connectableObject(0),
     m_regOverlay(0),
     m_metricOverlay(0),
     m_roiId("RegROI")
{
   setScene(new QGraphicsScene());
   m_lastClickedPoint.makeNan();
   m_trackPoint.makeNan();
   m_oldTrackPoint.makeNan();
   m_inputBounds.makeNan();
   m_imageViewJob->setCallback(new Callback(this));
   m_manipulator = new ImageViewManipulator(this);
   viewport()->setCursor(Qt::CrossCursor);
   m_regOverlay = new RegistrationOverlay("Reg", scene());
   m_metricOverlay = new MetricOverlay("Met", scene());

   // this->setBackgroundRole(QPalette::Window);
   // setViewport(new QGLWidget());
   // m_layers = new Layers();
   // m_imageViewJob = new ImageViewJob();
   // m_imageViewJob->setCallback(new Callback(this));
   // m_multiLayerAlgorithm = BOX_SWIPE_ALGORITHM;
   // m_trackPoint.makeNan();
   // m_oldTrackPoint.makeNan();
   // m_trackingFlag = true;
   // m_mouseInsideFlag = false;
   // m_showTrackingCursorFlag = false;
}

ImageScrollView::ImageScrollView (QGraphicsScene* scene, QWidget* parent)
   : QGraphicsView(scene, parent),
     m_lastClickedPoint(),
     m_trackPoint(),
     m_oldTrackPoint(),
     m_trackingFlag( true ),
     m_mouseInsideFlag( false ),
     m_showTrackingCursorFlag( false ),
     m_amDialogActive( false ),     
     m_mouseStartPoint(),
     m_activePointStart(),
     m_activePointEnd(),
     m_imageViewJob( new ImageViewJob() ),
     m_layers( new Layers() ),
     m_listener( new ConnectionListener(this) ),
     m_jobQueue( new DisplayTimerJobQueue() ),
     m_inputBounds(),
     m_multiLayerAlgorithm( BOX_SWIPE_ALGORITHM ),
     m_exploitationMode( DataManager::NO_MODE ),
     m_manipulator(0),
     m_connectableObject(0),
     m_regOverlay(0),
     m_metricOverlay(0),
     m_roiId("RegROI")
{
   m_lastClickedPoint.makeNan();
   m_trackPoint.makeNan();
   m_oldTrackPoint.makeNan();
   m_inputBounds.makeNan();
   m_imageViewJob->setCallback(new Callback(this));
   m_manipulator = new ImageViewManipulator(this);
   viewport()->setCursor(Qt::CrossCursor);
   m_regOverlay = new RegistrationOverlay( "Reg", scene );
   m_metricOverlay = new MetricOverlay("Met", scene);
   
   //setViewport(new QGLWidget());
   // m_layers = new Layers();
   // m_imageViewJob = new ImageViewJob();
   // m_imageViewJob->setCallback(new Callback(this));
   // m_multiLayerAlgorithm = BOX_SWIPE_ALGORITHM;
   // m_trackPoint.makeNan();
   // m_oldTrackPoint.makeNan();
   // m_trackingFlag = true;
   // m_mouseInsideFlag = false;
   // m_showTrackingCursorFlag = false;
   // m_manipulator = new ImageViewManipulator(this);
   // viewport()->setCursor(Qt::CrossCursor);
   // m_regOverlay = new RegistrationOverlay("Reg", scene);
   // m_metricOverlay = new MetricOverlay("Met", scene);
}

ImageScrollView::~ImageScrollView()
{
   m_manipulator->setImageScrollView(0);
   m_imageViewJob->cancel();
   if(m_connectableObject.get()&&m_listener)
   {
      if(m_listener)
      {
         m_connectableObject->removeListener(m_listener);
      }
      m_connectableObject->disconnect();
   }
   if(m_listener)
   {
      delete m_listener;
      m_listener = 0;
   }
   m_connectableObject = 0;
   m_layers = 0;
   if(m_manipulator.valid())
   {
   }
}
void ImageScrollView::setManipulator(ImageViewManipulator* manipulator)
{
   if(m_manipulator.valid())
   {
      m_manipulator->setImageScrollView(0);
   }
   m_manipulator = manipulator;
   manipulator->setImageScrollView(this);
}

ImageViewManipulator* ImageScrollView::manipulator()
{
   return m_manipulator.get();
}

void ImageScrollView::setConnectableObject(ConnectableImageObject* c)
{
   if(m_connectableObject.valid())
   {
      m_connectableObject->removeListener(m_listener);
   }
   m_connectableObject = c;
   if(m_connectableObject.valid())
   {
      m_connectableObject->addListener(m_listener);
   }
   if(m_connectableObject.valid()) inputConnected();
}

void ImageScrollView::refreshDisplay()
{
   m_layers->flushDisplayCaches();
   m_inputBounds = m_connectableObject->getBounds();
   updateSceneRect();
      
   if(m_jobQueue.valid())
   {
      if(!m_imageViewJob->isRunning()) m_imageViewJob->ready();

      m_jobQueue->add(m_imageViewJob.get());
   }
}
   
ossimDrect ImageScrollView::viewportBoundsInSceneSpace()const
{
   QRectF r = mapToScene(viewport()->rect()).boundingRect();
   return ossimDrect(r.x(),r.y(),r.x()+r.width()-1,r.y()+r.height()-1);
}
   
void ImageScrollView::setJobQueue(ossimJobQueue* jobQueue)
{
   m_jobQueue = jobQueue;
}
   
void ImageScrollView::inputConnected(ossim_int32 /* idx */)
{
   m_layers->adjustLayers(m_connectableObject.get());
   m_inputBounds = m_connectableObject->getBounds();
   updateSceneRect();
   if(m_connectableObject->getNumberOfInputs() == 1)
   {
      ossimDpt midPt = m_inputBounds.midPoint();
      centerOn(midPt.x, midPt.y);
      
      if(m_manipulator.valid())
      {
         m_manipulator->initializeToCurrentView();
      }
   }
   if(m_jobQueue.valid())
   {
      if(!m_imageViewJob->isRunning()) m_imageViewJob->ready();
      m_jobQueue->add(m_imageViewJob.get());
   }
}
   
void ImageScrollView::inputDisconnected(ossim_int32 /* idx */)
{
   m_layers->adjustLayers(m_connectableObject.get());
   
   m_inputBounds = m_connectableObject->getBounds();
   
   updateSceneRect();
   
   if(m_jobQueue.valid())
   {
      if(!m_imageViewJob->isRunning()) m_imageViewJob->ready();
      m_jobQueue->add(m_imageViewJob.get());
   }
}

void ImageScrollView::setCacheRect()
{
   QRectF r = mapToScene(viewport()->rect()).boundingRect();
   
   
   ossimIpt ul(r.x(), r.y());//origin.x(), origin.y());
   ossimIpt lr(r.x()+ r.width()-1,
               r.y()+ r.height()-1);
   
   ossimIrect rect(ul.x, ul.y, lr.x, lr.y);
   m_layers->setCacheRect(rect);
}

   
void ImageScrollView::resizeEvent(QResizeEvent* event)
{
   QGraphicsView::resizeEvent(event);
   if(!m_inputBounds.hasNans())
   {
      setCacheRect();
   }
   if(m_layers->findFirstDirtyLayer())
   {
      if(m_jobQueue.valid())
      {
         if(!m_imageViewJob->isRunning()) m_imageViewJob->ready();
         m_jobQueue->add(m_imageViewJob.get());
      }
   }
   if(m_manipulator.valid())
   {
      m_manipulator->resizeEvent(event);
   }
}
   
void ImageScrollView::scrollContentsBy( int dx, int dy )
{
   QGraphicsView::scrollContentsBy( dx,dy );
   
   if(m_manipulator.valid()) 
   {
      m_manipulator->scrollContentsBy(dx, dy);
   }
   if(!m_inputBounds.hasNans())
   {
      setCacheRect();
   }
   if(m_layers->findFirstDirtyLayer())
   {
      if(m_jobQueue.valid())
      {
         if(!m_imageViewJob->isRunning()) m_imageViewJob->ready();
         m_jobQueue->add(m_imageViewJob.get());
      }
   }
}

ConnectableImageObject* ImageScrollView::connectableObject()
{
   return m_connectableObject.get();
}

ossimImageGeometry* ImageScrollView::getGeometry()
{
   ossimImageGeometry* result = 0;
   if ( m_connectableObject.get() )
   {
      ossimImageSource* is = dynamic_cast<ossimImageSource*>(m_connectableObject->getInput());
      if(is)
      {
         result = is->getImageGeometry().get();
      }
   }
   return result;
}

void ImageScrollView::getRgb(const ossimIpt& location,
                             ossim_uint8& r,
                             ossim_uint8& g,
                             ossim_uint8& b)
{
   if ( m_layers.get() )
   {
      ossimRefPtr<Layer> topLayer = m_layers->layer( (ossim_uint32)0 );
      if( topLayer.valid() )
      {
         ossimRefPtr<StaticTileImageCache> topTileCache = topLayer->tileCache();
         if ( topTileCache.valid() )
         {
            // Scene to QImage offset:
            ossimIpt offset = topTileCache->getRect().ul();

            // QImage point:
            ossimIpt pt(location.x - offset.x, location.y - offset.y);

            QColor rgb = topTileCache->getCache().pixel( pt.x, pt.y );
            r = rgb.red();
            g = rgb.green();
            b = rgb.blue();
         }
      }
   }
}

void ImageScrollView::getRaw(const ossimIpt& location,
                             std::vector<ossim_float64>& values)
{
   if ( m_connectableObject.get() )
   {
      ossimImageSource* is = dynamic_cast<ossimImageSource*>(m_connectableObject->getInput());
      if(is)
      {
         // Find the image handlers in the chain
         ossimTypeNameVisitor visitor(ossimString("ossimImageHandler"),
                                      true,
                                      ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
         is->accept(visitor);
         
         // If there are multiple image handlers, e.g. a mosaic do not uses.
         if ( visitor.getObjects().size() == 1 )
         {
            ossimRefPtr<ossimImageHandler> ih = visitor.getObjectAs<ossimImageHandler>( 0 );
            if ( ih.valid() )
            {
               ossimIrect rect(location.x, location.y, location.x+1, location.y+1);
               ossimRefPtr<ossimImageData> id = ih->getTile(rect, 0);
               
               if ( id.valid() )
               {
                  const ossim_uint32 BANDS = id->getNumberOfBands();
                  
                  values.resize( BANDS );
                  for( ossim_uint32 i = 0; i < BANDS; ++i )
                  {
                     values[i] = id->getPix( location, i );
                  }
               }
            }
         }
      }
   }
}
   
const ossimDpt& ImageScrollView::trackPoint()const
{
   return m_trackPoint;
}

void ImageScrollView::setTrackPoint(const ossimDpt& position)
{
   if(position.hasNans())
   {
      m_trackPoint.makeNan();
   }
   else 
   {
      //QPointF p = mapFromScene(position.x, position.y);
      m_trackPoint = position;//ossimDpt(p.x(),p.y());
      // on windows we can't call just update we must call viewport()->update
      if(!m_mouseInsideFlag) viewport()->update();//update();
   }
}

void ImageScrollView::updateSceneRect()
{
   if(!m_inputBounds.hasNans())
   {
      //resetTransform();
      //resetMatrix();
      //setTransform(QTransform(1.0,0.0,0.0,1.0,-m_inputBounds.ul().x, -m_inputBounds.ul().y));
      
      setSceneRect(m_inputBounds.ul().x, 
                   m_inputBounds.ul().y,
                   m_inputBounds.width(),
                   m_inputBounds.height());
      setCacheRect();
   }
   else
   {
      setSceneRect(0,0,0,0);
   }
}

void ImageScrollView::drawBackground(QPainter* painter, 
                                     const QRectF& rect )
{
   if((m_layers->numberOfLayers() > 1)&&(m_multiLayerAlgorithm!=NO_ALGORITHM))
   {
      paintMultiLayer(*painter, rect);
   }
   else 
   {
      ossimRefPtr<Layer> topLayer = m_layers->layer((ossim_uint32)0);
      if(topLayer.valid())
      {
         ossimRefPtr<StaticTileImageCache> topTileCache = topLayer->tileCache();
         if(topTileCache.valid())
         {
            ossimIrect irect          = topTileCache->getRect();
            ossimIpt topOriginOffset = ossimDpt(irect.ul().x, 
                                                irect.ul().y);
            //std::cout << "CACHE === " << topOriginOffset.x<<","<<topOriginOffset.y
            //			<< ","<<irect.width() << "," << irect.height() << std::endl;
            //std::cout << "RECT === " << rect.x()<<","<<rect.y()<<","
            //			<< rect.width() << "," << rect.height() << std::endl;
            //painter->drawImage(topOriginOffset.x, 
            //	               topOriginOffset.y, 
            //	               topTileCache->getCache());
            painter->drawImage(rect.x(),//topOriginOffset.x, 
            	               rect.y(),//topOriginOffset.y, 
            	               topTileCache->getCache(),
            	               rect.x()-topOriginOffset.x,
            	               rect.y()-topOriginOffset.y,
            	               rect.width(),
            	               rect.height());
         }
      }
   }
}
   
void ImageScrollView::drawForeground(QPainter* painter, const QRectF& inputRect)
{
   if(!m_trackPoint.hasNans()&&m_showTrackingCursorFlag&&m_trackingFlag)
   {
      ossimIpt roundedPoint(m_trackPoint);
      bool hasClipping = painter->hasClipping();
      painter->setClipping(false);
      painter->setPen(QColor(255, 255, 255));
      
      ossimIrect rect = viewportBoundsInSceneSpace();//(0,0,size().width()-1, size().height()-1);
      // ossimIpt ul = rect.ul();
      // ossimIpt lr = rect.lr();
      int left   = rect.ul().x;
      int right  = rect.lr().x;
      int top    = rect.ul().y;
      int bottom = rect.lr().y;
      if(rect.pointWithin(roundedPoint))
      {
         // draw horizontal
         //
         int x1 = left;
         int x2 = right;
         int y1  = roundedPoint.y;
         int y2  = y1;
         painter->drawLine(x1, y1, x2, y2);
         
         // draw vertical
         x1 = roundedPoint.x;
         x2 = x1;
         y1 = top;
         y2 = bottom;
         painter->drawLine(x1, y1, x2, y2);
      }
      painter->setClipping(hasClipping);
   }

   m_oldTrackPoint = m_trackPoint;

   emit paintYourGraphics(painter, inputRect);

   // Fix painter color.
}

void ImageScrollView::paintMultiLayer(QPainter& painter, const QRectF& /* rect */)
{
   if(m_multiLayerAlgorithm != ANIMATION_ALGORITHM)
   {
      ossimRefPtr<Layer> topLayer       = m_layers->layer((ossim_uint32)0);
      ossimRefPtr<Layer> bottomLayer    = m_layers->layer((ossim_uint32)1);
      if(topLayer.valid()&&bottomLayer.valid())
      {
         ossimRefPtr<StaticTileImageCache> topTileCache = topLayer->tileCache();
         ossimRefPtr<StaticTileImageCache> bottomTileCache = bottomLayer->tileCache();
         
         if(topTileCache.valid()&&bottomTileCache.valid())
         {
            ossimIrect rect = topTileCache->getRect();
            QRectF rectF(rect.ul().x, rect.ul().y, rect.width(), rect.height());   // = m_scrollToLocal.mapRect(QRectF(rect.ul().x, rect.ul().y, rect.width(), rect.height()));
            ossimIpt topOriginOffset = ossimDpt(rectF.x(), rectF.y());
            // for scrolling we need to offset from the tile location to the actual rect indicated by the viewport.
            // 
            ossim_uint32 w = rect.width();
            ossim_uint32 h = rect.height();
            switch(m_multiLayerAlgorithm)
            {
               case HORIZONTAL_SWIPE_ALGORITHM:
               {
                  ossim_float64 topLayerx     = topOriginOffset.x;
                  ossim_float64 bottomLayerx  = m_activePointEnd.x();
                  ossim_float64 topLayerWidth = bottomLayerx - topLayerx;
                  painter.drawImage(topLayerx, topOriginOffset.y, 
                                    topTileCache->getCache(),0,0,topLayerWidth,h);
                  painter.drawImage(topLayerx+topLayerWidth, topOriginOffset.y, bottomTileCache->getCache(), topLayerWidth, 0);
                  break;
               }
               case VERTICAL_SWIPE_ALGORITHM:
               {
                  ossim_int64 topLayery    = topOriginOffset.y;
                  ossim_int64 bottomLayery = m_activePointEnd.y();
                  ossim_int64 topLayerHeight = bottomLayery - topLayery;
                  painter.drawImage(topOriginOffset.x, topLayery, topTileCache->getCache(), 0, 0, w, topLayerHeight);
                  painter.drawImage(topOriginOffset.x, topLayery+topLayerHeight, bottomTileCache->getCache(), 0, topLayerHeight);
                  break;
               }
               case BOX_SWIPE_ALGORITHM:
               {
                  painter.drawImage(topOriginOffset.x, topOriginOffset.y, topTileCache->getCache());
                  ossim_float64 minx = ossim::min(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_float64 maxx = ossim::max(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_float64 miny = ossim::min(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_float64 maxy = ossim::max(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_float64 w = maxx-minx;
                  ossim_float64 h = maxy-miny;
                  ossim_float64 x = minx;
                  ossim_float64 y = miny;
                  //QPointF scrollPoint = m_localToScroll.map(QPointF(x,y));
                  ossimDrect cacheRect = bottomTileCache->getRect();
                  ossimDpt delta = ossimDpt(x,y) - cacheRect.ul();
                  
                  painter.drawImage(x, y, bottomTileCache->getCache(), delta.x, delta.y, w, h);
                  break;
               }
               case CIRCLE_SWIPE_ALGORITHM:
               {
                  // QImage& cacheImage = topTileCache->getCache();
                  // draw top and then overlay the bottom
                  ossim_float64 minx = ossim::min(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_float64 maxx = ossim::max(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_float64 miny = ossim::min(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_float64 maxy = ossim::max(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_float64 w = maxx-minx;
                  ossim_float64 h = maxy-miny;
                  ossim_float64 x = minx;
                  ossim_float64 y = miny;
                  
                  if(w < 1) w = 1;
                  if(h < 1) h = 1;
                  //QPointF scrollPoint = m_localToScroll.map(QPointF(x,y));
                  // ossimDpt cachePt = ossimDpt(scrollPoint.x(), scrollPoint.y()) - topTileCache->getRect().ul();
                  painter.save();
                  painter.drawImage(topOriginOffset.x, topOriginOffset.y, topTileCache->getCache());
                  painter.setBrush(QBrush(bottomTileCache->getCache()));
                  painter.setPen(Qt::NoPen);
                  
                  // this part is a little tricky but for the texturing to be placed in the ellipse properly
                  // I had to add a translation for the painter because the cache might extend past the current scroll region because it
                  // is on tile boundaries
                  //
                  // Because we shift for texturing with the QBrush we must undo the shift when drawing the ellipse so it lines up with
                  // the mouse draws.  The topOriginOffset holds the shift.
                  //
                  painter.translate(topOriginOffset.x, topOriginOffset.y);
                  painter.drawEllipse(x-topOriginOffset.x,y-topOriginOffset.y,w,h);
                  painter.restore();
                  break;
               }
               default:
               {
                  break;
               }
            }
         }
         // refreshDisplay();
      }
   }
   else
   {
   }
}

void ImageScrollView::mouseDoubleClickEvent ( QMouseEvent * e )
{
   QGraphicsView::mouseDoubleClickEvent(e);
   
   if(!m_inputBounds.hasNans())
   {
      QPointF p = mapToScene(e->pos());
      ossimDpt scenePoint(p.x(), p.y());		
      ossimDrect sceneViewRect = viewportBoundsInSceneSpace();
      
      emit mouseDoubleClick(e,  sceneViewRect, scenePoint);//viewportPoint, localPoint, viewPoint);
   }
   //QPointF p = mapToScene(e->pos());
   //if(m_trackingFlag)
   //{
   //   emit track(ossimDpt(p.x(), p.y()));
   // }
}
   
void ImageScrollView::emitTracking(const ossimDpt& pt)
{
   emit track(pt);
}
   
void ImageScrollView::mousePressEvent ( QMouseEvent * e )
{
   bool consumeEvent = false;
   m_manipulator->mousePressEvent(e, consumeEvent);
   m_mouseStartPoint = e->pos();
   QPointF p = mapToScene(e->pos());
   if(!consumeEvent)
   {
      QGraphicsView::mousePressEvent(e);
   }
   m_activePointStart = p;
   m_activePointEnd = p;
   
   m_lastClickedPoint.x = p.x();
   m_lastClickedPoint.y = p.y();

   emit mousePress(e);
   emit mousePress(e, m_lastClickedPoint);
   
   // Transform to true image coordinates
   ossim_uint32 idxLayer = 0;
   ossimImageSource* src = m_layers->layer(idxLayer)->chain();
   ossimGui::GatherImageViewProjTransVisitor visitor;
   src->accept(visitor);
   
   if (visitor.getTransformList().size() == 1)
   {
      ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
      if (ivtg.valid())
      {
         ossimDpt imagePoint;
         ossimDpt scenePoint(p.x(), p.y());
         ivtg->viewToImage(scenePoint, imagePoint);

         // Registration
         if (m_exploitationMode == DataManager::REGISTRATION_MODE && m_regOverlay->isActive())
         {
            // ROI selector for auto
            if (m_amDialogActive)
            {
               m_regOverlay->removeRoi(m_roiId);
               ossimDpt widHgt(1,1);
               m_regOverlay->addRoi(scenePoint, imagePoint, widHgt, m_roiId);
            }
            // Manual tie point
            else
            {
               m_regOverlay->addPoint(scenePoint, imagePoint);
            }
         }

         // Point drop
         else if (m_exploitationMode == DataManager::GEOPOSITIONING_MODE)
         {
            m_metricOverlay->addPoint(scenePoint, imagePoint);
         }
      }
   }
}

void ImageScrollView::mouseMoveEvent ( QMouseEvent * e )
{
   bool consumeEvent = false;
   m_manipulator->mouseMoveEvent(e, consumeEvent);
   if(!consumeEvent)
   {
      QGraphicsView::mouseMoveEvent(e);
   }
   
   if(e->buttons() & Qt::LeftButton)
   {
      m_activePointEnd = mapToScene(e->pos());
      if(m_layers->numberOfLayers() > 1)
      {
         refreshDisplay();
         //update();
      }

      if (m_amDialogActive)
      {
         QPointF p = mapToScene(e->pos());
         ossimDpt scenePoint(p.x(), p.y());

         // Transform to true image coordinates
         ossim_uint32 idxLayer = 0;
         ossimImageSource* src = m_layers->layer(idxLayer)->chain();
         ossimGui::GatherImageViewProjTransVisitor visitor;
         src->accept(visitor);
         if (visitor.getTransformList().size() == 1)
         {
            ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
            if (ivtg.valid())
            {
               ossimDpt imagePoint;
               ivtg->viewToImage(scenePoint, imagePoint);
               m_regOverlay->dragRoi(scenePoint, imagePoint, m_roiId);
            }
         }
      }
   }
   emit mouseMove(e);
}


void ImageScrollView::mouseReleaseEvent ( QMouseEvent * e )
{
   bool consumeEvent = false;
   m_manipulator->mouseReleaseEvent(e, consumeEvent);
   QPointF p = mapToScene(e->pos());
   if(!consumeEvent)
   {
      QGraphicsView::mouseReleaseEvent(e);
   }
   m_activePointEnd = p;

   // If auto measurement window active, emit ROI complete signal
   if (m_amDialogActive)
   {
      ossimGui::RoiSelection* roiSelection = m_regOverlay->getRoiSelection(m_roiId);
      if(roiSelection)
      {
        ossimIrect imgRect =roiSelection->getRectImg();
        ossimDpt imageStart = imgRect.ul();
        ossimDpt imageStop = imgRect.lr();
        emit mouseBox(this, imageStart, imageStop);
      }
   }

   emit mouseRelease(e);
}

void ossimGui::ImageScrollView::wheelEvent ( QWheelEvent * e )
{
   bool consumeEvent = false;
   m_manipulator->wheelEvent(e, consumeEvent);
   if(!consumeEvent)
   {
      QGraphicsView::wheelEvent(e);
   }
   
#if 0
   QPointF p = mapToScene(e->pos());     
   //if(m_trackingFlag)
   //{
   //   emit track(ossimDpt(p.x(), p.y()));
   //}
   
   if(!m_inputBounds.hasNans())
   {
      QRectF  sceneRect = mapToScene(viewport()->rect()).boundingRect();
      QPointF scenePoint = mapToScene(e->pos());	  
      emit wheel(e,  
                 ossimDrect(sceneRect.x(), sceneRect.y(),
                            sceneRect.x()+sceneRect.width()-1,
                            sceneRect.y()+sceneRect.height()-1),
                 ossimDpt(scenePoint.x(), scenePoint.y()));
   }
#endif
}
   
void ossimGui::ImageScrollView::enterEvent ( QEvent *  e)
{
   bool consumeEvent = false;
   if(m_manipulator.valid())
   {
      m_manipulator->enterEvent(e, consumeEvent);
   }
   if(!consumeEvent)
   {
      QGraphicsView::enterEvent(e);
   }
}

void ImageScrollView::leaveEvent ( QEvent *  e)
{
   bool consumeEvent = false;
   if(m_manipulator.valid())
   {
      m_manipulator->leaveEvent(e, consumeEvent);
   }
   if(!consumeEvent)
   {
      QGraphicsView::enterEvent(e);
   }
}

void ImageScrollView::keyPressEvent ( QKeyEvent * e )
{
   bool consumeEvent = false;
   if(m_manipulator.valid())
   {
      m_manipulator->keyPressEvent(e, consumeEvent);
   }
   if(!consumeEvent)
   {
      QGraphicsView::keyPressEvent(e);
   }
   
//??????????? test keys/functions ???????????????????????
   switch(e->key())
   {
      case Qt::Key_H:
      {
         break;
      }
   }
   
   if(e->key() == Qt::Key_A)
   {
      m_regOverlay->setVisible(true);
   }
   if(e->key() == Qt::Key_D)
   {
      m_regOverlay->setVisible(false);
   }
//??????????? test keys ???????????????????????
}

void ImageScrollView::keyReleaseEvent ( QKeyEvent * e )
{
   bool consumeEvent = false;
   if(m_manipulator.valid())
   {
      m_manipulator->keyReleaseEvent(e, consumeEvent);
   }
   if(!consumeEvent)
   {
      QGraphicsView::keyReleaseEvent(e);
   }
}


void ossimGui::ImageScrollView::setExploitationMode(int expMode)
{
   m_exploitationMode = static_cast<DataManager::ExploitationModeType> (expMode);
   
   // Toggle annotation visibility based on mode
   if (m_exploitationMode == DataManager::REGISTRATION_MODE)
   {
      m_regOverlay->setVisible(true);
      m_metricOverlay->setVisible(false);
   }
   else if (m_exploitationMode == DataManager::GEOPOSITIONING_MODE)
   {
      m_regOverlay->setVisible(false);
      m_metricOverlay->setVisible(true);
   }
   else if (m_exploitationMode == DataManager::MENSURATION_MODE)
   {
      m_regOverlay->setVisible(false);
      m_metricOverlay->setVisible(false);
      // TODO overlay use here?
   }
}


void ossimGui::ImageScrollView::setAutoMeasActive(const bool state)
{
   m_amDialogActive = state;

   if (!m_amDialogActive)
      m_regOverlay->removeRoi(m_roiId);
}


void ossimGui::ImageScrollView::setPositionGivenView(const ossimDpt& position)
{
   centerOn(position.x, position.y);;
}
   

// Currently called by ImageViewManipulator zoom functions
void ossimGui::ImageScrollView::zoomAnnotation()
{
   ossim_uint32 idxLayer = 0;
   ossimImageSource* src = m_layers->layer(idxLayer)->chain();
   ossimGui::GatherImageViewProjTransVisitor visitor;
   src->accept(visitor);
   if (visitor.getTransformList().size() == 1)
   {
      ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
      if (ivtg.valid())
      {
         m_regOverlay->setView(ivtg);
         m_metricOverlay->setView(ivtg);
      }
   }
}

// Called by ImageViewManipulator::setViewToChains()
void ossimGui::ImageScrollView::emitViewChanged()
{
   emit viewChanged();
}
   
}
