#include <ossimGui/DataManager.h>
#include <ossimGui/ConnectableDisplayObject.h>
#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>
#include <ossimGui/IvtGeomTransform.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/RegistrationOverlay.h>
#include <ossimGui/MetricOverlay.h>
#include <ossimGui/ImageWidget.h>
#include <ossimGui/MultiImageDialog.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimDate.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageCombiner.h>
#include <ossim/projection/ossimProjection.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/base/ossimAdjustmentExecutive.h>
#include <ossim/base/ossimPointObservation.h>
#include <ossim/base/ossimObservationSet.h>

#ifdef OSSIM_PLANET_ENABLED
#include <ossimGui/PlanetMdiSubWindow.h>
#endif

#include <iostream>

ossimGui::DataManager::Node::Node(ossimRefPtr<ossimObject> source, const ossimString& /* name */, const ossimString& /* description */)
:m_object(source.get())
{
   setId();
}

ossimObject* ossimGui::DataManager::Node::getObject()
{
   return m_object.get();
}

const ossimObject* ossimGui::DataManager::Node::getObject()const
{
   return m_object.get();
}

void ossimGui::DataManager::Node::setObject(ossimObject* obj)
{
   m_object = obj;
   setId();
}

bool ossimGui::DataManager::Node::supportsInputs()const
{
   bool result = false;
   const ossimConnectableObject* connectable = getObjectAsConnectable();
   if(connectable)
   {
      result = ((!connectable->getInputListIsFixedFlag())||
                (connectable->getInputListIsFixedFlag()&&connectable->getNumberOfInputs()>0));
   }
   
   return result;
}

ossimConnectableObject*  ossimGui::DataManager::Node::getObjectAsConnectable()
{
   return dynamic_cast<ossimConnectableObject*>(m_object.get());
}

const ossimConnectableObject* ossimGui::DataManager::Node::getObjectAsConnectable()const
{
   return dynamic_cast<const ossimConnectableObject*>(m_object.get());
}

bool ossimGui::DataManager::Node::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
{
   kwl.add(prefix,
           "name",
           m_name.toAscii().data(),
           true);
   kwl.add(prefix,
           "description",
           m_description.toAscii().data(),
           true);
   if(m_object.valid())
   {
      m_object->saveState(kwl, prefix);
   }
   
   return true;
}

bool ossimGui::DataManager::Node::loadState(const ossimKeywordlist& kwl, const ossimString& prefix)
{
   m_name        = kwl.find(prefix, "name");
   m_description = kwl.find(prefix, "description");
   
   m_object = ossimObjectFactoryRegistry::instance()->createObject(kwl, prefix.c_str());
   setId();
   return m_object.valid();
}

void ossimGui::DataManager::Node::setId()
{
   ossimConnectableObject* connectable = getObjectAsConnectable();
   if(connectable)
   {
      m_id = connectable->getId();
   }
}

ossimGui::DataManager::DataManager()
{
   m_mdiArea = 0;
   m_defaultReprojectionChainTemplate = "type:ossimImageChain\n"
   "object0.type:ossimBandSelector\n"
   "object5.type:ossimHistogramRemapper\n"
   "object10.type:ossimCacheTileSource\n"
   "object20.type:ossimImageRenderer\n"
   "object20.max_levels_to_compute:0\n"
   "object20.image_view_trans.type:ossimImageViewProjectionTransform\n"
   "object30.type:ossimCacheTileSource\n"
   "object40.type:ossimBrightnessContrastSource\n"
   "object50.type:ossimHsiRemapper\n";
   
   m_defaultAffineChainTemplate = "type:ossimImageChain\n"
   "object0.type:ossimBandSelector\n"
   "object5.type:ossimHistogramRemapper\n"
   "object10.type:ossimCacheTileSource\n"
   "object20.type:ossimImageRenderer\n"
   "object20.max_levels_to_compute:0\n"
   "object20.image_view_trans.type:ossimImageViewAffineTransform\n"
   "object30.type:ossimCacheTileSource\n"
   "object40.type:ossimBrightnessContrastSource\n"
   "object50.type:ossimHsiRemapper\n";
}

bool ossimGui::DataManager::remove(ossimRefPtr<Node> obj, bool notifyFlag)
{
   ossimRefPtr<Callback> callback;
   bool result = false;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
      result = removeIndexMapping(obj.get());
      if(result)
      {
         NodeListType::iterator sourceIter = std::find(m_sourceList.begin(),
                                                       m_sourceList.end(),
                                                       obj.get());
         if(sourceIter != m_sourceList.end())
         {
            m_sourceList.erase(sourceIter);
         }
         NodeListType::iterator chainIter = std::find(m_chainList.begin(),
                                                      m_chainList.end(),
                                                      obj.get());
         if(chainIter != m_chainList.end())
         {
            m_chainList.erase(chainIter);
         }
         
         NodeListType::iterator displayIter = std::find(m_displayList.begin(),
                                                      m_displayList.end(),
                                                      obj.get());
         if(displayIter != m_displayList.end())
         {
            m_displayList.erase(displayIter);
         }
         callback = m_callback;
      }
   }      
   if(result)
   {
      //std::cout << "ossimGui::DataManager::remove(:REMOVING " << obj->getObject()->getClassName() << std::endl;
      if(obj->getObjectAsConnectable()) obj->getObjectAsConnectable()->disconnect();
      if(callback.valid()&&callback->enabled()&&notifyFlag)
      {
         callback->nodeRemoved(obj.get());
         obj = 0;
      }
   }
   
   
   return result;
}

