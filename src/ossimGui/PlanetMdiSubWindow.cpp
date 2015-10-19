#include <ossimGui/PlanetMdiSubWindow.h>
#include <iostream>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QtGui/QComboBox>
#include <QtGui/QMdiArea>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>
#include <QtGui/QApplication>
#include <ossimGui/ImageWidget.h>
#include <ossimGui/Event.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/base/ossimAdjustableParameterInterface.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimEcefPoint.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossimGui/BandSelectorEditor.h>
#include <ossimGui/BrightnessContrastEditor.h>
#include <ossimGui/HsiRemapperEditor.h>
#include <ossimGui/HistogramRemapperEditor.h>
#include <ossimGui/AdjustableParameterEditor.h>
#include <ossimGui/ExportImageDialog.h>
#include <ossimPlanet/ossimPlanetGrid.h>
#include <osgGA/KeySwitchMatrixManipulator>
#include <ossimPlanet/ossimPlanetLatLonHud.h>
#include <QtOpenGL/QGLFormat>
#include <list>

#include <ossim/projection/ossimLlxyProjection.h>

class OSSIMGUI_DLL ossimPlanetChainSetViewVisitor : public ossimTypeNameVisitor
{
public:
   ossimPlanetChainSetViewVisitor(ossimObject* view=0,
                                  int visitorType =(VISIT_INPUTS|VISIT_CHILDREN))
   :ossimTypeNameVisitor("ossimViewInterface", false,visitorType),
   m_obj(view)
   {
   }
   ossimPlanetChainSetViewVisitor(const ossimPlanetChainSetViewVisitor& src)
   :ossimTypeNameVisitor(src),
   m_obj(src.m_obj)
   {
   }
   virtual ossimRefPtr<ossimVisitor> dup()const{return new ossimPlanetChainSetViewVisitor(*this);}
   
   void setView()
   {
      int  refreshType = ossimRefreshEvent::REFRESH_NONE;
      ossim_uint32 nObjects =  m_collection.size();
      bool viewChanged = false;
      ossim_uint32 collectionIdx = 0;
      
      if(m_obj.valid())
      {
         for(collectionIdx = 0; collectionIdx < nObjects; ++collectionIdx)
         {
            ossimViewInterface* viewInterface = getObjectAs<ossimViewInterface>(collectionIdx);
            ossimPropertyInterface* propertyInterface = getObjectAs<ossimPropertyInterface>(collectionIdx);
            if(viewInterface)
            {
               ossimObject* input = dynamic_cast<ossimObject*>(viewInterface->getView());
               if(input)
               {
                  if(!input->isEqualTo(*(m_obj.get())))
                  {
                     refreshType |= ossimRefreshEvent::REFRESH_GEOMETRY;
                     viewInterface->setView(m_obj->dup());
                  }
               }
            }
            if(!m_resamplerType.empty()&&propertyInterface)
            {
               if(propertyInterface->getPropertyValueAsString("filter_type") != m_resamplerType)
               {
                  refreshType |= ossimRefreshEvent::REFRESH_PIXELS;
                  propertyInterface->setProperty("filter_type", m_resamplerType);
               }
            }
         }
      }
      if(refreshType!=ossimRefreshEvent::REFRESH_NONE)
      {
         ossimRefreshEvent* event = new ossimRefreshEvent();
         
         if(m_obj.valid())
         {
            event->setRefreshType(refreshType);
         }
         
         ossimEventVisitor eventVisitor(event);
         
         for(collectionIdx = 0; collectionIdx < nObjects; ++collectionIdx)
         {
            eventVisitor.reset();
            m_collection[collectionIdx]->accept(eventVisitor);
         }
      }
   }
   void setGeometry(ossimImageGeometry* obj){m_obj = obj;}
   void setResamplerType(const ossimString& value){m_resamplerType = value;}
protected:
   ossimRefPtr<ossimObject> m_obj;
   ossimString              m_resamplerType;
   
};


