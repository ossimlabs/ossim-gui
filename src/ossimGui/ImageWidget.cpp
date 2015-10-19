#include <QtGui/QMdiArea>
#include <QtGui/QScrollBar>
#include <QtGui/QLayout>
#include <QtGui/QPainter>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsRectItem>
#include <QtCore/QTime>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimCacheTileSource.h>
#include <ossimGui/ImageWidget.h>
#include <ossimGui/Image.h>
#include <ossimGui/DisplayTimerJobQueue.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>
#include <ossimGui/RegistrationOverlay.h>

ossimGui::ImageWidgetJob::ImageWidgetJob()
{
   m_maxProcessingTime = 20;
}

void ossimGui::ImageWidgetJob::start()
{
   if(m_inputSource.valid())
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_imageWidgetJobMutex);
      QTime start = QTime::currentTime();
      ossimDrect cacheRect(m_tileCache->getRect());
      // ossimDpt ulCachePt = cacheRect.ul();
      ossimIrect rect;
      ossimDpt ul;      
      // Because the cache rect is a sub rect of the scroll we need the upper left which starts at offset 0,0 in scroll space
      m_cacheToView.map(0.0, 0.0, &ul.x, &ul.y);
      while(m_tileCache->nextInvalidTile(rect) && (!isCanceled()))
      {
         // shift to zero based rectangle and then set back for opying purposes.
         ossimRefPtr<ossimImageData> data =m_inputSource->getTile(rect+ul);
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

ossimGui::ImageScrollWidget::ImageScrollWidget(QWidget* parent)
:QScrollArea(parent),
m_listener(new ConnectionListener(this)),
m_jobQueue(new DisplayTimerJobQueue())
{
   m_trackPoint.makeNan();
   m_oldTrackPoint.makeNan();
   m_trackingFlag = true;
   m_mouseInsideFlag = false;
   
   m_layers = new Layers();
   //m_connector->addListener(m_listener);
   m_widget = new ImageWidget(this, viewport());
   //m_widget->setAttribute(Qt::WA_StaticContents);
   m_widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
   // setWidget(m_widget);
   setWidgetResizable(false);
   m_widget->show();
   m_trackingFlag = true;
   m_scrollOrigin = ossimDpt(0.0,0.0);
   m_tileSize = ossimIpt(64,64);
   //m_widget->setTileCache(new StaticTileImageCache(m_tileSize));
   m_timerId = -1;
   viewport()->setCursor(Qt::CrossCursor);
   m_imageWidgetJob = new ImageWidgetJob();//this);
   m_imageWidgetJob->setCallback(new Callback(this));
   
 //  m_multiLayerAlgorithm = HORIZONTAL_SWIPE_ALGORITHM;
 //  m_multiLayerAlgorithm = VERTICAL_SWIPE_ALGORITHM;
 //  m_multiLayerAlgorithm = BOX_SWIPE_ALGORITHM;
   m_multiLayerAlgorithm = CIRCLE_SWIPE_ALGORITHM;


   // Initialize drawing control
   m_drawPts = false;
   // m_regOverlay = new RegistrationOverlay();

   ///????????????
   // m_scene = new QGraphicsScene(0,0,500,400,this);
   // m_view = new QGraphicsView(this);
   // m_view->setScene(m_scene);
   // m_view->setStyleSheet("background: transparent");

   // QGraphicsRectItem* rect = m_scene->addRect(50,40,100,200);
   // rect->setFlags(QGraphicsItem::ItemIsMovable);
   // rect->setBrush(QBrush(Qt::blue));
   // rect->setOpacity(0.3);

   // QGraphicsItem* item = m_scene->addText("QGraphicsTextItem");
   // item->setFlags(QGraphicsItem::ItemIsMovable);

   // m_view->show();

}

ossimGui::ImageScrollWidget::~ImageScrollWidget()
{
   m_imageWidgetJob->cancel();
   
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
}

ossimGui::ConnectableImageObject* ossimGui::ImageScrollWidget::connectableObject()
{
   return m_connectableObject.get();
}

void ossimGui::ImageScrollWidget::setConnectableObject(ConnectableImageObject* c)
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


void ossimGui::ImageScrollWidget::inputConnected(ossim_int32 /* idx */)
{
   
   m_layers->adjustLayers(m_connectableObject.get());
   
  // m_scalarRemapperChain->connectMyInputTo(0, m_connectableObject->getInput());
   
   m_inputBounds = m_connectableObject->getBounds();
   
//   setCacheRect();
   if(!m_inputBounds.hasNans())
   {
      updateScrollBars();
   }
   
   updateTransforms();
   setCacheRect();
   
   // QPoint localPt(50,50);
   // QPoint viewPoint = m_localToView.map(localPt);
   
   if(m_jobQueue.valid())
   {
      if(!m_imageWidgetJob->isRunning()) m_imageWidgetJob->ready();
      m_jobQueue->add(m_imageWidgetJob.get());
   }
}

void ossimGui::ImageScrollWidget::inputDisconnected(ossim_int32 /* idx */)
{
   m_layers->adjustLayers(m_connectableObject.get());
   
   m_inputBounds = m_connectableObject->getBounds();
   
   
//   setCacheRect();
   if(!m_inputBounds.hasNans())
   {
      updateScrollBars();
   }
   
   updateTransforms();
   setCacheRect();
   
   if(m_jobQueue.valid())
   {
      if(!m_imageWidgetJob->isRunning()) m_imageWidgetJob->ready();
      m_jobQueue->add(m_imageWidgetJob.get());
   }
}

ossimDrect ossimGui::ImageScrollWidget::viewportBoundsInViewSpace()const
{
   QSize size = viewport()->size();
   QRectF out = m_localToView.mapRect(QRectF(0,0,size.width(), size.height()));
   return ossimDrect(out.x(), out.y(),out.x()+(out.width()-1), out.y() + (out.height()-1));
}

void ossimGui::ImageScrollWidget::setJobQueue(ossimJobQueue* jobQueue)
{
   m_jobQueue = jobQueue;
}

void ossimGui::ImageScrollWidget::refreshDisplay()
{
   m_layers->flushDisplayCaches();
   m_inputBounds = m_connectableObject->getBounds();
   
   
//   setCacheRect();
   if(!m_inputBounds.hasNans())
   {
      updateScrollBars();
   }
   
   updateTransforms();
   setCacheRect();
      
   if(m_jobQueue.valid())
   {
      if(!m_imageWidgetJob->isRunning()) m_imageWidgetJob->ready();

      m_jobQueue->add(m_imageWidgetJob.get());
   }
}

void ossimGui::ImageScrollWidget::updateTransforms()
{
   if(!m_inputBounds.hasNans())
   {
      m_viewToScroll = QTransform(1.0, 0.0, 0.0, 1.0, -m_inputBounds.ul().x, -m_inputBounds.ul().y);
      m_scrollToView = m_viewToScroll.inverted();
      m_scrollToLocal = QTransform(1.0,0.0,0.0,1.0, - m_scrollOrigin.x, -m_scrollOrigin.y);
      m_localToScroll = m_scrollToLocal.inverted();
      
      m_viewToLocal = m_scrollToLocal*m_viewToScroll;
      m_localToView = m_viewToLocal.inverted();
   }
   else 
   {
      m_viewToScroll = m_scrollToView = m_scrollToLocal = m_localToScroll = m_viewToLocal = m_localToView =QTransform();
   }

   //m_imageWidgetJob->setCacheToViewTransform(scrollToView());
   //m_imageWidgetJob->setViewToCacheTransform(viewToScroll());

}

void ossimGui::ImageScrollWidget::resizeEvent(QResizeEvent* event)
{
   QAbstractScrollArea::resizeEvent(event);
   m_widget->resize(size());
   updateScrollBars();

   m_scrollOrigin.x = horizontalScrollBar()->value();
   m_scrollOrigin.y = verticalScrollBar()->value();
   

   updateTransforms();
   
   setCacheRect();
   if(m_layers->findFirstDirtyLayer())
   {
      if(m_jobQueue.valid())
      {
         if(!m_imageWidgetJob->isRunning()) m_imageWidgetJob->ready();
         m_jobQueue->add(m_imageWidgetJob.get());
      }
   }

   // This would be useful only for HUD??
   //m_regOverlay->resize(event->size());
   // event->accept(); //???
}

void ossimGui::ImageScrollWidget::scrollContentsBy( int dx, int dy )
{
   QScrollArea::scrollContentsBy( dx,dy );
   m_scrollOrigin.x = horizontalScrollBar()->value();
   m_scrollOrigin.y = verticalScrollBar()->value();
   
   updateTransforms();

   setCacheRect();
   m_widget->update();
   if(m_layers->findFirstDirtyLayer())
   {
      if(m_jobQueue.valid())
      {
         if(!m_imageWidgetJob->isRunning()) m_imageWidgetJob->ready();
         m_jobQueue->add(m_imageWidgetJob.get());
      }
   }
}
void ossimGui::ImageScrollWidget::setCacheRect()
{
   /*
   ossimIpt ul(m_scrollOrigin);
   ossimIpt lr(m_scrollOrigin.x + size().width()-1,
               m_scrollOrigin.y + size().height()-1);
   ossimIrect rect(ul.x, ul.y, lr.x, lr.y);
   m_layers->setCacheRect(rect);
    */
   
   QRectF rect = m_localToScroll.mapRect(QRectF(0,0,size().width(), size().height()));
   m_layers->setCacheRect(ossimDrect(rect.x(), rect.y(),
                                     rect.x() + rect.width()-1,
                                     rect.y() + rect.height()-1));
}

void ossimGui::ImageScrollWidget::updateScrollBars()
{
   if(!m_inputBounds.hasNans())
   {
      if(m_inputBounds.width() > viewport()->size().width())
      {
         horizontalScrollBar()->setRange(0,m_inputBounds.width()-viewport()->size().width());
         horizontalScrollBar()->show();
      }
      else
      {
         horizontalScrollBar()->setRange(0,0);//,viewport()->size().width());
         horizontalScrollBar()->setVisible(false);
         horizontalScrollBar()->setValue(0);
      }
      if(m_inputBounds.height() > viewport()->size().height())
      {
         verticalScrollBar()->setRange(0,m_inputBounds.height()-viewport()->size().height());
         verticalScrollBar()->show();
      }
      else
      {
         verticalScrollBar()->setRange(0,0);//viewport()->size().height());
         verticalScrollBar()->setVisible(false);
         verticalScrollBar()->setValue(0);
      }
      m_scrollOrigin = ossimIpt(horizontalScrollBar()->value(), verticalScrollBar()->value());
   }
}

void ossimGui::ImageScrollWidget::setPositionGivenView(const ossimDpt& position)
{
   QSize size = viewport()->size();
   ossimDpt adjusted = (position - m_inputBounds.ul()) - ossimDpt(size.width()*.5, size.height()*.5);
   setPositionGivenLocal(adjusted);
}

void ossimGui::ImageScrollWidget::setPositionGivenLocal(const ossimDpt& position)
{
   QSize size = viewport()->size();
   ossimIpt scrollPosition(position);
   
   if(scrollPosition.x < 0) scrollPosition.x = 0;
   if(scrollPosition.y < 0) scrollPosition.y = 0;
   
   if(size.width() > m_inputBounds.width()) scrollPosition.x = 0;
   if(size.height() > m_inputBounds.height()) scrollPosition.y = 0;
   
   if(scrollPosition.x > m_inputBounds.width()) scrollPosition.x = m_inputBounds.width()-1;
   if(scrollPosition.y > m_inputBounds.height()) scrollPosition.y = m_inputBounds.height()-1;
   
   if(horizontalScrollBar()->value() != scrollPosition.x)
   {
      horizontalScrollBar()->setValue(scrollPosition.x);
   }
   if(verticalScrollBar()->value() != scrollPosition.y)
   {
      verticalScrollBar()->setValue(scrollPosition.y);
   }   
}

void	ossimGui::ImageScrollWidget::mouseDoubleClickEvent ( QMouseEvent * e )
{
   QScrollArea::mouseDoubleClickEvent(e);
   
   
   if(!m_inputBounds.hasNans())
   {
      ossimIpt origin = m_inputBounds.ul();
      ossimIpt localPoint(m_scrollOrigin.x +e->x(), m_scrollOrigin.y+e->y());
      ossimIpt viewPoint(localPoint.x+origin.x,
                         localPoint.y+origin.y);
      
      ossimDrect rect = viewportBoundsInViewSpace();
      
      emit mouseDoubleClick(e,  rect, viewPoint);//viewportPoint, localPoint, viewPoint);
   }
   
}

void	ossimGui::ImageScrollWidget::mouseMoveEvent ( QMouseEvent * e )
{
   QScrollArea::mouseMoveEvent(e);
   
   if(e->buttons() & Qt::LeftButton)
   {
      m_activePointEnd = e->pos();
      if(m_layers->numberOfLayers() > 1)
      {
         m_widget->update();
      }
   }
   if(!m_inputBounds.hasNans())
   {
      ossimIpt origin = m_inputBounds.ul();
      ossimIpt localPoint(m_scrollOrigin.x +e->x(), m_scrollOrigin.y+e->y());
      ossimIpt viewPoint(localPoint.x+origin.x,
                         localPoint.y+origin.y);
      
      ossimDrect rect = viewportBoundsInViewSpace();
      
      emit mouseMove(e,  rect, viewPoint);//viewportPoint, localPoint, viewPoint);
   }
}

void	ossimGui::ImageScrollWidget::mousePressEvent ( QMouseEvent * e )
{
   QScrollArea::mousePressEvent(e);
   
   m_activePointStart = e->pos();
   m_activePointEnd = e->pos();
   if(!m_inputBounds.hasNans())
   {
      ossimIpt origin = m_inputBounds.ul();
      ossimIpt localPoint(m_scrollOrigin.x +e->x(), m_scrollOrigin.y+e->y());
      ossimIpt viewPoint(localPoint.x+origin.x,
                         localPoint.y+origin.y);
      
      ossimDrect rect = viewportBoundsInViewSpace();
      
      // Save the measured point position
      //  (viewPoint = view<-scroll<-local)
      ossim_uint32 idxLayer = 0;
      ossimImageSource* src = m_layers->layer(idxLayer)->chain();
      ossimGui::GatherImageViewProjTransVisitor visitor;
      src->accept(visitor);
      if (visitor.getTransformList().size() == 1)
      {
         // Transform to true image coordinates and save
         ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
         if (ivtg.valid())
         {
            ivtg->viewToImage(viewPoint, m_measImgPoint);
         }
      }
      
      m_measPoint = viewPoint;
      m_drawPts = true;
      update();

      // m_regOverlay->setMeasPoint(m_measPoint);

// cout << "\n ImageScrollWidget::mousePressEvent ("
//    << viewPoint.x << ", "<< viewPoint.y << ") ("
//    << m_measImgPoint.x << ", "<< m_measImgPoint.y << ")"
//    << endl;

      emit mousePress(e,  rect, viewPoint);//viewportPoint, localPoint, viewPoint);
   }
}

void	ossimGui::ImageScrollWidget::mouseReleaseEvent ( QMouseEvent * e )
{
   QScrollArea::mouseReleaseEvent(e);
   
   m_activePointEnd = e->pos();
   
   if(!m_inputBounds.hasNans())
   {
      ossimIpt origin = m_inputBounds.ul();
      ossimIpt localPoint(m_scrollOrigin.x +e->x(), m_scrollOrigin.y+e->y());
      ossimIpt viewPoint(localPoint.x+origin.x,
                         localPoint.y+origin.y);
      
      ossimDrect rect = viewportBoundsInViewSpace();
      
      
      emit mouseRelease(e,  rect, viewPoint);//viewportPoint, localPoint, viewPoint);
  }
}

void	ossimGui::ImageScrollWidget::wheelEvent ( QWheelEvent * e )
{
   //QScrollArea::wheelEvent(e);
   if(!m_inputBounds.hasNans())
   {
      ossimIpt origin = m_inputBounds.ul();
      ossimIpt localPoint(m_scrollOrigin.x +e->x(), m_scrollOrigin.y+e->y());
      ossimIpt viewPoint(localPoint.x+origin.x,
                         localPoint.y+origin.y);
      
      ossimDrect rect = viewportBoundsInViewSpace();
      
      
      emit wheel(e,  rect, viewPoint);//viewportPoint, localPoint, viewPoint);
   }      
}
void	ossimGui::ImageScrollWidget::enterEvent ( QEvent *  )
{
   m_mouseInsideFlag = true;
   m_widget->update();
}

void	ossimGui::ImageScrollWidget::leaveEvent ( QEvent *  )
{
   m_mouseInsideFlag = false;
   m_widget->update();
}

ossimImageGeometry*  ossimGui::ImageScrollWidget::getGeometry()
{
   ossimImageSource* is = dynamic_cast<ossimImageSource*>(m_connectableObject->getInput());
   if(is)
   {
      return is->getImageGeometry().get();
   }
   return 0;
}

void ossimGui::ImageScrollWidget::setTrackPoint(const ossimDpt& position)
{
   if(position.hasNans())
   {
      m_trackPoint.makeNan();
   }
   else 
   {
      ossimDpt pt;
      m_viewToScroll.map(position.x, position.y, &pt.x, &pt.y);
      m_scrollToLocal.map(pt.x, pt.y, &m_trackPoint.x, &m_trackPoint.y);
      m_widget->update();
   }
}

ossimGui::ImageScrollWidget::Layer::Layer(ossimConnectableObject* obj)
{
   m_inputObject = obj;
   m_tileCache      = new StaticTileImageCache();
   m_scalarRemapperChain = new ossimImageChain();
   m_scalarRemapperChain->addFirst(new ossimScalarRemapper());
   m_scalarRemapperChain->addFirst(new ossimCacheTileSource());
   if(obj) m_scalarRemapperChain->connectMyInputTo(0, obj);
}

ossimGui::ImageScrollWidget::Layer::~Layer()
{
   clear();
}

ossimGui::ImageScrollWidget::Layers::Layers()
{
}

ossimGui::ImageScrollWidget::Layers::~Layers()
{
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      m_layers[idx]->clear();
      m_layers[idx] = 0;
   }
   m_layers.clear();
}