bool ossimGui::DataManager::remove(NodeListType& nodes, bool notifyFlag)
{
   ossim_uint32 idx = 0;
   NodeListType nodesRemoved;
   bool result = true;
   for(idx = 0; idx < nodes.size();++idx)
   {
      if(remove(nodes[idx], false))
      {
         nodesRemoved.push_back(nodes[idx]);
      }
      else 
      {
         result = false;
      }

   }
   if(notifyFlag)
   {
      ossimRefPtr<Callback> callback;
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
         if(m_callback.valid()&&m_callback->enabled())
         {
            callback = m_callback;
         }
      }
      if(callback.valid())
      {
         callback->nodesRemoved(nodesRemoved);
         nodesRemoved.clear();
      }
   }
   
   return result;
}


bool ossimGui::DataManager::nodeExists(ossimObject* obj)const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return (m_nodeMap.find(obj)!=m_nodeMap.end());
}

ossimRefPtr<ossimGui::DataManager::Node>  ossimGui::DataManager::findNode(ossimObject* obj)
{
   // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   m_mutex.lock();
   ossimRefPtr<ossimGui::DataManager::Node> result = 0;
   
   NodeMapType::iterator iter = m_nodeMap.find(obj);
   
   if(iter != m_nodeMap.end())
   {
      result = iter->second;
   }
   m_mutex.unlock();
   return result;
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::addSource(ossimRefPtr<ossimObject> obj, bool notifyFlag)
{
   ossimRefPtr<Node> result;
   ossimRefPtr<Callback> callback;
   if(obj.valid()&&!nodeExists(obj.get()))
   {
      QString defaultName = "";
      QString defaultDescription = "";
      result = new Node(obj.get());
      // ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*> (obj.get());
      ossimImageHandler* handler = dynamic_cast<ossimImageHandler*> (obj.get());
      ossimImageChain* chain = dynamic_cast<ossimImageChain*> (obj.get());
      if(handler)
      {
         defaultName = " Entry " + ossimString::toString(handler->getCurrentEntry())+ ": " + ossimString(handler->getFilename());
      }
      else
      {
         defaultName = obj->getClassName().c_str();
      }
      
      result->setName(defaultName);
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
         if(handler)
         {
            m_sourceList.push_back(result.get());
         }
         else if(chain)
         {
            m_chainList.push_back(result.get());
         }
         else if(dynamic_cast<ConnectableDisplayObject*> (obj.get()))
         {
            m_displayList.push_back(result.get());
         }
         else
         {
            m_sourceList.push_back(result.get());
         }
         addIndexMapping(result.get());
         if(m_callback.valid()&&m_callback->enabled()&&notifyFlag)
         {
            callback = m_callback;
         }
      }
   }
   if(result.valid()&&callback.valid())
   {
      callback->nodeAdded(result.get());
   }      
   return result.get();
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createDefaultImageChain(ossimRefPtr<Node> input, bool notifyFlag)
{
   ossimRefPtr<Node> result;
   ossimConnectableObject* connectableInput = input->getObjectAsConnectable();
   ossimRefPtr<Callback> callback;

   if(connectableInput)
   {
      ossimImageSource* source = dynamic_cast<ossimImageSource*> (input->getObjectAsConnectable());
      if(source)
      {
         ossimRefPtr<ossimImageGeometry> geom = source->getImageGeometry();
         if(geom.valid()&&geom->getProjection())
         {
            result = createChainFromTemplate(m_defaultReprojectionChainTemplate, input.get(), false);
            result->setName("Reprojection Chain:" + input->name());
         }
         else
         {
            result = createChainFromTemplate(m_defaultAffineChainTemplate, input.get(), false);
            result->setName("Affine Chain:" + input->name());
         }
         {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
            callback = m_callback;
         }
     }
      
   }
   if(callback.valid()&&callback->enabled()&&notifyFlag)
   {
      callback->nodeAdded(result.get());
   }
   return result.get();
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createChainFromTemplate(const ossimString& templatChain, 
                                                                                        ossimRefPtr<Node> input,
                                                                                        bool notifyFlag)
{
   ossimKeywordlist kwl;
   
   if(kwl.parseString(templatChain))
   {
      return createChainFromTemplate(kwl, input.get(), notifyFlag);
   }
   
   return 0;
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createChainFromTemplate(const ossimKeywordlist& templatChain, 
                                                                                        ossimRefPtr<Node> input,
                                                                                        bool notifyFlag)
{
   ossimRefPtr<Node> result;
   ossimConnectableObject* connectableInput = input->getObjectAsConnectable();
   ossimRefPtr<Callback> callback;
   
   ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(templatChain);
   if(obj.valid())
   {
      ossimRefPtr<ossimConnectableObject> connectable = dynamic_cast<ossimConnectableObject*> (obj.get());
      connectable->connectMyInputTo(connectableInput);
      result = new Node(connectable.get());
      
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
         m_chainList.push_back(result.get());
         addIndexMapping(result.get());
         
         result->setName(input->name());
         callback = m_callback;
      }
   }
   if(callback.valid()&&callback->enabled()&&notifyFlag)
   {
      callback->nodeAdded(result.get());
   }
   
   return result.get();
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createDefaultCombinerChain(const ossimString& combinerType, NodeListType& nodeList, bool notifyFlag)
{
   ossimRefPtr<Node> result;
   ossimRefPtr<ossimImageSource> obj = ossimImageSourceFactoryRegistry::instance()->createImageSource(combinerType);
   ossimRefPtr<ossimImageCombiner> combinerObj = dynamic_cast<ossimImageCombiner*>(obj.get());
   ossimRefPtr<Callback> callback;
   if(combinerObj.valid())
   {
         // we will need to make the combiner chain a template as well and remove from here
      ossimString combinerChain = 
      "type:ossimImageChain\n"
      "object0.type:ossimBandSelector\n"
      "object5.type:ossimHistogramRemapper\n"
      "object40.type:ossimBrightnessContrastSource\n"
      "object50.type:ossimHsiRemapper\n";
      
      ossimKeywordlist kwl;
      if(kwl.parseString(combinerChain))
      {
         ossimRefPtr<ossimImageSource> chainObj = ossimImageSourceFactoryRegistry::instance()->createImageSource(kwl);
         if(chainObj.valid())
         {
            ossimRefPtr<ossimImageChain> chain = dynamic_cast<ossimImageChain*> (chainObj.get());
            if(chain.valid())
            {
               chain->addLast(combinerObj.get());
               result = new Node(chain.get());
               m_chainList.push_back(result.get());
               result->setName(combinerType.c_str());
               addIndexMapping(result.get());
               if(!nodeList.empty())
               {
                  NodeListType::iterator iter = nodeList.begin();
                  while(iter!=nodeList.end())
                  {
                     if((*iter)->getObject())
                     {
                        ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
                        if(connectable)
                        {
                           chain->connectMyInputTo(connectable);
                        }
                     }
                     ++iter;
                  }
               }
               if(m_callback.valid()&&m_callback->enabled()&&notifyFlag)
               {
                  OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
                  callback = m_callback.get();
               }
            }
         }
      }
   }
   if(result.valid())
   {
      if(callback.valid())
      {
         callback->nodeAdded(result.get());
      }
   }
   return result;
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createDefault2dImageDisplay(ossimRefPtr<Node> input, bool notifyFlag)
{
   ImageMdiSubWindow* display = new ImageMdiSubWindow();
   ossimRefPtr<ossimGui::DataManager::Node> result = addSource(display->connectableObject(), notifyFlag);
   if(m_mdiArea)
   {
      m_mdiArea->addSubWindow(display);
   }

   // Connect input
   if(display->connectableObject()&&input.valid()&&input->getObjectAsConnectable())
   {
      display->connectableObject()->connectMyInputTo(0, input->getObjectAsConnectable());
   }

   // Set title
   if(result.valid())
   {
      ossimConnectableObject* connectable = result->getObjectAs<ossimConnectableObject>();
      if(connectable)
      {
         ossimTypeNameVisitor visitor("ossimImageHandler");
         connectable->accept(visitor);
         if(!visitor.getObjects().empty())
         {
            ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
            ossimString source = input->getFilename();
            QString windowTitle = QFontMetrics(QFont()).elidedText
               (source.data(), Qt::ElideLeft, display->width()-100);
            display->setWindowTitle(windowTitle);
          }
      }
      else
      {
         display->setWindowTitle(result->name());
      }
   }

   // Set the current mode for the new display
   display->scrollWidget()->setExploitationMode(m_exploitationMode);
   
   return result.get();
}

ossimRefPtr<ossimGui::DataManager::Node> ossimGui::DataManager::createDefault3dPlanetaryDisplay(bool notifyFlag)
{
#ifdef OSSIM_PLANET_ENABLED
   PlanetMdiSubWindow* display = new PlanetMdiSubWindow();
   if(m_mdiArea)
   {
      m_mdiArea->addSubWindow(display);
   }
   ossimRefPtr<ossimGui::DataManager::Node> result = addSource(display->connectableObject(), notifyFlag);
   if(result.valid())
   {
      result->setName("Planetary Viewer");
      display->setWindowTitle("Planetary Viewer");
   }
   display->show();
   return result.get();
#else
   return 0;
#endif
}

void ossimGui::DataManager::accept(ossimVisitor& visitor)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   NodeListType::iterator iter = m_sourceList.begin();
   while(iter != m_sourceList.end())
   {
      (*iter)->accept(visitor);
      ++iter;
   }
   
   iter = m_chainList.begin();
   while(iter != m_chainList.end())
   {
      (*iter)->accept(visitor);
      
      ++iter;
   }
   iter = m_displayList.begin();
   while(iter != m_displayList.end())
   {
      (*iter)->accept(visitor);
      
      ++iter;
   }
}

void ossimGui::DataManager::print()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   ossim_uint32 idx = 0;
   for(idx =0; idx < m_sourceList.size();++idx)
   {
      std::cout << "Name: " << m_sourceList[idx]->name().toAscii().data() << std::endl;
   }
   
   for(idx=0;idx < m_chainList.size();++idx)
   {
      std::cout << "Name: " << m_chainList[idx]->name().toAscii().data() << std::endl;
   }
   for(idx=0;idx < m_displayList.size();++idx)
   {
      std::cout << "Name: " << m_displayList[idx]->name().toAscii().data() << std::endl;
   }
   
}

void ossimGui::DataManager::clear(bool notifyFlag)
{
   ossimRefPtr<Callback> callback;
   NodeListType removedNodes;
   {
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
      
         removedNodes.insert(removedNodes.end(), m_sourceList.begin(), m_sourceList.end());
         removedNodes.insert(removedNodes.end(), m_chainList.begin(), m_chainList.end());
         removedNodes.insert(removedNodes.end(), m_displayList.begin(), m_displayList.end());
         m_sourceList.clear();
         m_chainList.clear();
         m_displayList.clear();
      }
      ossim_uint32 idx = 0;
      for(idx = 0; idx < removedNodes.size(); ++idx)
      {
         remove(removedNodes[idx],false);
      }
      if(m_callback.valid()&&m_callback->enabled()&&notifyFlag)
      {
         callback = m_callback;
      }
   }
   if(callback.valid())
   {
      callback->nodesRemoved(removedNodes);
   }
   ossim_uint32 idx = 0;
   // disconnect outside the scope lock for there might be a call to the remove from outside this interface
   for(idx = 0; idx < removedNodes.size(); ++idx)
   {
      ossimConnectableObject* connectable = removedNodes[idx]->getObjectAsConnectable();
      if(connectable) connectable->disconnect();
   }
}

bool ossimGui::DataManager::saveState(ossimKeywordlist& kwl, const ossimString& prefix)const
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   
   kwl.add(prefix,
           "type",
           "DataManager",
           true);
   ossimString nodePrefix = prefix + "objectList.object";
   ossim_uint32 idx = 0;
   ossim_uint32 nodeIdx = 0;
   for(idx =0; idx < m_sourceList.size();++idx)
   {
      ossimString nodeNumberPrefix = nodePrefix + ossimString::toString(nodeIdx) + ".";
      m_sourceList[idx]->saveState(kwl, nodeNumberPrefix);
      ++nodeIdx;
   }
   
   for(idx=0;idx < m_chainList.size();++idx)
   {
      ossimString nodeNumberPrefix = nodePrefix + ossimString::toString(nodeIdx) + ".";
      m_chainList[idx]->saveState(kwl, nodeNumberPrefix);
      
      ++nodeIdx;
   }
   for(idx=0;idx < m_displayList.size();++idx)
   {
      ossimString nodeNumberPrefix = nodePrefix + ossimString::toString(nodeIdx) + ".";
      m_displayList[idx]->saveState(kwl, nodeNumberPrefix);
      
      ++nodeIdx;
   }
   return true;
}