class ossimPlanetChainTextureLayer : public ossimPlanetTextureLayer
{
public:
   ossimPlanetChainTextureLayer(ossimImageChain* chain=0)
   {
      m_mapProj = new ossimLlxyProjection();
      m_mapProj->setMetersPerPixel(ossimDpt(20.0,20.0));
      m_geom = new ossimImageGeometry(0, m_mapProj.get());
      setChain(chain);
   }
   ossimPlanetChainTextureLayer(const ossimPlanetChainTextureLayer& src)
   {
      
   }
   virtual ossimPlanetTextureLayer* dup()const
   {
      return  new ossimPlanetChainTextureLayer(*this); 
   }
   virtual ossimPlanetTextureLayer* dupType()const
   {
      return new ossimPlanetChainTextureLayer();
   }
   virtual ossimString getClassName()const
   {
      return "ossimPlanetChainTextureLayer";
   }
   virtual ossimPlanetTextureLayerStateCode updateExtents()
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      clearState(ossimPlanetTextureLayerStateCode(ossimPlanetTextureLayer_NO_OVERVIEWS |
                                                  ossimPlanetTextureLayer_NO_GEOM |
                                                  ossimPlanetTextureLayer_NO_SOURCE_DATA));
      theDirtyExtentsFlag = false;
      ossimTypeNameVisitor visitor("ossimViewInterface");
      if(m_chain.valid())
      {
         m_chain->accept(visitor);
         ossimVisitor::ListRef& objList= visitor.getObjects();
         if(!objList.empty())
         {
            ossim_uint32 idx = 0;
            for(idx = 0; idx < objList.size();++idx)
            {
               ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*>(objList[idx].get());
               if(connectable)
               {
                  ossimImageSource* isrc = dynamic_cast<ossimImageSource*>(connectable->getInput());
                  if(isrc)
                  {
                     ossim_uint32 levels = isrc->getNumberOfDecimationLevels();

                     ossimRefPtr<ossimImageGeometry> geom = isrc->getImageGeometry();
                     if(geom.valid())
                     {
                        ossimDpt metersPerPixel = geom->getMetersPerPixel();
                        ossim_uint32 levels = isrc->getNumberOfDecimationLevels();
                        ossim_uint32 stopLevels = levels;
                        ossim_uint32 levelIdx = 0;
#if 1
                        for(levelIdx = 0; levelIdx < levels; ++levelIdx)
                        {
                           ossimIrect rect = isrc->getBoundingRect(levelIdx);
                           if((rect.width()>16)||(rect.height()>16))
                           {
                              ++stopLevels;
                           }
                           else
                           {
                              break;
                           }
                        }
#endif

                        ossimGpt ul, ur, lr, ll;
                        geom->getCornerGpts(ul, ur, lr, ll);
                        double minLat = ossim::min(ul.latd(), ossim::min(ur.latd(), ossim::min(lr.latd(), ll.latd())));
                        double minLon = ossim::min(ul.lond(), ossim::min(ur.lond(), ossim::min(lr.lond(), ll.lond())));
                        double maxLat = ossim::max(ul.latd(), ossim::max(ur.latd(), ossim::max(lr.latd(), ll.latd())));
                        double maxLon = ossim::max(ul.lond(), ossim::max(ur.lond(), ossim::max(lr.lond(), ll.lond())));
                        if(idx == 0)
                        {
                           theExtents = new ossimPlanetExtents();
                           theExtents->setMinMaxLatLon(minLat, minLon, maxLat, maxLon);
                           theExtents->setMinMaxScale(metersPerPixel.y,
                                                      metersPerPixel.y*std::pow(2.0, (double)(stopLevels)));// clamp the zoom out though
                        }
                        else
                        {
                           theExtents->combineMinMaxLatLon(minLat, minLon, maxLat, maxLon);
                           theExtents->combineScale(metersPerPixel.y,
                                                    metersPerPixel.y*std::pow(2.0, (double)(stopLevels)));
                        }
                     }
                  }
               }
            }
         }
      }