ossimGui::ImageScrollWidget::Layer* ossimGui::ImageScrollWidget::Layers::layer(ossim_uint32 idx)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return layerNoMutex(idx);
}

ossimGui::ImageScrollWidget::Layer* ossimGui::ImageScrollWidget::Layers::layer(ossimConnectableObject* input)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return layerNoMutex(input);
}

void  ossimGui::ImageScrollWidget::Layers::setCacheRect(const ossimDrect& rect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      m_layers[idx]->tileCache()->setRect(rect);
   }
}

ossimGui::ImageScrollWidget::Layer* ossimGui::ImageScrollWidget::Layers::layerNoMutex(ossim_uint32 idx)
{
   Layer* result = 0;
   if(idx < m_layers.size())
   {
      return m_layers[idx].get();
   }
   
   return result;
}


ossimGui::ImageScrollWidget::Layer* ossimGui::ImageScrollWidget::Layers::layerNoMutex(ossimConnectableObject* input)
{
   Layer* result = 0;
   LayerListType::iterator iter = std::find_if(m_layers.begin(), m_layers.end(), FindConnectable(input));
   if(iter != m_layers.end())
   {
      result = (*iter).get();
   }
   
   return result;
}

ossimGui::ImageScrollWidget::Layer* ossimGui::ImageScrollWidget::Layers::findFirstDirtyLayer()
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

