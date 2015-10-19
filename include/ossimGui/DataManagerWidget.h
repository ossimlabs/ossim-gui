#ifndef ossimGuiDataManagerWidget_HEADER
#define ossimGuiDataManagerWidget_HEADER 1
#include <ossimGui/DataManager.h>
#include <QtGui/QTreeView>
#include <QtGui/QItemDelegate>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QStandardItem>
#include <QtGui/QMenu>
#include <QtCore/QModelIndex>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
//#include <QtGui/QProgressBar>
#include <ossimGui/Export.h>
#include <ossimGui/Event.h>
#include <ossimGui/DataManagerPropertyView.h>
#include <ossimGui/DisplayTimerJobQueue.h>
#include <ossimGui/ProgressWidget.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimTieMeasurementGeneratorInterface.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/parallel/ossimJobQueue.h>
#include <QtGui/QApplication>
class QMainWindow;

class ossimSensorModelTuple;

namespace ossimGui{
   
   class DataManagerWidget;
   class MultiImageDialog;
   class AutoMeasurementDialog;
   class RegPoint;
   
   class OSSIMGUI_DLL DataManagerItem : public QTreeWidgetItem
   {
   public:
      DataManagerItem(QTreeWidget* parent):QTreeWidgetItem(parent),m_markedForDeletion(false){}
      DataManagerItem(QTreeWidgetItem* parent):QTreeWidgetItem(parent),m_markedForDeletion(false){}
      DataManagerItem():QTreeWidgetItem(),m_markedForDeletion(false){}
      virtual ~DataManagerItem()
      {
         m_object = 0;
      }
      DataManager* dataManager();
      DataManagerWidget* dataManagerWidget();
      template<class T>
      bool isAnyParentSelectedOfType()const
      {
         bool result = false;
         T* parentItem = dynamic_cast<T*>(parent());
         while(parentItem&&!result)
         {
            result = parentItem->isSelected();
            parentItem = dynamic_cast<T*>(parentItem->parent());
         }
         
         return result;
      }
      template<class T>
      T* itemAs(){return dynamic_cast<T*>(this);}
      template<class T>
      const T* itemAs()const{return dynamic_cast<T*>(this);}
      template<class T>
      T* parentItemAs(){return dynamic_cast<T*>(parent());}
      template<class T>
      const T* parentItemAs()const{return dynamic_cast<T*>(parent());}
      template<class T>
      T* findParentItemAs()
      {
         T* result = 0;
         QTreeWidgetItem* currentItem = this;
         while(!result&&currentItem)
         {
            result = dynamic_cast<T*> (currentItem);
            currentItem = currentItem->parent();
         }
         return result;
      }
      template<class T>
      const T* findParentItemAs()const
      {
         const T* result = 0;
         const QTreeWidgetItem* currentItem = this;
         while(!result&&currentItem)
         {
            result = dynamic_cast<const T*> (currentItem);
            currentItem = currentItem->parent();
         }
         return result;
      }
      