      return theStateCode;
   }
   virtual void updateStats()const
   {
   }
   virtual void resetStats()const
   {
   }
   ossimScalarType scalarType()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      if(m_chain.valid())
      {
         return m_chain->getOutputScalarType();
      }
      
      return OSSIM_SCALAR_UNKNOWN;
   }
   virtual bool hasTexture(ossim_uint32 width,
                           ossim_uint32 height,
                           const ossimPlanetTerrainTileId& tileId,
                           const ossimPlanetGrid& grid)
   {
      if(!getEnableFlag())
      {
         return false;
      }
      if(theDirtyExtentsFlag)
      {
         updateExtents();
      }
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      
      osg::Vec2d metersPerPixel;
      grid.getUnitsPerPixel(metersPerPixel, tileId, width, height, OSSIM_METERS);
      ossimDpt metersGsd(metersPerPixel[0], metersPerPixel[1]);
      if(theExtents.valid())
      {
         osg::ref_ptr<ossimPlanetExtents> extents = new ossimPlanetExtents;
         if(grid.convertToGeographicExtents(tileId, *extents, width, height))
         {
            if(theExtents->intersectsLatLon(*extents)&&
               theExtents->intersectsScale(*extents))
            {
               return true;
            }
         }
      }
      
      return false;
   }
   virtual osg::ref_ptr<ossimPlanetImage> getTexture(ossim_uint32 width,
                                                     ossim_uint32 height,
                                                     const ossimPlanetTerrainTileId& tileId,
                                                     const ossimPlanetGrid& grid,
                                                     ossim_int32 padding=0)
   {
      if(theDirtyExtentsFlag)
      {
         updateExtents();
      }
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      osg::ref_ptr<ossimPlanetImage> result;
      osg::ref_ptr<ossimPlanetExtents> tileExtents = new ossimPlanetExtents;
      if(m_chain.valid())
      {
         if(grid.convertToGeographicExtents(tileId, *tileExtents, width, height))
         {
            if(theExtents.valid())
            {
               if(!theExtents->intersectsLatLon(*tileExtents))
               {
                  return 0;
               }
            }
         }
      }
      else
      {
         return result;
      }
      osg::Vec2d unitsPerPixel;
      osg::Vec2d metersPerPixel;
      grid.getUnitsPerPixel(unitsPerPixel, tileId, width, height, OSSIM_DEGREES);
      grid.getUnitsPerPixel(metersPerPixel, tileId, width, height, OSSIM_METERS);
      ossimDpt gsd(unitsPerPixel[0], unitsPerPixel[1]);
      ossimDpt metersGsd(metersPerPixel[0], metersPerPixel[1]);
      osg::Vec2d deltaXY;
      grid.widthHeightInModelSpace(tileId, deltaXY);
      
      
      if(metersGsd.y < theExtents->getMaxScale())
      {
         if(!grid.isPolar(tileId))
         {
          //  ossimDpt ul;
          //  m_mapProj->worldToLineSample(ossimGpt(tileExtents->getMaxLat(), tileExtents->getMinLon()), ul);
          //  ossimDrect requestRect(ul.x, ul.y,
          //                         ul.x+width-1,
          //                         ul.y+height-1);
            ossimIrect requestRect(0,
                                   0,
                                   width-1,
                                   height-1);
            //         theProjection->setDecimalDegreesPerPixel(ossimDpt(deltaLon,
            //                                                           deltaLat));
            m_mapProj->setDecimalDegreesPerPixel(ossimDpt(gsd.x,gsd.y));
            m_mapProj->setUlTiePoints(ossimGpt(tileExtents->getMaxLat()-(gsd.y*.5), 
                                               tileExtents->getMinLon()+(gsd.x*.5)));
            m_mapProj->update();
            ossimPlanetChainSetViewVisitor visitor(m_geom.get());
            visitor.setResamplerType(getFilterTypeAsString());
            m_chain->accept(visitor);
            visitor.setView();
            ossimRefPtr<ossimImageData> data;
            data = m_chain->getTile(requestRect);
            
            if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus()!=OSSIM_EMPTY))
            {
               result = new ossimPlanetImage(tileId);
               convertToOsg(data.get(), result.get());
               result->flipVertical();
            }  
         }
         else
         {
            //   std::cout << "________________________" << std::endl;
            ossimPlanetGrid::ModelPoints modelPoints;
            grid.createModelPoints(tileId,
                                   width,
                                   height,
                                   modelPoints);
            
            //std::cout << "NEW WAY" << std::endl;
            std::vector<osg::Vec2d> minMaxPairs;
            grid.getInternationalDateLineCrossings(tileId, minMaxPairs);
            ossim_uint32 size = minMaxPairs.size();
            ossim_uint32 idx = 0;
            double maxLon=0.0, minLon=0.0, maxLat=tileExtents->getMaxLat(), minLat=tileExtents->getMinLat();
            for(idx = 0; idx < size; ++idx)
            {
               minLon = minMaxPairs[idx][0];
               maxLon = minMaxPairs[idx][1];
               
               ossim_uint32 dx = (ossim_uint32)(((maxLon-minLon)/gsd.x));//+.5);
               ossim_uint32 dy = (ossim_uint32)(((maxLat-minLat)/gsd.y));//+.5);
               ossimIrect requestRect(0,
                                      0,
                                      dx-1,
                                      dy-1);
               ossimRefPtr<ossimImageData> data;
               
               ossim_float32 tiles = (double)requestRect.width()/(double)width;
               ossim_uint32 nTiles = floor((double)requestRect.width()/(double)width);
               ossim_float32 residualTile = tiles - nTiles;
               ossim_uint32 tileX = 0;
               m_mapProj->setDecimalDegreesPerPixel(gsd);
               m_mapProj->update();
#if 0
               theProjection->setUlTiePoints(ossimGpt(maxLat-(gsd.y*.5), 
                                                      minLon+(gsd.x*.5)));
               ossimPlanetChainSetViewVisitor visitor(m_geom.get());
               visitor.setResamplerType(getFilterTypeAsString());
               m_chain->accept(visitor);
               visitor.setView();
               data = m_chain->getTile(requestRect);
               theChain->initialize();
               if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus()!=OSSIM_EMPTY))
               {
                  addBytesTransferredStat(data->getSizeInBytes());
                  if(!texture.valid())
                  {
                     texture = new ossimPlanetImage(tileId);
                  }
                  
                  convertToOsg(data.get(),
                               texture.get(),
                               osg::Vec2d(minLon,
                                          maxLat),
                               osg::Vec2d(maxLon,//deltaLon,
                                          maxLat),
                               osg::Vec2d(maxLon,//deltaLon,
                                          minLat),//deltaLat),
                               osg::Vec2d(minLon,
                                          minLat),//deltaLat),
                               modelPoints,
                               width,
                               height);
                  
               }