bool ossimGui::ImageScrollWidget::Layers::isEmpty()const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return m_layers.empty();
}

void ossimGui::ImageScrollWidget::Layers::adjustLayers(ossimConnectableObject* connectable)
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

void ossimGui::ImageScrollWidget::Layers::flushDisplayCaches()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_layers.size(); ++idx)
   {
      if(m_layers[idx]->tileCache()) m_layers[idx]->tileCache()->flush();
   }
}


void ossimGui::ImageScrollWidget::paintWidget(QPainter& painter)
{
   if((m_layers->numberOfLayers() > 1)&&(m_multiLayerAlgorithm!=NO_ALGORITHM))
   {
      paintMultiLayer(painter);
   }
   else 
   {
      ossimRefPtr<Layer> topLayer    = m_layers->layer((ossim_uint32)0);
      if(topLayer.valid())
      {
         ossimRefPtr<StaticTileImageCache> topTileCache = topLayer->tileCache();
         if(topTileCache.valid())
         {
            ossimIrect rect          = topTileCache->getRect();
            QRectF rectF = m_scrollToLocal.mapRect(QRectF(rect.ul().x, rect.ul().y, rect.width(), rect.height()));
            
            ossimIpt topOriginOffset = ossimDpt(rectF.x(), rectF.y());
            painter.drawImage(topOriginOffset.x, topOriginOffset.y, topTileCache->getCache());
         }
      }
   }
   if(!m_trackPoint.hasNans()&&m_trackingFlag&&!m_mouseInsideFlag)
   {
      drawCursor(painter);
   }


   // Temporary marker control
   // if (m_drawPts == true)
   // {
   //    m_regOverlay->drawMeas(painter, m_viewToLocal);
   // }
   // m_regOverlay->drawProj(painter, m_viewToLocal);

}

