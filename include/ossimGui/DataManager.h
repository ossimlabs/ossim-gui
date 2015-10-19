#ifndef ossimDataManager_HEADER
#define ossimDataManager_HEADER
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimTieMeasurementGeneratorInterface.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossimGui/Export.h>
#include <vector>
#include <map>
#include <QtCore/QString>
#include <QtGui/QMdiArea>
#include <ossim/projection/ossimSensorModelTuple.h>


namespace ossimGui{

   class RegistrationOverlay;
   class MetricOverlay;
   class MultiImageDialog;

   class OSSIMGUI_DLL DataManager : public ossimObject
   {
   public:
      class OSSIMGUI_DLL Node : public ossimObject
      {
      public:
         Node(ossimRefPtr<ossimObject> source=0, 
              const ossimString& name=ossimString(), const ossimString& description=ossimString());
         virtual ~Node()
         {
            m_object = 0;
         }
         ossimObject* getObject();
         const ossimObject* getObject()const;
         ossimConnectableObject* getObjectAsConnectable();
         const ossimConnectableObject* getObjectAsConnectable()const;
         const QString& name()const{return m_name;}
         const QString& description()const{return m_description;}
         void setName(const QString& name){m_name = name;}
         void setDescription(const QString& value){m_description = value;}
         void setObject(ossimObject* obj);
         bool supportsInputs()const;
         template <class T>
         T* getObjectAs(){return dynamic_cast<T*>(m_object.get());}
         template <class T>
         const T* getObjectAs()const{return dynamic_cast<const T*>(m_object.get());}
         virtual bool saveState(ossimKeywordlist& kwl, const ossimString& prefix)const;
         virtual bool loadState(const ossimKeywordlist& kwl, const ossimString& prefix);
         const ossimId& id()const{return m_id;}
         virtual void accept(ossimVisitor& visitor)
         {
            if(!visitor.stopTraversal()&&!visitor.hasVisited(this))
            {
               visitor.visit(this);
               if(m_object.valid())
               {
                  m_object->accept(visitor);
               }
            }
         }
      protected:
         void setId();
         mutable OpenThreads::Mutex m_mutex;
         
         QString m_name;
         QString m_description;
         mutable ossimId m_id;
         ossimRefPtr<ossimObject> m_object;
      };
      typedef std::vector<ossimRefPtr<Node> >            NodeListType;
      typedef std::map<ossimObject*, ossimRefPtr<Node> > NodeMapType;
      typedef std::map<ossimId, ossimRefPtr<Node> >      NodeIdMapType;
      
      class OSSIMGUI_DLL Callback : public ossimReferenced
      {
      public:
         Callback():ossimReferenced(),m_enabled(true){}
         
         virtual void nodeRemoved(ossimRefPtr<Node> node)
         {
            DataManager::NodeListType nodes;
            nodes.push_back(node.get());
            nodesRemoved(nodes);
         }
         virtual void nodeAdded(ossimRefPtr<Node> node)
         {
            DataManager:: NodeListType nodes;
            nodes.push_back(node.get());
            nodesAdded(nodes);
         }
         virtual void nodesRemoved(DataManager::NodeListType& /* nodes */){}
         virtual void nodesAdded(DataManager::NodeListType& /* nodes */){}
         void setEnabled(bool flag){m_enabled = flag;}
         bool enabled()const{return m_enabled;}
      protected:
         bool m_enabled;
      };
      
      
      enum ExploitationModeType
      {
         NO_MODE=0,
         REGISTRATION_MODE,
         GEOPOSITIONING_MODE,
         MENSURATION_MODE
      };

      DataManager();
      ossimRefPtr<ossimGui::DataManager::Node> findNode(ossimObject* obj);
      bool remove(ossimRefPtr<Node> obj, bool notifyFlag=true);
      bool remove(NodeListType& nodes, bool notifyFlag=true);
      