#else
               for(tileX = 0; tileX < nTiles; ++tileX)
               {
                  ossimIpt origin(tileX*width, 0);
                  ossimIrect tempRect(0, 0, width - 1, dy-1);
                  
                  double tempMinLon = minLon     + gsd.x*(origin.x);
                  double tempMaxLon = tempMinLon + gsd.x*width;
                  m_mapProj->setUlTiePoints(ossimGpt(maxLat-(gsd.y*.5), 
                                                     tempMinLon+(gsd.x*.5)));
                  ossimPlanetChainSetViewVisitor visitor(m_geom.get());
                  visitor.setResamplerType(getFilterTypeAsString());
                  m_chain->accept(visitor);
                  visitor.setView();
                  data = m_chain->getTile(tempRect);
                  if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus()!=OSSIM_EMPTY))
                  {
                     if(!result.valid())
                     {
                        result = new ossimPlanetImage(tileId);
                     }
                     
                     convertToOsg(data.get(),
                                  result.get(),
                                  osg::Vec2d(tempMinLon,
                                             maxLat),
                                  osg::Vec2d(tempMaxLon,//deltaLon,
                                             maxLat),
                                  osg::Vec2d(tempMaxLon,//deltaLon,
                                             minLat),//deltaLat),
                                  osg::Vec2d(tempMinLon,
                                             minLat),//deltaLat),
                                  modelPoints,
                                  width,
                                  height);
                     
                  }
                  data = 0;
               }
               if(residualTile > FLT_EPSILON)
               {
                  ossim_uint32 w = (residualTile*width);
                  ossimIpt origin(nTiles*width, 0);
                  ossimIrect tempRect(0, 0, w - 1, dy-1);
                  double tempMinLon = minLon + gsd.x*(origin.x);
                  double tempMaxLon = maxLon;
                  m_mapProj->setUlTiePoints(ossimGpt(maxLat,//-(deltaLat*.5), 
                                                     tempMinLon));//+(deltaLon*.5)));
                  ossimPlanetChainSetViewVisitor visitor(m_geom.get());
                  visitor.setResamplerType(getFilterTypeAsString());
                  m_chain->accept(visitor);
                  visitor.setView();
                  data = m_chain->getTile(tempRect);
                  if(data.valid()&&data->getBuf()&&(data->getDataObjectStatus()!=OSSIM_EMPTY))
                  {
                     if(!result.valid())
                     {
                        result = new ossimPlanetImage(tileId);
                        if(result->data())
                        {
                           memset(result->data(), '\0', result->getWidth()*result->getHeight()*4);
                        }
                     }
                     
                     convertToOsg(data.get(),
                                  result.get(),
                                  osg::Vec2d(tempMinLon,
                                             maxLat),
                                  osg::Vec2d(tempMaxLon,//deltaLon,
                                             maxLat),
                                  osg::Vec2d(tempMaxLon,//deltaLon,
                                             minLat),//deltaLat),
                                  osg::Vec2d(tempMinLon,
                                             minLat),//deltaLat),
                                  modelPoints,
                                  width,
                                  height);
                     
                  }
                  data = 0;
               }