void ossimGui::ImageScrollWidget::paintMultiLayer(QPainter& painter)
{
   if(m_multiLayerAlgorithm != ANIMATION_ALGORITHM)
   {
      ossimRefPtr<Layer> topLayer    = m_layers->layer((ossim_uint32)0);
      ossimRefPtr<Layer> bottomLayer    = m_layers->layer((ossim_uint32)1);
      if(topLayer.valid()&&bottomLayer.valid())
      {
         ossimRefPtr<StaticTileImageCache> topTileCache = topLayer->tileCache();
         ossimRefPtr<StaticTileImageCache> bottomTileCache = bottomLayer->tileCache();
         
         if(topTileCache.valid()&&bottomTileCache.valid())
         {
            ossimIrect rect = topTileCache->getRect();
            QRectF rectF    = m_scrollToLocal.mapRect(QRectF(rect.ul().x, rect.ul().y, rect.width(), rect.height()));
            ossimIpt topOriginOffset = ossimDpt(rectF.x(), rectF.y());
            // for scrolling we need to offset from the tile location to the actual rect indicated by the viewport.
            // 
            ossim_uint32 w = rect.width();
            ossim_uint32 h = rect.height();
            switch(m_multiLayerAlgorithm)
            {
               case HORIZONTAL_SWIPE_ALGORITHM:
               {
                  ossim_int64 topLayerx     = topOriginOffset.x;
                  ossim_int64 bottomLayerx  = m_activePointEnd.x();
                  ossim_int64 topLayerWidth = bottomLayerx - topLayerx;
                  painter.drawImage(topLayerx, topOriginOffset.y, topTileCache->getCache(), 0,0,topLayerWidth,h);
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
                  ossim_int64 minx = ossim::min(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_int64 maxx = ossim::max(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_int64 miny = ossim::min(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_int64 maxy = ossim::max(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_int64 w = maxx-minx;
                  ossim_int64 h = maxy-miny;
                  ossim_int64 x = minx;
                  ossim_int64 y = miny;
                  QPointF scrollPoint = m_localToScroll.map(QPointF(x,y));
                  ossimDrect cacheRect = bottomTileCache->getRect();
                  ossimDpt delta = ossimDpt(scrollPoint.x(), scrollPoint.y()) - cacheRect.ul();
                  
                  painter.drawImage(x, y, bottomTileCache->getCache(), delta.x, delta.y, w, h);
                  break;
               }
               case CIRCLE_SWIPE_ALGORITHM:
               {
                  // QImage& cahceImage = topTileCache->getCache();
                  // draw top and then overlay the bottom
                  ossim_int64 minx = ossim::min(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_int64 maxx = ossim::max(m_activePointStart.x(), m_activePointEnd.x());
                  ossim_int64 miny = ossim::min(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_int64 maxy = ossim::max(m_activePointStart.y(), m_activePointEnd.y());
                  ossim_int64 w = maxx-minx;
                  ossim_int64 h = maxy-miny;
                  ossim_int64 x = minx;
                  ossim_int64 y = miny;

                  // QPointF scrollPoint = m_localToScroll.map(QPointF(x,y));
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
      }
   }
   else
   {
   }
   
}

void ossimGui::ImageScrollWidget::drawCursor(QPainter& painter)
{
   if(!m_trackPoint.hasNans())
   {
      ossimIpt roundedPoint(m_trackPoint);
      bool hasClipping = painter.hasClipping();
      painter.setClipping(false);
      painter.setPen(QColor(255, 255, 255));
      
      ossimIrect rect(0,0,size().width()-1, size().height()-1);
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
         painter.drawLine(x1, y1, x2, y2);
         
         // draw vertical
         x1 = roundedPoint.x;
         x2 = x1;
         y1 = top;
         y2 = bottom;
         painter.drawLine(x1, y1, x2, y2);
      }
      painter.setClipping(hasClipping);
   }
   m_oldTrackPoint = m_trackPoint;
}


void ossimGui::ImageScrollWidget::updateAnnotation()
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
         ivtg->imageToView(m_measImgPoint, m_measPoint);
         ivtg->imageToView(m_markImgPoint, m_markPoint);

         // m_regOverlay->setMeasPoint(m_measPoint);
         // m_regOverlay->setProjPoint(m_markPoint);
      }
   }
   update();
}

void ossimGui::ImageScrollWidget::setMarkPointPosition(const ossimDpt& setViewPoint, const ossimDpt& setImagePoint)
{
   m_markPoint = setViewPoint;
   m_markImgPoint = setImagePoint;
   // m_regOverlay->setProjPoint(m_markPoint);
}

void ossimGui::ImageScrollWidget::storeCurrentPosition(const ossimString& /* id */)
{
   // m_regOverlay->addPoint(id, m_measImgPoint);
}

bool ossimGui::ImageScrollWidget::getPoint(const ossimString& /* id */, ossimDpt& imgPt)
{
   // bool foundPt = m_regOverlay->getPoint(id, imgPt);

   // return foundPt;
   return true;
}

ossimGui::ImageScrollWidget::ImageWidget::ImageWidget(ImageScrollWidget* scrollWidget, QWidget* parent)
:QFrame(parent), m_scrollWidget(scrollWidget)
{
   // m_scene = new QGraphicsScene(0,0,500,400,this);
   // m_view = new QGraphicsView(this);
   // m_view->setScene(m_scene);
   // m_view->setStyleSheet("background: transparent");

   // QGraphicsRectItem* rect = m_scene->addRect(50,40,100,200);
   // rect->setFlags(QGraphicsItem::ItemIsMovable);
   // rect->setBrush(QBrush(Qt::blue));
   // rect->setOpacity(0.3);

   // QGraphicsItem* item = m_scene->addText("QGraphicsTextItem");
   // item->setFlags(QGraphicsItem::ItemIsMovable);

   // m_view->show();
}


void ossimGui::ImageScrollWidget::ImageWidget::paintEvent(QPaintEvent* /* event */ )
{
   QPainter painter(this);
   m_scrollWidget->paintWidget(painter);

#if 0
   // for scrolling we need to offset from the tile location to the actual rect indicated by the viewport.
   // 
   ossimIpt originOffset =  m_tileCache->getRect().ul() - m_tileCache->getActualRect().ul();
   painter.drawImage(originOffset.x, originOffset.y, m_tileCache->getCache());
   
   
   //eraseCursor(painter);
   if(!m_trackPoint.hasNans()&&m_trackingFlag&&!m_mouseInsideFlag)
   {
      drawCursor(painter);
   }
#endif
}

#if 0

void ossimGui::ImageWidget::eraseCursor(QPainter& painter)
{
   if(!m_oldTrackPoint.hasNans())
   {
      ossimIpt originOffset =  m_tileCache->getActualRect().ul();
      ossimIpt roundedPoint(m_oldTrackPoint);
      QImage& cacheImage      = m_tileCache->getCache();
      ossimIrect rect(0,0,size().width()-1, size().height()-1);
      if(rect.pointWithin(roundedPoint))
      {
            // erase horizontal line            
         painter.drawImage(0,
                           roundedPoint.y,
                           cacheImage,
                           originOffset.x+rect.ul().x,
                           originOffset.y+roundedPoint.y,
                           (int)rect.width(),
                           (int)1);
         
         // erase vertical line
         
         painter.drawImage(roundedPoint.x,
                           0,
                           cacheImage,
                           originOffset.x + roundedPoint.x,
                           originOffset.y + rect.ul().y ,
                           (int)1,
                           (int)rect.height());
      }
      m_oldTrackPoint.makeNan();
   }
}



void ossimGui::ImageWidget::setTrackPoint(const ossimDpt& position)
{
   m_trackPoint = position;
}


//void ossimGui::ImageWidget::setTrackingFlag(bool flag)
//{
//   m_trackingFlag = flag;
//}

void ossimGui::ImageWidget::setMouseInsideFlag(bool flag)
{
   m_mouseInsideFlag = flag;
}

#endif