      virtual void setMarkForDeletion(bool flag)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_itemMutex);
         m_markedForDeletion = flag;
      }
      bool markedForDeletion()const
      {  
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_itemMutex);
         return m_markedForDeletion;
      }
      virtual void setObject(ossimObject* obj)
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_itemMutex);
         m_object = obj;
      }
      virtual DataManager::Node* objectAsNode(){return dynamic_cast<DataManager::Node*> (object());}
      virtual const DataManager::Node* objectAsNode()const{return dynamic_cast<const DataManager::Node*> (object());}
      virtual ossimObject* object()
      { 
         return m_object.get();
      }
      virtual const ossimObject* object()const
      {
         return m_object.get();
      }
      
      virtual void populateChildren(){}
      virtual void clearChildren();
      virtual void setNodeListenerEnabled(bool /* flag */){}
      virtual void reset(){}
   protected:
      mutable OpenThreads::Mutex m_itemMutex;
      bool m_markedForDeletion;
      ossimRefPtr<ossimObject> m_object;
   };
   
   class DataManagerNodeItem;
   class OSSIMGUI_DLL DataManagerNodeItemListener : public ossimConnectableObjectListener
   {
   public:
      DataManagerNodeItemListener(DataManagerNodeItem* item):m_nodeItem(item){}
      virtual void disconnectInputEvent(ossimConnectionEvent& /* event */);
      virtual void connectInputEvent(ossimConnectionEvent& /* event */);
      
      void setNodeItem(DataManagerNodeItem* item){m_nodeItem = item;}
      DataManagerNodeItem* nodeItem(){return m_nodeItem;}
   protected:
      DataManagerNodeItem* m_nodeItem;
   };
   
   class DataManagerInputConnectionFolder;
   class DataManagerPropertyFolder;
   class OSSIMGUI_DLL DataManagerNodeItem : public DataManagerItem, public ossimConnectableObjectListener
   {
   public:
      friend class DataManagerNodeItemListener;
      DataManagerNodeItem(DataManager::Node* node=0);
      virtual ~DataManagerNodeItem();
      virtual void setObject(ossimObject* obj);
      void getInputs(DataManager::NodeListType& result);
      virtual ossimObject* object(){return m_node.get();}
      virtual const ossimObject* object()const{return m_node.get();}
     // virtual void removeChildItem(DataManagerNodeItem* item);
      
      template<class T>
      T* parentAs(){return dynamic_cast<T*> (parent());}
      template<class T>
      const T* parentAs()const{return dynamic_cast<const T*> (parent());}
      
      virtual bool autoDelete()const;
      virtual void setAutoDelete(bool flag);
      virtual bool isCombiner()const;
      virtual bool isParentCombiner()const;
      virtual void refreshChildConnections();
      virtual void refreshChildProperties();
      virtual void populateChildren();
      virtual void initializePropertiesFromNode();
      virtual void setMarkForDeletion(bool flag);
    
      virtual void setNodeListenerEnabled(bool flag);
      virtual void reset();
   protected:
     // virtual void disconnectInputEvent(ossimConnectionEvent& /* event */);
     // virtual void connectInputEvent(ossimConnectionEvent& /* event */);
      ossimRefPtr<DataManager::Node>     m_node;  
      DataManagerNodeItemListener*       m_listener;
      bool                               m_combinerFlag;
      bool                               m_autoDelete;
      DataManagerInputConnectionFolder*  m_inputConnectionFolder;
      DataManagerPropertyFolder*         m_propertyFolder;
   };
   

   class OSSIMGUI_DLL DataManagerRawImageSourceItem : public DataManagerNodeItem
   {
   public:
      DataManagerRawImageSourceItem(DataManager::Node* node=0):DataManagerNodeItem(node){}
      virtual void reset();
   protected:
   };
   
   class OSSIMGUI_DLL DataManagerFolder : public DataManagerItem
   {
   public:
      DataManagerFolder():DataManagerItem(){}
      DataManagerFolder(QTreeWidget* parent):DataManagerItem(parent){}
      DataManagerFolder(QTreeWidgetItem* parent):DataManagerItem(parent){}
      virtual void dropItems(QList<DataManagerItem*>& /*chainItemList*/){}
      virtual void setMarkForDeletion(bool flag);
      virtual void setNodeListenerEnabled(bool flag);
     
   protected:
   };
   class DataManagerImageFilterFolder;
   class OSSIMGUI_DLL DataManagerImageFilterItem : public DataManagerItem
   {
   public:
      DataManagerImageFilterItem();
      virtual ~DataManagerImageFilterItem();
      virtual void populateChildren();
      DataManagerImageFilterFolder* folder();
      virtual void setObject(ossimObject* obj);
      
   protected:
      DataManagerPropertyFolder* m_properties;
   };
   
   class OSSIMGUI_DLL DataManagerImageFilterFolder : public DataManagerFolder
   {
   public:
      DataManagerImageFilterFolder();
      virtual void populateChildren();
      virtual void setObject(ossimObject* obj);
      
      /**
       * This will remove all filters from the managed object.
       */
      void removeFilters();
      void removeFilter(ossimObject* obj);
      void addFilterToFront(ossimObject* obj);
      void addFilterToEnd(ossimObject* obj);
      void insertFilterBefore(ossimObject* newObj, ossimObject* before);
      void insertFilterAfter(ossimObject* newObj, ossimObject* after);
      
   protected:
     // void printChain();
   };

   class OSSIMGUI_DLL DataManagerImageFolder : public DataManagerFolder
   {
   public:
      DataManagerImageFolder():DataManagerFolder(){}
      DataManagerImageFolder(QTreeWidget* parent):DataManagerFolder(parent){}
      DataManagerImageFolder(QTreeWidgetItem* parent):DataManagerFolder(parent){}
   protected:
   };
   
   class OSSIMGUI_DLL DataManagerRawImageSourceFolder : public DataManagerFolder
   {
   public:
      DataManagerRawImageSourceFolder():DataManagerFolder(){}
   protected:
   };
   
   class OSSIMGUI_DLL DataManagerImageChainFolder : public DataManagerFolder
   {
   public:
      DataManagerImageChainFolder():DataManagerFolder(){}
   protected:
      
   };
   class DataManagerPropertyItem;
   class OSSIMGUI_DLL DataManagerPropertyFolder : public DataManagerFolder
   {
   public:
      DataManagerPropertyFolder();
      virtual ~DataManagerPropertyFolder();
      
      virtual void populateChildren();
      
   protected:
   };
   class OSSIMGUI_DLL DataManagerPropertyItem : public DataManagerItem
   {
   public:
      DataManagerPropertyItem(QTreeWidget* parent):DataManagerItem(parent){}
      DataManagerPropertyItem(QTreeWidgetItem* parent):DataManagerItem(parent){}
      DataManagerPropertyItem():DataManagerItem(){}
      DataManagerPropertyFolder* propertyFolder();
      void setProperty(ossimProperty* prop);
      
   protected:
      ossimRefPtr<ossimProperty> m_property;
   };
   class OSSIMGUI_DLL DataManagerImageChainItem : public DataManagerNodeItem
   {
   public:
      DataManagerImageChainItem(DataManager::Node* node=0);
      virtual void dropItems(QList<DataManagerImageChainItem*>& chainItemList, 
                             DataManagerNodeItem* targetItem=0, 
                             bool insertBefore=true);
      virtual void populateChildren();
      virtual void clearChildren();
      
   protected:
      DataManagerImageFilterFolder* m_filters;
   };
   
   class OSSIMGUI_DLL DataManagerInputConnectionItem : public DataManagerItem
   {
   public:
      DataManagerInputConnectionItem(DataManager::Node* inputNode=0);
      virtual ~DataManagerInputConnectionItem();
      virtual void setObject(ossimObject* obj);
      virtual ossimObject* object(){return m_node.get();}
      virtual const ossimObject* object()const{return m_node.get();}
      
   protected:
      ossimRefPtr<DataManager::Node> m_node;
   };

   class OSSIMGUI_DLL DataManagerInputConnectionFolder : public DataManagerFolder
   {
   public:
      DataManagerInputConnectionFolder(DataManager::Node* node=0);
      virtual ~DataManagerInputConnectionFolder();
      virtual void setObject(ossimObject* node);
      virtual ossimObject* object(){return m_node.get();}
      virtual const ossimObject* object()const{return m_node.get();}
      
      void getInputs(DataManager::NodeListType& result);
      virtual void connect(QList<DataManagerItem*>& nodeItems, DataManagerInputConnectionItem* targetItem=0);
      virtual void disconnectSelected();
      virtual void populateChildren();
      ossimConnectableObject* connectableObject();
      const ossimConnectableObject* connectableObject()const;

   protected:
      ossimRefPtr<DataManager::Node> m_node;
   };
   
   class OSSIMGUI_DLL DataManagerDisplayItem : public DataManagerNodeItem
   {
   public:
      DataManagerDisplayItem(DataManager::Node* node=0);
      virtual ~DataManagerDisplayItem();
      virtual void setObject(ossimObject* obj);
     
   protected:
   //   DataManagerInputConnectionFolder* m_inputConnectionFolder;
   };
   
   class OSSIMGUI_DLL DataManagerDisplayFolder : public DataManagerFolder
   {
   public:
      DataManagerDisplayFolder();
   };
   
   class OSSIMGUI_DLL DataManagerImageWriterItem : public DataManagerNodeItem
   {
   public:
      DataManagerImageWriterItem(DataManager::Node* node=0);
      virtual ~DataManagerImageWriterItem();
     // virtual void setNode(DataManager::Node*);
      virtual void execute();
      
   protected:
      
     // std::vector<ossimRefPtr<ossimConnectableObject> > m_duplicatedInputs;
      
     // DataManagerInputConnectionFolder* m_inputConnectionFolder;
   };
   
   class OSSIMGUI_DLL DataManagerImageFileWriterItem : public DataManagerImageWriterItem
   {
   public:
      DataManagerImageFileWriterItem(DataManager::Node* node=0);
      virtual ~DataManagerImageFileWriterItem();
      
   protected:
      DataManagerInputConnectionFolder* m_inputConnectionFolder;
   };
   
   class OSSIMGUI_DLL DataManagerImageWriterFolder : public DataManagerFolder
   {
   public:
      DataManagerImageWriterFolder();
   };
   
   
   class OSSIMGUI_DLL DataManagerJobItem : public DataManagerItem
   {
   public:
      DataManagerJobItem();
      virtual ~DataManagerJobItem();
      virtual void setJob(ossimJob* job);
      virtual void cancel();
      void setPercentComplete(double value)
      {
         if(m_progressBar)
         {
            m_progressBar->setValue(static_cast<int> (value));
         }
      }
   protected:
      class JobCallback : public ossimJobCallback
      {
      public:
         JobCallback(DataManagerJobItem* item, ossimJobCallback* next):ossimJobCallback(next),m_jobItem(item){}
         virtual void ready(ossimJob* job);
         virtual void started(ossimJob* job);
         virtual void finished(ossimJob* job);
         virtual void canceled(ossimJob* job);
         
         virtual void nameChanged(const ossimString& name, ossimJob* job);
         virtual void descriptionChanged(const ossimString& description, ossimJob* job);
         virtual void idChanged(const ossimString& id, ossimJob* job);
         
         virtual void percentCompleteChanged(double percentValue, ossimJob* job);
         
         DataManagerJobItem* m_jobItem;
      };
      
      ossimRefPtr<ossimJob> m_job;
      ossimRefPtr<JobCallback> m_jobCallback;
      QTreeWidgetItem* m_progressItem;
      QProgressBar* m_progressBar;
   };
   
   class OSSIMGUI_DLL DataManagerJobsFolder : public DataManagerFolder
   {
   public:
      typedef std::vector<ossimRefPtr<ossimJobQueue> > QueueListType;
      typedef std::map<ossimRefPtr<ossimJob>, QTreeWidgetItem* > JobMapType;
      DataManagerJobsFolder();
      DataManagerJobsFolder(QTreeWidget* parent);
      DataManagerJobsFolder(QTreeWidgetItem* parent);
      virtual ~DataManagerJobsFolder();
      void setQueue(ossimJobQueue* q);
      void removeStoppedJobs();
      void addJob(ossimJob* job)
      {
         m_jobsFolderMutex.lock();
         if(m_jobItemMap.find(job)==m_jobItemMap.end())
         {
            DataManagerJobItem* item = new DataManagerJobItem();
            addChild(item);
            item->setJob(job);
            m_jobItemMap.insert(make_pair(job, item));
         }
         m_jobsFolderMutex.unlock();
         stateChanged(job);
      }
      void removeJob(ossimJob* job)
      {
         m_jobsFolderMutex.lock();
         JobMapType::iterator iter = m_jobItemMap.find(job);
         
         if(iter!=m_jobItemMap.end())
         {
            delete (*iter).second;
            m_jobItemMap.erase(iter);
         }
         m_jobsFolderMutex.unlock();
      }
      void stateChanged(ossimJob* job)
      {
         m_jobsFolderMutex.lock();
         JobMapType::iterator iter = m_jobItemMap.find(job);
         if(iter!=m_jobItemMap.end())
         {
            if(job->isReady())
            {
               (*iter).second->setText(0, ("Ready: " + job->name()).c_str());
            }
            if(job->isCanceled())
            {
               if(!job->isStopped())
               {
                  (*iter).second->setText(0, ("Request Cancel: " + job->name()).c_str());
               }
               else if(job->isStopped())
               {
                  (*iter).second->setText(0, ("Canceled: " + job->name()).c_str());
               }

            }
            else if(job->isRunning())
            {
            
               (*iter).second->setText(0, ("Running: " + job->name()).c_str());
            }
            if(job->isStopped())
            {
               delete (*iter).second;
               m_jobItemMap.erase(iter);
            }
         }
         m_jobsFolderMutex.unlock();
      }
      void propertyChanged(ossimJob* job)
      {
         m_jobsFolderMutex.lock();
         JobMapType::iterator iter = m_jobItemMap.find(job);
         if(iter!=m_jobItemMap.end())
         {
            (*iter).second->setText(0, job->name().c_str());
         }
         m_jobsFolderMutex.unlock();
      }
      void percentCompleteChanged(ossimJob* job, double percentComplete)
      {
         m_jobsFolderMutex.lock();
         JobMapType::iterator iter = m_jobItemMap.find(job);
         if(iter!=m_jobItemMap.end())
         {
            DataManagerJobItem* jobItem = dynamic_cast<DataManagerJobItem*>((*iter).second);
            if(jobItem)
            {
               jobItem->setPercentComplete(percentComplete);
            }
         }
         m_jobsFolderMutex.unlock();
      }
   protected:
      class JobQueueCallback : public ossimJobQueue::Callback
      {
      public:
         JobQueueCallback(DataManagerJobsFolder* f):m_folder(f){}
         virtual void adding(ossimJobQueue* /* q */, ossimJob* job)
         {
            if(m_folder)
            {
               DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_ADDED);
               e->setJobList(job);
               QCoreApplication::postEvent(m_folder->treeWidget(), e);
            }
         }
         virtual void removed(ossimJobQueue* /*q*/, ossimJob* /*job*/)
         {
            if(m_folder)
            {
            }
         }
         
         DataManagerJobsFolder* m_folder;
      };
      
      QueueListType m_queues;   
      ossimRefPtr<JobQueueCallback> m_jobQueueCallback;
      mutable OpenThreads::Mutex m_jobsFolderMutex;
      JobMapType m_jobItemMap;
   };
   
   class OSSIMGUI_DLL DataManagerWidget : public QTreeWidget
   {
      Q_OBJECT
   public:
      friend class DataManagerNodeItem;
      DataManagerWidget(QWidget* parent=0);
      
      //virtual void setDataManager(ossimRefPtr<DataManager> manager);
      DataManager* dataManager(){return m_dataManager.get();}
      const DataManager* dataManager()const{return m_dataManager.get();}
      
      QMenu* createMenu(QList<DataManagerItem*>& selection, DataManagerItem* activeItem=0);
      QMainWindow* mainWindow();
      void setJobQueue(ossimJobQueue* que){m_jobQueue = que; m_rootJobsFolder->setQueue(que);}
      ossimJobQueue* jobQueue(){return m_jobQueue.get();}
      ossimJobQueue* displayQueue(){return m_displayQueue.get();}
      void setDisplayQueue(ossimJobQueue* q){m_displayQueue = q;}
      bool openDataManager(const ossimFilename& file);
      void refresh();
      QModelIndex indexFromDataManagerItem(DataManagerItem* item, int col=0);
      virtual void unselectAll();

      /** @return the last opened directory. */
      const ossimFilename& getLastOpenedDirectory() const;

   public slots:
      virtual void buildOverviewsForSelected(QAction* action);
      virtual void buildOverviewsForSelected(const QString& type);
      virtual void buildOverviewsForSelected();
      virtual void createFullHistogramsForSelected();
      virtual void createFastHistogramsForSelected();
      
    
      virtual void createDefaultChain();
      virtual void createAffineChain();
      virtual void createMapProjectedChain();
      virtual void createImageNormalsChain();

      virtual void registrationExploitationSelected();
      virtual void geoPositioningExploitationSelected();
      virtual void mensurationExploitationSelected();

      virtual void miDialogDestroyed();

      void aOverBMosaic();
      void blendMosaic();
      void featherMosaic();
      void hillShadeCombiner();
      void factoryCombiner();
      
      void geographicView();
      void scaledGeographicView();
      void utmView();
      
      void exportSelected();
      void showSelected();
      void swipeSelected();
      void deleteSelected();
   
      void cancelSelected();
      
      virtual void openLocalImage();

      /**
       * @brief Opens local image(s).
       *
       * This method is like openLocalImage method but is interactive in that
       * it will bring up a dialog box if needed to select entries or
       * potentially different geometry options.
       */
      virtual void openLocalImageInteractive();

      virtual void openJpipImage();
      
      virtual void createTiffWriter();
      virtual void createJpegWriter();
      virtual void createWriterFromFactory();
      
      virtual void executeSelected();
      
      virtual void displayCropViewport();
      
      virtual void removeAllFilters();
      virtual void addFilterToFront();
      virtual void addFilterToEnd();
      virtual void insertFilterBefore();
      virtual void insertFilterAfter();
      virtual void removeFilter();
      
      virtual void planetaryView();

      // Multi-image dialog
      virtual void miDialog(const int& mode);
      virtual void miSync();
      virtual void miSync(ossimGui::RegPoint*, ossimRefPtr<DataManager::Node>);
      virtual bool miDrop(DataManager::NodeListType& nodes);
      virtual bool miReg(DataManager::NodeListType& nodes);
      virtual bool miResetMode(DataManager::NodeListType& nodes);
      virtual bool miClearCurrentPoint(DataManager::NodeListType& nodes);
      virtual bool miAcceptReg(DataManager::NodeListType& nodes);
      virtual bool miResetReg(DataManager::NodeListType& nodes);
      virtual bool miAutoMeas(DataManager::NodeListType& nodes);
      virtual bool miAcceptMeas(DataManager::NodeListType& nodes);
      virtual bool miDismissMeas();

      // Auto-measurement dialog
      virtual void amDialog(DataManager::NodeListType& nodes);
      bool amDialogIsActive()const {return m_amDialogActive;}
      

      void	itemChanged( QTreeWidgetItem * item, int column );
      void	itemCollapsed (  QTreeWidgetItem * item );
      void	itemExpanded (  QTreeWidgetItem * item );     
      
   signals:
      void resetMode();

   protected:
      /*************************** REFRESH VISITOR **************************/
      class OSSIMGUI_DLL RefreshVisitor : public ossimVisitor
      {
      public:
         RefreshVisitor(DataManagerWidget* widget):m_widget(widget){}
         RefreshVisitor(const RefreshVisitor& src):m_widget(src.m_widget){}
         virtual ossimRefPtr<ossimVisitor> dup()const{return new RefreshVisitor(*this);}
         
         virtual void visit(ossimObject* obj);
         
         DataManagerWidget* m_widget;
      };
      friend class RefreshVisitor;                           
      /**********************************************************************/
      
      /*************************** DATAMANAGER CALLBACK **************************/
      class DataManagerCallback : public DataManager::Callback
      {
      public:
         DataManagerCallback(DataManagerWidget* widget);
         virtual void nodesRemoved(DataManager::NodeListType& nodes);
         virtual void nodesAdded(DataManager::NodeListType& nodes);
      protected:
         DataManagerWidget* m_dataManagerWidget;
      };
      friend class DataManagerCallback;
      /***************************************************************************/
      template<class T>
      QList<T*> grabSelectedChildItemsOfType()
      {
         QList<T*> result;
         
         QList<QTreeWidgetItem*> selectedNodes = selectedItems();
         QList<QTreeWidgetItem*>::iterator iter = selectedNodes.begin();
         while(iter!=selectedNodes.end())
         {
            T* node = dynamic_cast<T*> ((*iter));
            if(node)
            {
               result.push_back(node);
            }
            ++iter;
         }
         
         return result;
      }
      
      void combineImagesWithType(const QString& classType);
      void createWriterFromType(const QString& classType);
      virtual void incrementScrollBars(const QPoint& pos);
      
      /***************************** QT events **************************/
      
      virtual void	dragEnterEvent ( QDragEnterEvent * event );
      virtual void	dragLeaveEvent ( QDragLeaveEvent * event );
      virtual void	dragMoveEvent ( QDragMoveEvent * event );
      virtual void	dropEvent ( QDropEvent * event );
      void keyPressEvent ( QKeyEvent * e);
      void mousePressEvent(QMouseEvent *e);
      void mouseMoveEvent(QMouseEvent *e);
      void mouseReleaseEvent(QMouseEvent * e);
      /*******************************************************************/
      virtual bool	event ( QEvent * e );      
      virtual void deleteSelectedItems();
      virtual void initialize();
      void populateTreeWithNodes(DataManager::NodeListType& nodes);

      ossimGui::DataManager::NodeListType getSelectedNodeList();
      
      ossimRefPtr<DataManager>         m_dataManager;
      ossimRefPtr<DataManagerCallback> m_dataManagerCallback;
      ossimRefPtr<ossimJobQueue>       m_jobQueue;
      ossimRefPtr<ossimJobQueue>       m_displayQueue;

      DataManagerImageFolder*          m_rootImageFolder;
      DataManagerJobsFolder*           m_rootJobsFolder;
      DataManagerRawImageSourceFolder* m_rawImageSources;
      DataManagerImageChainFolder*     m_imageChains;
     // DataManagerImageOutputFolder*    m_imageOutput;
      DataManagerDisplayFolder*        m_imageDisplays;
      DataManagerImageWriterFolder*    m_imageWriters;
      
      QPoint m_dragStartPosition;
      
      std::set<DataManagerItem*> m_activeItems;
      mutable OpenThreads::Mutex m_activeItemsMutex;
      
      // Registration-related members
      MultiImageDialog* m_miDialog;
      AutoMeasurementDialog* m_amDialog;
      ossimTieMeasurementGeneratorInterface* m_tGen;
      ossimRefPtr<ossimObject> m_tGenObj;
      bool m_amDialogAvailable;
      bool m_amDialogActive;

      /**
       * We will temporarily store the filter lists here from the factories
       */
      QStringList m_filterList;
      QStringList m_combinerList;
      
      ossimRefPtr<DataManager::Node> m_planetaryDisplayNode;

      ossimFilename m_lastOpenedDirectory; // For QFileDialog::getOpenFileNames
      
   }; // End: class DataManagerWidget 
   
} // End: namespace ossimGui{ 

#endif /** #ifndef ossimGuiDataManagerWidget_HEADER */