#endif
            }
         }
      }
      
      return result; 
   }
   virtual osg::ref_ptr<ossimPlanetImage> getTexture(ossim_uint32 level,
                                                     ossim_uint64 row,
                                                     ossim_uint64 col,
                                                     const ossimPlanetGridUtility& utility)
   {
      osg::ref_ptr<ossimPlanetImage> result;
      return result; 
   }
   bool setChain(ossimImageChain* connectable)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      m_chain = connectable;
      if(m_chain.valid())
      {
         ossimPlanetChainSetViewVisitor visitor(m_geom.get());
         m_chain->accept(visitor);
         visitor.setView();
         
         dirtyExtents();
      }
      return true;
   }
   
   ossimImageChain* chain(){return m_chain.get();}
   const ossimImageChain* chain()const{return m_chain.get();}
   
   virtual double getApproximateHypotneusLength()const
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      ossimGpt p1(theExtents->getMinLat(), theExtents->getMinLon());
      ossimGpt p2(theExtents->getMaxLat(), theExtents->getMaxLon());
      ossimEcefPoint ep1(p1);
      ossimEcefPoint ep2(p2);
      
      return (ep2-ep1).length();
   }
   virtual void getCenterLatLonLength(double& centerLat,
                                      double& centerLon,
                                      double& length)const
   {
      centerLat = ossim::nan();
      centerLon = ossim::nan();
      length    = ossim::nan();
   }
   void setFilterType(const ossimString& filterType)
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_chainTextureLayerMutex);
      if(m_chain.valid())
      {
         ossimPlanetChainSetViewVisitor visitor;
         ossimPlanetTextureLayer::setFilterType(filterType);
         visitor.setResamplerType(filterType);
         if(m_chain.valid())
         {
            m_chain->accept(visitor);
            visitor.setView();
         }
      }
   }
   
protected:
   ossimRefPtr<ossimImageChain> m_chain;
   ossimRefPtr<ossimMapProjection> m_mapProj;
   ossimRefPtr<ossimImageGeometry> m_geom;
   
   mutable OpenThreads::Mutex m_chainTextureLayerMutex;
};


class ossimPlanetCopyToChainVisitor : public ossimVisitor
{
public:
   typedef std::vector<ossimConnectableObject* > ListType;
   ossimPlanetCopyToChainVisitor():ossimVisitor(ossimVisitor::VISIT_INPUTS)
   {
   }
   ossimPlanetCopyToChainVisitor(const ossimPlanetCopyToChainVisitor& src)
   :m_list(src.m_list)
   {
      
   }
   virtual ossimRefPtr<ossimVisitor> dup()const{return new ossimPlanetCopyToChainVisitor(*this);}
   void reset()
   {
      m_list.clear();
   }
   virtual void visit(ossimConnectableObject* obj)
   {
      if(!hasVisited(obj))
      {
         ossimVisitor::visit(obj);
         m_list.push_back(obj);
      }
   }
   ListType& list(){return m_list;}
   const ListType& list()const{return m_list;}
   
   ossimImageChain* copyToChain()const
   {
      ossimRefPtr<ossimImageChain> result;
      ListType l = m_list;
      if(!l.empty())
      {
         ossimString prefix;
         ossim_uint32 objIdx = 0;
         ossimKeywordlist kwl;
         kwl.add("type", "ossimImageChain");
   
         ossimRefPtr<ossimScalarRemapper> scalarRemapper = new ossimScalarRemapper();
         while(!l.empty())
         {
            prefix = "object" + ossimString::toString(objIdx) + ".";
            ossimRefPtr<ossimConnectableObject> obj = l.back();
            l.pop_back();
            obj->saveState(kwl, prefix);
            ++objIdx;
         }
         scalarRemapper->connectMyInputTo(m_list[0]);
         prefix = "object" + ossimString::toString(objIdx) + ".";
         scalarRemapper->saveState(kwl, prefix.c_str());
         result = new ossimImageChain();
        // std::cout << kwl << std::endl;
         if(!result->loadState(kwl))
         {
            result = 0;
         }
         else
         {
            result->initialize();
         }
      }
      
      return result.release();
   }
   
protected:
   mutable ListType m_list;
};