bool ossimGui::DataManager::loadState(const ossimKeywordlist& kwl, const ossimString& prefix)
{
   bool result = true;
   NodeListType nodes;
   
   ossimRefPtr<Callback> callback;
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
      if(m_callback.valid()&&m_callback->enabled())
      {
         callback = m_callback;
      }
      ossimString copyPrefix = prefix;
      ossimString type = kwl.find(copyPrefix, "type");
      ossimString regExpression =  ossimString("^(") + copyPrefix + "objectList.object[0-9]+.)";
      std::vector<ossimString> nodeKeys;
      kwl.getSubstringKeyList( nodeKeys, regExpression );
      ossim_uint32 nNodes = nodeKeys.size();
      ossim_uint32 nodeKeyOffset = (copyPrefix.size()+ossimString("objectList.object").size());
      std::vector<ossim_uint32> nodeKeyIndices(nNodes);
      ossim_uint32 idx = 0;
      for(idx = 0; idx < nodeKeyIndices.size();++idx)
      {
         ossimString numberStr(nodeKeys[idx].begin() + nodeKeyOffset,
                               nodeKeys[idx].end());
         nodeKeyIndices[idx] = numberStr.toInt();
      }
      std::sort(nodeKeyIndices.begin(), nodeKeyIndices.end());
      
      
      // first allocate the nodes before we can connect them
      //
      for(idx=0;((idx < nodeKeyIndices.size())&&result);++idx)
      {
         ossimString newPrefix = copyPrefix;
         newPrefix += ossimString("objectList.object");
         newPrefix += ossimString::toString(nodeKeyIndices[idx]);
         newPrefix += ossimString(".");
         
         ossimRefPtr<Node> node = new Node();
         if(node->loadState(kwl, newPrefix))
         {
            nodes.push_back(node.get());
            if(node->supportsInputs())
            {
               if(dynamic_cast<ConnectableDisplayObject*> (node->getObject()))
               {
                  m_displayList.push_back(node.get());
               }
               else 
               {
                  m_chainList.push_back(node.get());
               }

            }
            else
            {
               m_sourceList.push_back(node.get());
            }
            addIndexMapping(node.get());
         }
         else 
         {
            result = false;
         }
         
      } 
      if(result)
      {
         for(idx = 0; idx < nodeKeyIndices.size();++idx)
         {
            ossimRefPtr<ossimConnectableObject> connectable = nodes[idx]->getObjectAsConnectable();
            if(connectable.valid())
            {
               if(nodes[idx]->supportsInputs())
               {
                  ossimString newPrefix = copyPrefix;
                  newPrefix += ossimString("objectList.object");
                  newPrefix += ossimString::toString(nodeKeyIndices[idx]);
                  newPrefix += ossimString(".");
                  std::vector<ossimId> ids;
                  findInputConnectionIds(ids, kwl, newPrefix); 
                  if(ids.size())
                  {
                     ossim_uint32 idsIndex = 0;
                     for(idsIndex = 0; idsIndex < ids.size(); ++idsIndex)
                     {
                        ossimRefPtr<Node> inputNode = findNodeNoMutex(ids[idsIndex]);
                        if(inputNode.valid())
                        {
                           ossimRefPtr<ossimConnectableObject> inputConnectable = inputNode->getObjectAsConnectable();
                           if(inputConnectable.valid())
                           {
                              connectable->connectMyInputTo(inputConnectable.get());
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }   
   if(!nodes.empty()&&callback.valid())
   {
      callback->nodesAdded(nodes);
   }
   return result;
}

bool ossimGui::DataManager::removeIndexMapping(Node* node)
{
   bool objectFound = false;
   NodeMapType::iterator nodeMapIter=m_nodeMap.find(node->getObject());
   if(nodeMapIter!=m_nodeMap.end())
   {
      objectFound = true;
      m_nodeMap.erase(nodeMapIter);
   }
   NodeIdMapType::iterator idMapIter=m_idMap.find(node->id());
   if(idMapIter!=m_idMap.end())
   {
      objectFound = true;
      m_idMap.erase(idMapIter);
   }
   
   return objectFound;
}
ossimGui::DataManager::Node* ossimGui::DataManager::findNode(const ossimId& id)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   return findNodeNoMutex(id);
}

ossimGui::DataManager::Node* ossimGui::DataManager::findNodeNoMutex(const ossimId& id)
{
   Node* result = 0;
   NodeIdMapType::iterator idMapIter=m_idMap.find(id);
   
   if(idMapIter != m_idMap.end())
   {
      result = idMapIter->second.get();
   }
   
   return result;
}

void ossimGui::DataManager::addIndexMapping(Node* node)
{
   if(node)
   {
      m_idMap.insert(std::make_pair(node->id(), node));
      m_nodeMap.insert(std::make_pair(node->getObject(), node));
   }
}

void ossimGui::DataManager::findInputConnectionIds(std::vector<ossimId>& result,
                                                   const ossimKeywordlist& kwl,
                                                   const ossimString& prefix)
{
   ossimString copyPrefix = prefix;
   ossim_uint32 idx = 0;
   
   ossimString regExpression =  ossimString("^") + ossimString(prefix) + "input_connection[0-9]+";
   vector<ossimString> keys =
   kwl.getSubstringKeyList( regExpression );
   
   ossim_int32 offset = (ossim_int32)(copyPrefix+"input_connection").size();
   ossim_uint32 numberOfKeys = (ossim_uint32)keys.size();
   std::vector<int> numberList(numberOfKeys);
   for(idx = 0; idx < numberList.size();++idx)
   {
      ossimString numberStr(keys[idx].begin() + offset,
                            keys[idx].end());
      numberList[idx] = numberStr.toInt();
   }
   std::sort(numberList.begin(), numberList.end());
   copyPrefix += ossimString("input_connection");
   for(idx=0;idx < numberList.size();++idx)
   {
      const char* lookup = kwl.find(copyPrefix,ossimString::toString(numberList[idx]));
      if(lookup)
      {
         ossim_int64 id = ossimString(lookup).toInt64();
         result.push_back(ossimId(id));
      }
   }
}

void ossimGui::DataManager::setExploitationMode(int expMode)
{
   m_exploitationMode = static_cast<ExploitationModeType> (expMode);

   for(ossim_uint32 idx = 0; idx < m_displayList.size();++idx)
   {
      ConnectableDisplayObject* displayObj = m_displayList[idx]->getObjectAs<ConnectableDisplayObject>();
      if(displayObj && displayObj->display())
      {
         ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
         subWindow->scrollWidget()->setExploitationMode(expMode);
      }
   }
}

void ossimGui::DataManager::setAutoMeasActive(const bool state)
{
   for(ossim_uint32 idx = 0; idx < m_displayList.size();++idx)
   {
      ConnectableDisplayObject* displayObj = m_displayList[idx]->getObjectAs<ConnectableDisplayObject>();
      if(displayObj && displayObj->display())
      {
         ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
         subWindow->scrollWidget()->setAutoMeasActive(state);
      }
   }
}

void ossimGui::DataManager::syncImagesTo(ossimRefPtr<DataManager::Node> node)
{
   ConnectableDisplayObject* displayObj = node->getObjectAs<ConnectableDisplayObject>();

   // Get point from sync image
   if(displayObj && displayObj->display())
   {     
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      ossimDpt vp = subWindow->scrollWidget()->getLastClickedPoint();
      syncImagesTo(vp, node);
   }
}


void ossimGui::DataManager::syncImagesTo(const ossimDpt& sp, ossimRefPtr<DataManager::Node> node)
{
   ConnectableDisplayObject* displayObj = node->getObjectAs<ConnectableDisplayObject>();
   ossim_uint32 idxLayer = 0;
   ossimGpt gp;
   gp.makeNan();

   // Get the ground point
   if(displayObj && displayObj->display())
   {
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      ossimImageSource* src = subWindow->scrollWidget()->layers()->layer(idxLayer)->chain();
      ossimGui::GatherImageViewProjTransVisitor visitor;
      src->accept(visitor);
      if (visitor.getTransformList().size() == 1)
      {
         ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
         if (ivtg.valid())
         {
            ivtg->viewToGround(sp, gp);
            if (gp.isHgtNan())
            {
               ossim_float64 hgt =
                  ossimElevManager::instance()->getHeightAboveEllipsoid(gp);
               gp.height(hgt);
            }
         }
      }
   }

   // Reproject and move all images to sync point
   if (!gp.hasNans())
   {
      for(ossim_uint32 idx = 0; idx < m_displayList.size(); ++idx)
      {
         ConnectableDisplayObject* displayObj =
            m_displayList[idx]->getObjectAs<ConnectableDisplayObject>();
         if(displayObj && displayObj->display())
         {
            ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
            ossimImageSource* src = subWindow->scrollWidget()->layers()->layer(idxLayer)->chain();
            ossimGui::GatherImageViewProjTransVisitor visitor;
            src->accept(visitor);
            if (visitor.getTransformList().size() == 1)
            {
               ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
               if (ivtg.valid())
               {
                  ossimDpt vp;
                  subWindow->getImageActions()->fullRes();
                  ivtg->groundToView(gp, vp);
                  subWindow->scrollWidget()->setPositionGivenView(vp);
                  subWindow->scrollWidget()->setLastClickedPoint(vp);

                  // ossimDpt ip;
                  // ivtg->viewToImage(vp, ip);
                  // std::cout<<"\n ...set position to "<<ip<<std::endl;
               }
            }
         }
      }
   }
}


bool ossimGui::DataManager::setAutoMeasureResults(NodeListType& nodes,
                                                  ossimTieMeasurementGeneratorInterface* tGen)
{
   bool measOK = true;
   ossim_uint32 idxLayer = 0;

   ConnectableDisplayObject* displayObj;
   ImageMdiSubWindow* subWindow;

   // Load ossimImageSource vector
   std::vector<ossimImageSource*> src;
   NodeListType::iterator iter = nodes.begin();
   while(iter != nodes.end())
   {
      displayObj = (*iter)->getObjectAs<ConnectableDisplayObject>();
      subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      src.push_back(subWindow->scrollWidget()->layers()->layer(idxLayer)->chain());
      ++iter;
   }

   // Load measurements
   ossimRefPtr<IvtGeomTransform> ivtg;
   ossimGui::GatherImageViewProjTransVisitor visitor;
   ossimDpt scenePoint;
   RegistrationOverlay* currentOverlay;
   for (int m=0; m<tGen->numMeasurements(); ++m)
   {
      // Create column for point in table
      m_miDialog->addObsPoint();

      // Add measurements to current column
      int img = 0;
      iter = nodes.begin();
      while(iter != nodes.end())
      {
         currentOverlay = regOverlayForNode(*iter);
         src[img]->accept(visitor);
         ivtg = visitor.getTransformList()[0].get();

         ossimDpt pt = tGen->pointIndexedAt(img,m);
// std::cout<<"measx "<<m<<", img "<<img<<" = "<<pt<<endl;
         if (!pt.hasNans())
         {
            ivtg->imageToView(pt, scenePoint);
            currentOverlay->addPoint(scenePoint, pt);
         }

         ++img;
         ++iter;
      }
   }

   return measOK;
}


bool ossimGui::DataManager::intersectRays(NodeListType& nodes)
{
   bool intersectionOK;
   ostringstream report;

   DptSet_t measSet;
   NodeListType::iterator iter = nodes.begin();
   std::vector<ossimRefPtr<ossimProjection> > proj;
   m_imgSet = new ossimSensorModelTuple();

   int kk = 0;
   ossim_uint32 idxLayer = 0;
   ossimDpt imgPt;

   report<<"\nSingle-ray..."<<endl;

   while(iter != nodes.end())
   {
      bool isActive = false;
      MetricOverlay* currentOverlay = metOverlayForNode(*iter);

      // Get current point
      if (currentOverlay->getImgPoint(currentOverlay->getCurrentId(), imgPt, isActive))
      {

        if (isActive)
        {

         ConnectableDisplayObject* displayObj = (*iter)->getObjectAs<ConnectableDisplayObject>();
         
         if(displayObj && displayObj->display())
         {
            ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
            ossimImageSource* src = subWindow->scrollWidget()->layers()->layer(idxLayer)->chain();
            ossimGui::GatherImageViewProjTransVisitor visitor;
            src->accept(visitor);

            if (visitor.getTransformList().size() == 1)
            {
               // Transform to true image coordinates and populate measurement set
               ossimRefPtr<IvtGeomTransform> ivtg = visitor.getTransformList()[0].get();
               if (ivtg.valid())
               {
                  // Save coordinates
                  measSet.push_back(imgPt);

                  // Single-ray drop comparison
                  ossimGpt gp;
                  ivtg->imageToGround(imgPt, gp);
                  report<<" Image "<<kk+1<<": "<<gp<<endl;
               }

               // Populate sensor model set
               ossimRefPtr<ossimImageGeometry> geom = ivtg->getGeom();
               if (geom != NULL)
               {
                  proj.push_back(geom->getProjection());
                  if (proj.back() != NULL)
                  {
                     ossimSensorModel* m = dynamic_cast<ossimSensorModel*>(proj.back().get());
                     if(m)
                     {
                        m_imgSet->addImage(m);
                     }
                  }
               }
            }
         }

        }
      }
      ++iter;
      ++kk;
   }

   ossimEcefPoint intECF;
   NEWMAT::Matrix covMat(3,3);

   // Execute multi-ray intersection solution
   ossimSensorModelTuple::IntersectStatus stat = m_imgSet->intersect(measSet, intECF, covMat);

   ossimGpt intG(intECF);
   double dh = ossimGeoidManager::instance()->offsetFromEllipsoid(intG);
   double hgtMSL = intG.height() - dh;

   ossimString statS;
   if (stat==0)
   {
      statS = "OPERATION_SUCCESS";
      intersectionOK = true;
   }
   else if (stat==1)
   {
      statS = "ERROR_PROP_UNAVAILABLE";
      intersectionOK = true;
   }
   else 
   {
      statS = "OPERATION_FAIL";
      intersectionOK = false;
   }

   report << "\nMulti-ray..." << std::endl;

   if (stat==0 || stat==1)
   {
      report << setprecision(15);
      report << "  Position: (" << intG.latd() << ", " << intG.lond() << ") DD" << std::endl;
      report << setprecision(3);
      report << "       HAE: " << intG.height() << " m" << std::endl;
      report << "       MSL: " << hgtMSL << " m" << std::endl;
      report << setprecision(1);
      report << "      ECEF: (" << intECF[0] << ", " << intECF[1] << ", " << intECF[2] << ") m" << std::endl;
   }
   report << "\n  Status: " << statS << std::endl;

   m_miDialog->setPointPositionContent(report.str());

   delete m_imgSet;
   return intersectionOK;
}


bool ossimGui::DataManager::clearCurrentPoint(NodeListType& nodes)
{
   bool clearOK = true;
   NodeListType::iterator iter = nodes.begin();

   while(iter != nodes.end())
   {
      MetricOverlay* currentOverlay = metOverlayForNode(*iter);
      currentOverlay->reset();

      ++iter;
   }

   return clearOK;
}


bool ossimGui::DataManager::loadImageGeometries(NodeListType& nodes)
{
   bool status = true;
   m_imgGeoms.clear();
   NodeListType::iterator iter = nodes.begin();
   while(iter != nodes.end())
   {
      ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
      if(connectable)
      {
         ossimTypeNameVisitor visitor("ossimImageHandler");
         connectable->accept(visitor);
         ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
         ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry(*input->getImageGeometry());
         m_imgGeoms.push_back(geom);
      }
      ++iter;
   }
   return status;
}


bool ossimGui::DataManager::saveImageGeometries(NodeListType& nodes)
{
   bool status = true;
   NodeListType::iterator iter = nodes.begin();
   ossim_uint32 k = 0;

   ossimLocalTm stamp;
   stamp.now();
   ostringstream tmp;
   int fmt = ossimLocalTm::ossimLocalTmFormatDMY|
             ossimLocalTm::ossimLocalTmFormatMonText|
             ossimLocalTm::ossimLocalTmFormatPadMon|
             ossimLocalTm::ossimLocalTmFormatYearShort;
   tmp<<"GeoCell_";
   stamp.printDate(tmp, fmt);
   tmp<<"_";
   stamp.printTime(tmp);
   string adjTag(tmp.str());

   while(iter != nodes.end())
   {
      // Skip control image
      if (!regOverlayForNode(*iter)->isControlImage())
      {
         ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
         if(connectable)
         {
            ossimTypeNameVisitor visitor("ossimImageHandler");
            connectable->accept(visitor);
            ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
            input->setImageGeometry(m_imgGeoms[k].get());

            ossimAdjustableParameterInterface* iface = input->getImageGeometry()->getAdjustableParameterInterface();
            if (iface != NULL)
            {
               iface->setAdjustmentDescription(adjTag);
            }
         }
      }
      ++iter;
      ++k;
   }
   return status;
}


bool ossimGui::DataManager::registerImages(NodeListType& nodes)
{
   bool solutionOK = false;
   bool useRegistration = true;

   ossimObservationSet obsSet;

   ossimString id;
   ossimColumnVector3d gSigmas;
   ossimGpt gp;

   // Load copies of current image geometries
   loadImageGeometries(nodes);

   // Load the observation set
   for (ossim_uint32 obs=0; obs<m_miDialog->getNumObs(); ++obs)
   {
      id = m_miDialog->getIdByIndex(obs);
      ossimDpt imgPt;

      // Initialize ground point
      gp.makeNan();
      gSigmas[0] = 50;
      gSigmas[1] = 50;
      gSigmas[2] = 50;

      ossimRefPtr<ossimPointObservation> pt = new ossimPointObservation(gp, id, gSigmas);

      // Iterate over nodes to get image measurements
      NodeListType::iterator iter = nodes.begin();
      while(iter != nodes.end())
      {
         bool isActive = false;
         RegistrationOverlay* currentOverlay = regOverlayForNode(*iter);

         if (currentOverlay->getImgPoint(id, imgPt, isActive))
         {
            if (isActive)
            {
               // Get image handler
               ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
               if(connectable)
               {
                  ossimTypeNameVisitor visitor("ossimImageHandler");
                  connectable->accept(visitor);
                  ossimRefPtr<ossimImageHandler> input =
                     dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
                  if ( input.valid() )
                  {
                     // If control, set ground coordinates and reset sigmas
                     if (currentOverlay->isControlImage())
                     {
                        // Point drop to get control coordinates
                        ossimGpt worldPt;
                        input->getImageGeometry()->localToWorld(imgPt, worldPt);
                        if (worldPt.isHgtNan())
                        {
                           ossim_float64 hgt =
                              ossimElevManager::instance()->getHeightAboveEllipsoid(worldPt);
                           worldPt.height(hgt);
                        }
                        pt->Gpt() = worldPt;

                        // TODO  Tighten up control sigmas
                        pt->setGroundSigmas(1,1,1);
                     }
                     else
                     {
                        // Get image filename
                        ossimFilename filename = input->getFilename().expand();
                        
                        // Add measurement to point observation
                        pt->addMeasurement(imgPt, filename);
                     }
                  }
               }
            }
         }
         
         ++iter;
      }
      
      // Add point observation to set
      if (pt->numMeas() > 0)
      {
         obsSet.addObservation(pt);
      }
   }
   

   // Set image geometries prior to adjustment
   if (useRegistration)
   {
      NodeListType::iterator iter = nodes.begin();
      ossim_uint32 k = 0; 
      while(iter != nodes.end())
      {
         ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
         if(connectable)
         {
            ossimTypeNameVisitor visitor("ossimImageHandler");
            connectable->accept(visitor);
            ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
            
            if(input.valid())
            {
               ossimFilename filename = input->getFilename().expand();

               for (ossim_uint32 i=0; i<obsSet.numImages(); ++i)
               {
                  if (filename == obsSet.imageFile(i))
                  {
                     // If control image, lock all parameters (TODO not needed if image removed?)
                     if (regOverlayForNode(*iter)->isControlImage())
                     {
                        ossimAdjustableParameterInterface* iface = m_imgGeoms[k]->getAdjustableParameterInterface();
                        if (iface != NULL)
                        {
                           iface->lockAllParametersCurrentAdjustment();
                        }
                     }

                     obsSet.setImageGeom(i, m_imgGeoms[k].get());
                  }
               }
            }

         }
         ++iter;
         ++k;
      }
   }


   // std::cout<<"\nObservation Summary...";
   ossim_uint32 idx = 0;
   for (ossim_uint32 no=0; no<obsSet.numObs(); ++no)
   {
      // std::cout<<"\n Obs: "<<no+1;
      for (ossim_uint32 meas=0; meas<obsSet.observ(no)->numMeas(); ++meas)
      {
         NEWMAT::Matrix coords(1,2);
         obsSet.observ(no)->getMeasurement(meas, coords);
         ++idx;
         // std::cout<<"\n  Meas: "<<imIdx+1<<"  "<<coords(1,1)<<","<<coords(1,2);
      }
   }

   ostringstream report;
   ossimAdjustmentExecutive adjExec(report);

   if (adjExec.initializeSolution(obsSet))
   {
      if (adjExec.isValid())
      {
         solutionOK = adjExec.runSolution();
         adjExec.summarizeSolution();
      }
   }

   m_miDialog->setRegistrationReportContent(report.str());

   return solutionOK;
}


bool ossimGui::DataManager::resetMode(NodeListType& /* nodes */)
{
   return true;
}


bool ossimGui::DataManager::resetAdj(NodeListType& nodes)
{
   NodeListType::iterator iter = nodes.begin();

   while(iter != nodes.end())
   {
      ossimConnectableObject* connectable = (*iter)->getObjectAs<ossimConnectableObject>();
      if(connectable)
      {
         ossimTypeNameVisitor visitor("ossimImageHandler");
         connectable->accept(visitor);
         if(visitor.getObjects().size() > 0)
         {
            ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
            
            if(input.valid())
            {
               ossimRefPtr<ossimImageGeometry> geom = input->getImageGeometry();
               if(geom.valid())
               {
                  ossimAdjustableParameterInterface* adjustableParamterInterface =
                     geom->getAdjustableParameterInterface();

                  if(adjustableParamterInterface)
                  {
                     adjustableParamterInterface->setDirtyFlag(true);
                     adjustableParamterInterface->eraseAdjustment(true);
                     if(adjustableParamterInterface->getNumberOfAdjustments() < 1)
                     {
                        adjustableParamterInterface->initAdjustableParameters();
                     }                  
                  }
               }
            }
         }
      }
      ++iter;
   }
   return true;
}

ossimGui::RegistrationOverlay* ossimGui::DataManager::regOverlayForNode(ossimRefPtr<Node> node)
{
   RegistrationOverlay* regOverlay = 0;

   ConnectableDisplayObject* displayObj = node->getObjectAs<ConnectableDisplayObject>();
   if(displayObj && displayObj->display())
   {
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      regOverlay = subWindow->scrollWidget()->regOverlay();
   }

   return regOverlay;
}

ossimGui::MetricOverlay* ossimGui::DataManager::metOverlayForNode(ossimRefPtr<Node> node)
{
   ossimGui::MetricOverlay* metOverlay = 0;

   ConnectableDisplayObject* displayObj = node->getObjectAs<ConnectableDisplayObject>();
   if(displayObj && displayObj->display())
   {
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      metOverlay = subWindow->scrollWidget()->metOverlay();
   }

   return metOverlay;
}