      ossimRefPtr<ossimGui::DataManager::Node> addSource(ossimRefPtr<ossimObject> obj, bool notifyFlag=true);
      ossimRefPtr<ossimGui::DataManager::Node> createDefaultImageChain(ossimRefPtr<Node> obj, bool notifyFlag=true);
      ossimRefPtr<ossimGui::DataManager::Node> createChainFromTemplate(const ossimString& templatChain, 
                                                                       ossimRefPtr<Node> input,
                                                                       bool notifyFlag=true);
      ossimRefPtr<ossimGui::DataManager::Node> createChainFromTemplate(const ossimKeywordlist& templatChain, 
                                                                       ossimRefPtr<Node> input,
                                                                       bool notifyFlag=true);
      
      ossimRefPtr<ossimGui::DataManager::Node> createDefaultCombinerChain(const ossimString& combinerType, NodeListType& nodeList, bool notifyFlag=true);
      ossimRefPtr<ossimGui::DataManager::Node> createDefault2dImageDisplay(ossimRefPtr<Node> input=0, bool notifyFlag=true);
      ossimRefPtr<ossimGui::DataManager::Node> node(ossimObject* obj);
      
      ossimRefPtr<ossimGui::DataManager::Node> createDefault3dPlanetaryDisplay(bool notifyFlag=true);
      
      virtual void accept(ossimVisitor& visitor);
      
      void setCallback(ossimRefPtr<Callback> callback){m_callback = callback.get();}
      void print();
      
      bool saveState(ossimKeywordlist& kwl, const ossimString& prefix=ossimString())const;
      bool loadState(const ossimKeywordlist& kwl, const ossimString& prefix = ossimString());
      void clear(bool notifyFlag=true);
      Node* findNode(const ossimId& id);
      void setMdiArea(QMdiArea* mdi){m_mdiArea = mdi;}
      QMdiArea* mdiArea(){return m_mdiArea;}
      
      const ossimString& defaultReprojectionChain()const{return m_defaultReprojectionChainTemplate;}
      const ossimString& defaultAffineChain()const{return m_defaultAffineChainTemplate;}

      void setExploitationMode(int expMode);
      void setAutoMeasActive(const bool state);
      ossim_int32 exploitationMode()const{return m_exploitationMode;}
      void syncImagesTo(const ossimDpt& sp, ossimRefPtr<DataManager::Node>);
      void syncImagesTo(ossimRefPtr<DataManager::Node> node);
      bool setAutoMeasureResults(NodeListType& nodes,
                                 ossimTieMeasurementGeneratorInterface* tgen);
      bool intersectRays(NodeListType& nodes);
      bool registerImages(NodeListType& nodes);
      bool loadImageGeometries(NodeListType& nodes);
      bool saveImageGeometries(NodeListType& nodes);
      bool resetMode(NodeListType& nodes);
      bool resetAdj(NodeListType& nodes);
      bool clearCurrentPoint(NodeListType& nodes);
      RegistrationOverlay* regOverlayForNode(ossimRefPtr<Node> node);
      MetricOverlay* metOverlayForNode(ossimRefPtr<Node> node);
      void setDialog(MultiImageDialog* miDialog) {m_miDialog = miDialog;}

  protected:
      bool removeIndexMapping(Node* node);
      void addIndexMapping(Node* node);
      Node* findNodeNoMutex(const ossimId& id);
      
      bool nodeExists(ossimObject* obj)const;
      void findInputConnectionIds(std::vector<ossimId>& result,
                                  const ossimKeywordlist& kwl,
                                  const ossimString& prefix);
      mutable OpenThreads::Mutex m_mutex;
      ossimRefPtr<Callback> m_callback;
      
      NodeListType m_sourceList;
      NodeListType m_chainList;
      NodeListType m_displayList;
      mutable NodeMapType   m_nodeMap;
      mutable NodeIdMapType m_idMap;
      
      ossimString m_defaultReprojectionChainTemplate;
      ossimString m_defaultAffineChainTemplate;
      QMdiArea*   m_mdiArea; 

      ossimSensorModelTuple* m_imgSet;
      ExploitationModeType m_exploitationMode;
      MultiImageDialog* m_miDialog;
      std::vector<ossimRefPtr<ossimImageGeometry> > m_imgGeoms;

   };
}   
#endif