namespace ossimGui
{
   PlanetMdiSubWindow::PlanetMdiSubWindow( QWidget * parent, Qt::WindowFlags flags)
   :MdiSubWindowBase(parent, flags)
   {
      m_inputListener = new InputListener(this);
      m_connectableObject = new ConnectableDisplayObject(this);
      m_connectableObject->addListener(m_inputListener);
      setGeometry(0,0,512,512);
      setMinimumSize(QSize(64,64));
      
      QGLFormat format;
#if (defined(Q_WS_MAC) || defined(Q_WS_WIN))
      format.setSwapInterval(1);
#else
      if(format.swapInterval() >= 0)
      {
         format.setSwapInterval(1);
      }
#endif
      format.setOverlay(false);
      
#if 1 
      m_planetViewer = new GlViewer(format, this);
      //m_builder = new ossimPlanetViewMatrixBuilder();
      //m_builder->setLookAxis(ossimPlanetViewMatrixBuilder::LOOK_AXIS_NEGATIVE_Z);
      // m_builder->setLookAxis(ossimPlanetViewMatrixBuilder::LOOK_AXIS_NEGATIVE_Y);
      //m_imageScrollWidget->setConnectableObject(static_cast<ConnectableImageObject*>(m_connectableObject.get()));
      setWidget(m_planetViewer);
      ossimPlanetViewer* viewer = new ossimPlanetViewer;
      viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
      m_planetViewer->setViewer(viewer);
      m_planetViewer->setFocusPolicy(Qt::StrongFocus);
      m_planetViewer->setMouseTracking(true);
      ossimRefPtr<ossimPlanet> planet = new ossimPlanet;
      ossimRefPtr<ossimPlanetTerrain> terrain = new ossimPlanetTerrain();
      terrain->initElevation();
      osg::ref_ptr<ossimPlanetGrid> grid = new ossimPlanetAdjustableCubeGrid(ossimPlanetAdjustableCubeGrid::MEDIUM_CAP);  
      terrain->setGrid(grid.get());
      terrain->setElevationDensityType(ossimPlanetTerrain::MEDIUM_LOW_ELEVATION_DENSITY);
      m_textureLayers = new ossimPlanetTextureLayerGroup();
      m_textureLayers->setBackgroundColor(osg::Vec4f(1.0,1.0,1.0,1.0));
      m_textureLayers->setFillNullOrEmptyTileMaxLevel(0);
      m_textureLayers->setFillTranslucentPixelsWithBackground(true);
      terrain->setTextureLayer(0,m_textureLayers.get());
      ossimRefPtr<ossimPlanetMemoryImageCache> cache = new ossimPlanetMemoryImageCache;
      cache->setMinMaxCacheSizeInMegaBytes(16, 20);
      terrain->setElevationMemoryCache(cache.get());
      ossimRefPtr<ossimPlanetLatLonHud> hudLayer     = new ossimPlanetLatLonHud;
      
      planet->addChild(terrain.get());
      planet->addChild(hudLayer.get());
      
      m_manipulator = new ossimPlanetManipulator();
      m_planetViewer->viewer()->setCameraManipulator(m_manipulator.get());
      // osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
      //keyswitchManipulator->addMatrixManipulator( '1', "Standard", m_manipulator.get() );
      //m_planetViewer->viewer()->setCameraManipulator(keyswitchManipulator.get());
      m_planetViewer->viewer()->addEventHandler( new osgGA::StateSetManipulator(m_planetViewer->viewer()->getCamera()->getOrCreateStateSet()) );
      m_planetViewer->viewer()->addEventHandler(new osgViewer::StatsHandler);
      
      m_planetViewer->viewer()->setSceneData(planet.get());
      //m_builder->setGeoRefModel(m_planetViewer->viewer()->planet()->model().get());
#if 0
      m_planetViewer->setMouseTracking(true);
      //   tabWidget->setContentsMargins(0, 0, 0, 0);
      m_planetViewer->setFocusPolicy(Qt::StrongFocus);
      ossimRefPtr<ossimPlanet> planet = new ossimPlanet;
      //thePlanet->setupDefaults();
      
      ossimRefPtr<ossimPlanetLatLonHud> hudLayer     = new ossimPlanetLatLonHud;
      hudLayer->setAutoUpdateFlag(true);
      
      ossimRefPtr<ossimPlanetSousaLayer> sousaLayer   = new ossimPlanetSousaLayer;
      ossimRefPtr<ossimPlanetAnnotationLayer> annotationLayer = new ossimPlanetAnnotationLayer;
      
      
      
      ossimRefPtr<ossimPlanetTerrain> terrain = new ossimPlanetTerrain();
      terrain->initElevation();
      osg::ref_ptr<ossimPlanetGrid> grid = new ossimPlanetAdjustableCubeGrid(ossimPlanetAdjustableCubeGrid::MEDIUM_CAP);  
      terrain->setGrid(grid.get());
      terrain->setElevationDensityType(ossimPlanetTerrain::MEDIUM_LOW_ELEVATION_DENSITY);
      ossimRefPtr<ossimPlanetMemoryImageCache> cache = new ossimPlanetMemoryImageCache;
      cache->setMinMaxCacheSizeInMegaBytes(16, 20);
      terrain->setElevationMemoryCache(cache.get());
      planet->addChild(terrain.get());
      planet->addChild(annotationLayer.get());
      planet->addChild(sousaLayer.get());
      planet->addChild(hudLayer.get());
      
      ossimRefPtr<ossimPlanetManipulator> manipulator = new ossimPlanetManipulator();
      m_manipulator->viewMatrixBuilder()->setLookAxis(ossimPlanetViewMatrixBuilder::LOOK_AXIS_Z);
      
      //osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;
      // keyswitchManipulator->addMatrixManipulator( '1', "Standard", manipulator.get() );
      //   keyswitchManipulator->addMatrixManipulator( '2', "Trackball", new osgGA::TrackballManipulator() );
      //   keyswitchManipulator->addMatrixManipulator( '3', "Flight", new osgGA::FlightManipulator() );
      //   keyswitchManipulator->addMatrixManipulator( '4', "Drive", new osgGA::DriveManipulator() );
      //   keyswitchManipulator->addMatrixManipulator( '5', "Terrain", new osgGA::TerrainManipulator() );
      m_planetViewer->viewer()->setCameraManipulator(m_planetViewer.get());
      m_planetViewer->viewer()->addEventHandler( new osgGA::StateSetManipulator(m_planetViewer->viewer()->getCamera()->getOrCreateStateSet()) );
      m_planetViewer->viewer()->addEventHandler(new osgViewer::StatsHandler);
      m_planetViewer->viewer()->removeEphemeris();
      
      m_planetViewer->viewer()->setSceneData(planet.get());
      // m_planetViewer->viewer()->addCallback(theViewerCallback.get());
      
      m_planetViewer->viewer()->terrainLayer()->setNumberOfTextureLayers(1);
#endif
#endif
      //setAttribute(Qt::WA_DeleteOnClose);
      connect(this, SIGNAL(windowStateChanged ( Qt::WindowStates , Qt::WindowStates  )),this, SLOT(stateChanged(Qt::WindowStates , Qt::WindowStates)));
   }
   
   PlanetMdiSubWindow::~PlanetMdiSubWindow()
   {
      // std::cout << "PlanetMdiSubWindow::~PlanetMdiSubWindow()" << std::endl;
      m_planetViewer->viewer()->setSceneData(0);
      if(m_inputListener)
      {
         if(m_connectableObject.valid())
         {
            m_connectableObject->removeListener(m_inputListener);
         }
         delete m_inputListener;
         m_inputListener = 0;
      }
   }
   
   void PlanetMdiSubWindow::setConnectableObject(ConnectableObject* connectable)
   {
      if(m_connectableObject.valid())
      {
         m_connectableObject->removeListener(m_inputListener);
      }
      m_connectableObject = connectable;
   }
   
   void PlanetMdiSubWindow::stateChanged( Qt::WindowStates oldState, Qt::WindowStates newState)
   {
      if((oldState == Qt::WindowNoState) && (newState & Qt::WindowActive))
      {
      }
      else if(newState == Qt::WindowNoState)
      {
      }
   }
   
   void PlanetMdiSubWindow::syncView(View& viewInfo)
   {
     // std::cout << "ossimGui::PlanetMdiSubWindow::syncView(View& viewInfo)" << std::endl;
   }
   
   
   void PlanetMdiSubWindow::sync(View& view)
   { 
     // std::cout << "ossimGui::PlanetMdiSubWindow::sync" << std::endl;
      ossimGpt oldCenter;
      
      oldCenter.makeNan();
      
      ossimRefPtr<ossimImageGeometry> geom      = view.geometry();
      int syncType = view.syncType();
      
      if((syncType&View::SYNC_TYPE_POSITION)||
         (syncType&View::SYNC_TYPE_GEOM))
      {
         ossimGpt referencePosition = view.lookPositionAsGpt();
         if(!referencePosition.isLatNan()&&
            !referencePosition.isLonNan())
         {
            const ossimPlanetLookAt* lookAt = m_planetViewer->viewer()->currentLookAt();
            if(lookAt)
            {
               referencePosition.height( ossimElevManager::instance()->getHeightAboveEllipsoid(referencePosition));
               if(referencePosition.isHgtNan()) referencePosition.height(0.0);
               osg::ref_ptr<ossimPlanetLookAt> lookAtTarget = new ossimPlanetLookAt(referencePosition.latd(),
                                                                                    referencePosition.lond(),
                                                                                    referencePosition.height(),
                                                                                    lookAt->heading(),
                                                                                    lookAt->pitch(),
                                                                                    0.0,
                                                                                    lookAt->range(),
                                                                                    ossimPlanetAltitudeMode_ABSOLUTE);
               m_manipulator->navigator()->gotoLookAt(*lookAtTarget.get(), false);
               
               m_planetViewer->viewer()->requestRedraw();
            }
            
         }
      }
   }
   
   
   //void ossimGui::PlanetMdiSubWindow::ContainerListener::containerEvent(ossimContainerEvent& /* event */)
   //{
   //   QApplication::postEvent(m_window, new QEvent((QEvent::Type)ossimGui::WINDOW_REFRESH_ACTIONS_EVENT_ID));
   //}
   void PlanetMdiSubWindow::closeEvent ( QCloseEvent * event )
   {
      event->accept();
   }

   bool	PlanetMdiSubWindow::event(QEvent* evt)
   {
      switch(evt->type())
      {
         case ossimGui::WINDOW_REFRESH_ACTIONS_EVENT_ID:
         {
            if(windowState()&Qt::WindowActive)
            {
            }
            return true;
            break;
         }
         default:
         {
            break;
         }
      }
      return MdiSubWindowBase::event(evt);
   }
   
   
   
   void PlanetMdiSubWindow::InputListener::disconnectInputEvent(ossimConnectionEvent& event )
   {
      ossim_uint32 nObjects = event.getNumberOfOldObjects();
      ossim_uint32 idx = 0;
      for(idx = 0; idx < nObjects; ++idx)
      {
         ossimRefPtr<ossimConnectableObject> connectable = event.getOldObject(idx);
         PlanetMdiSubWindow::ChainToTextureType::iterator iter = m_window->m_chainToTextureMap.find(connectable.get());
         if(iter!= m_window->m_chainToTextureMap.end())
         {
            m_window->m_textureLayers->removeLayer(iter->second);
            m_window->m_chainToTextureMap.erase(iter);
         }
      }
   }
   
   void PlanetMdiSubWindow::InputListener::connectInputEvent(ossimConnectionEvent& event)
   {
      
      ossim_uint32 nObjects = event.getNumberOfNewObjects();
      ossim_uint32 idx = 0;
      for(idx = 0; idx < nObjects; ++idx)
      {
         ossimRefPtr<ossimConnectableObject> connectable = event.getNewObject(idx);
         if(connectable.valid())
         {
            ossim_int32 inputIdx = m_window->connectableObject()->findInputIndex(connectable.get());
            bool alreadyAdded =  m_window->m_chainToTextureMap.find(connectable.get())!=m_window->m_chainToTextureMap.end();
            if((inputIdx >= 0)&&(!alreadyAdded))
            {
               ossimPlanetCopyToChainVisitor copyVisitor;
               connectable->accept(copyVisitor);
               
               ossimRefPtr<ossimImageChain> chain = copyVisitor.copyToChain();
               if(chain.valid())
               {
                  ossimPlanetChainTextureLayer* textureLayer = new ossimPlanetChainTextureLayer(chain.get());
                  
                  m_window->m_textureLayers->addAfterIdx(inputIdx-1, textureLayer);
                  
                  m_window->m_chainToTextureMap.insert(std::make_pair(connectable.get(), textureLayer));
               }
            }
         }
      }
   }

}
