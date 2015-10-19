#include <ossimGui/MainWindow.h>
#include <ossimGui/ImageWidget.h>
#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/DataManager.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/Event.h>
#include <ossimGui/About.h>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QToolBar>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>

#include <ossim/ossimVersion.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossimGui/Common.h>
#include <ossimGui/OpenImageUrlJob.h>

class ossimImageOpenJobCallback : public ossimJobCallback
{
public:
   virtual void finished(ossimJob* job)
   {
      ossimGui::ImageOpenEvent* imageOpenEvent = new ossimGui::ImageOpenEvent();
      ossimGui::OpenImageUrlJob* imageOpenJob = dynamic_cast<ossimGui::OpenImageUrlJob*> (job);
      if(imageOpenJob)
      {
         ossimGui::HandlerList& handlerList =imageOpenJob->handlerList();
         ossim_uint32 idx = 0;
         ossim_uint32 nImages = handlerList.size();
         for(idx = 0; idx < nImages; ++idx)
         {
            imageOpenEvent->handlerList().push_back(handlerList[idx]);
         }
      }
      if(!imageOpenEvent->handlerList().empty())
      {
         QCoreApplication::postEvent(m_object, imageOpenEvent);
      }
      else
      {
         delete imageOpenEvent; 
         imageOpenEvent=0;
      }
   }
   
   QObject* m_object;
};

ossimGui::MainWindow::MainWindow(QWidget* parent)
:QMainWindow(parent)
{
   setupUi(this);
   setWindowTitle("OSSIM Main Window");
   createAndSetMenuBar();
   statusBar()->showMessage(tr("Ready"));
   QToolBar* toolbar = addToolBar("Main Tool Bar");
   toolbar->setObjectName("mainToolbar");
   setAcceptDrops(true);
   m_stagerQueue  = new ossimJobMultiThreadQueue(new ossimJobQueue(), 4); 
   m_displayQueue = new DisplayTimerJobQueue();
   m_dataManagerWidget->setDisplayQueue(m_displayQueue.get());
   m_dataManager  = m_dataManagerWidget->dataManager();
   m_dataManager->setMdiArea(m_mdiArea);
   m_dataManagerWidget->setJobQueue(m_stagerQueue->getJobQueue());
  // createModeSelector(toolbar);
}

bool	ossimGui::MainWindow::event ( QEvent * e )
{
   if ( e )
   {
      ossimGui::EventId id = (ossimGui::EventId)(e->type());
      switch(id)
      {
         case IMAGE_OPEN_EVENT_ID:
         {
            ImageOpenEvent* ioEvent = dynamic_cast<ImageOpenEvent*> (e);
            if(ioEvent)
            {
               ossim_uint32 idx = 0;
               ossim_uint32 nImages = ioEvent->handlerList().size();
               for(idx = 0; idx<nImages;++idx)
               {
                  ossimRefPtr<ossimGui::DataManager::Node> result = m_dataManager->addSource(ioEvent->handlerList()[idx].get());
                  if(result.valid())
                  {
                     result = m_dataManager->createDefaultImageChain(result.get());
                  }
                  showNode(result.get());
               }
            }
            e->accept();
            break;
         }
         case DATA_MANAGER_EVENT_ID:
         {
            DataManagerEvent* dmEvent = dynamic_cast<DataManagerEvent*> (e);
            if(dmEvent)
            {
               switch(dmEvent->command())
               {
                  case DataManagerEvent::COMMAND_DISPLAY_NODE:
                  {
                     DataManager::NodeListType& nodes = dmEvent->nodeList(); 
                     ossim_uint32 idx = 0;
                     for(idx = 0; idx < nodes.size();++idx)
                     {
                        showNode(nodes[idx].get());
                     }
                     break;
                  }
                  default:
                  {
                     break;
                  }
               }
               e->accept();
            }
            break;
         }
         default:
         {
            // pass all user events to the data manager widget
            if(e->type()>=QEvent::User)
            {
               QObject* baseObject = static_cast<QObject*>(m_dataManagerWidget);
            
               return baseObject->event(e);
            }
            break;
         }
      }
   }
   return QMainWindow::event(e);
}

void ossimGui::MainWindow::dropEvent( QDropEvent * event )
{
   const QMimeData* data = event->mimeData();
   if(data)
   {
      if(data->hasUrls())
      {
#if 0
         // test to see if we dropped a geocell project on the canvas
         //
         if(m_url.scheme().toLower() == "file")
         {
            ossimFilename file = m_url.toLocalFile().toAscii().data();
            ossimString ext = file.ext().downcase();

            if(ext == "gcl" || ext == "geocell")
            {
               //std::cout << "GEOCELL PROJECT DROPPED\n";
               if(!m_dataManagerWidget->openDataManager(fileName))
               {
                  
               }
            }
         }
#endif

         QList<QUrl> urls = data->urls();
         QList<QUrl>::iterator iter = urls.begin();
         while(iter != urls.end())
         {
            QString input  = iter->toString();
            QString scheme = iter->scheme();
            ossimFilename projectFile = (*iter).toLocalFile().toAscii().data();
            ossimString ext = projectFile.ext().downcase();
            if((ext == "gcl" || ext=="geocell")&&projectFile.exists())
            {
               m_dataManagerWidget->openDataManager(projectFile);
            }           
            else
            {
               OpenImageUrlJob* job = new OpenImageUrlJob(*iter);
               job->setName("Open " + ossimString((*iter).toString().toAscii().data()));
               ossimImageOpenJobCallback* callback = new ossimImageOpenJobCallback();
               callback->m_object = this;
               job->setCallback(callback);
               m_stagerQueue->getJobQueue()->add(job);
            }
           
            ++iter;
         }
      }
      //PlanetMdiSubWindow* w = new PlanetMdiSubWindow();
     // m_mdiArea->addSubWindow(w);
    //  w->show();

   }
}

void ossimGui::MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
   event->acceptProposedAction();
}

QMenuBar* ossimGui::MainWindow::createAndSetMenuBar()
{
   if(m_menubar)
   {
      setMenuBar(0);
      delete m_menubar;
   }
   m_menubar = new QMenuBar(0);
   m_menubar->setObjectName(QString::fromUtf8("mainmenubar"));
   QMenu* menuFile   = new QMenu("File", m_menubar);
   QMenu* menuWindow = new QMenu("Window", m_menubar);
   QMenu* menuHelp = new QMenu("Help", m_menubar);
   menuWindow->setObjectName("windowMenu");
   menuFile->setObjectName("fileMenu");
   menuHelp->setObjectName("helpMenu");
   QAction* aboutAction = menuHelp->addAction("About");
   m_menubar->addAction(menuFile->menuAction());
   m_menubar->addAction(menuWindow->menuAction());
   m_menubar->addAction(menuHelp->menuAction());
   
   QAction* openImageAction = menuFile->addAction("Open Image");
   connect(openImageAction, SIGNAL(triggered(bool)),m_dataManagerWidget, SLOT(openLocalImage()));
//   connect(openImageAction, SIGNAL(triggered(bool)),this, SLOT(openImage()));

   QAction* openImageInteractiveAction = menuFile->addAction("Open Image Interactive");
   connect(openImageInteractiveAction, SIGNAL(triggered(bool)),
           m_dataManagerWidget, SLOT(openLocalImageInteractive()));
   
   QAction* openJpipAction = menuFile->addAction("Open JPIP");
   connect(openJpipAction, SIGNAL(triggered(bool)),this, SLOT(openJpip(bool)));
   menuFile->addSeparator();

   // SETUP FILE MENU
   QAction* saveProjectAction = menuFile->addAction("Save Project");
   QAction* saveProjectAsAction = menuFile->addAction("Save Project As");
   QAction* openProjectAction = menuFile->addAction("Open Project");
   connect(saveProjectAsAction, SIGNAL(triggered(bool)),this, SLOT(saveProjectAs(bool)));
   connect(saveProjectAction, SIGNAL(triggered(bool)),this, SLOT(saveProject(bool)));
   connect(openProjectAction, SIGNAL(triggered(bool)),this, SLOT(openProject(bool)));
   menuFile->addSeparator();


   QAction *exitAction = new QAction("Exit", menuFile);
   menuFile->addAction(exitAction);
   connect(exitAction, SIGNAL(triggered(bool)), this, SLOT(close()));


   //SETUP WINDOW MENU
   QAction* windowTabAction = menuWindow->addAction("Tab");
   connect(windowTabAction, SIGNAL(triggered(bool)),this, SLOT(tabWindows(bool)));
   
   QAction* windowCascadeAction = menuWindow->addAction("Cascade");
   connect(windowCascadeAction, SIGNAL(triggered(bool)),this, SLOT(cascadeWindows(bool)));
   QAction* windowTileAction = menuWindow->addAction("Tile");
   connect(windowTileAction, SIGNAL(triggered(bool)),this, SLOT(tileWindows(bool)));
   QAction* closeAll = menuWindow->addAction("Close All");
   connect(closeAll, SIGNAL(triggered(bool)),this, SLOT(closeAllWindows(bool)));
 
   connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(about(bool)));
   menuWindow->addAction("Minimize All");
   menuWindow->addAction("Restore All");
   menuWindow->addAction("Maximize");
   menuWindow->addAction("Minimize");
   
   setMenuBar(m_menubar);
   
   return m_menubar;
}

void ossimGui::MainWindow::showNode(DataManager::Node* node)
{
   if(node)
   {
      ossimRefPtr<DataManager::Node> n = node;
      // bool needToConnect = false;
      ConnectableDisplayObject* d =node->getObjectAs<ConnectableDisplayObject>();
      // check if we need to create a new display
      if(!d)
      {
         // needToConnect = true;
         n = m_dataManager->createDefault2dImageDisplay(node);
         d = n->getObjectAs<ConnectableDisplayObject>();
         if(d)
         {
            MdiSubWindowBase* child = d->display();
            child->setJobQueue(m_displayQueue.get());
            child->show();
         }
      }
      else 
      {
         MdiSubWindowBase* display = d->display();
         if(!display)
         {
            display = new ImageMdiSubWindow();
            display->setWindowTitle(n->name());
            display->setJobQueue(m_displayQueue.get());
            if(display->connectableObject() != d)
            {
               display->setConnectableObject(d);
            }
            m_mdiArea->addSubWindow(display);
            display->show();
         }
         else if(!display->parent()) 
         {
            display->setWindowTitle(n->name());
            display->setJobQueue(m_displayQueue.get());
            if(display->connectableObject() != d)
            {
               display->setConnectableObject(d);
            }
            m_mdiArea->addSubWindow(display);
            display->show();
         }
         else 
         {
            display->setJobQueue(m_displayQueue.get());
            display->show();
         }

      }      
   }     
}

void ossimGui::MainWindow::saveProject(bool /*checked*/)
{
   QString fileName = QFileDialog::getSaveFileName(this);
   if(fileName != "")
   {
      ossimFilename file = fileName.toAscii().data();
      file.setExtension("gcl");
      ossimKeywordlist kwl;
      m_dataManager->saveState(kwl, "dataManager.");
      kwl.write(file);
   }
}

void ossimGui::MainWindow::saveProjectAs(bool /*checked*/)
{
   QString fileName = QFileDialog::getSaveFileName(this);
   if(fileName != "")
   {
      ossimFilename file = fileName.toAscii().data();
      file.setExtension("gcl");
      ossimKeywordlist kwl;
      
      m_dataManager->saveState(kwl, "dataManager.");
      kwl.write(file);
   }
}

void ossimGui::MainWindow::openProject(bool /*checked*/)
{
   QString file = QFileDialog::getOpenFileName(this);
   if(file != "")
   {
      ossimFilename fileName(file.toAscii().data());
      if(!m_dataManagerWidget->openDataManager(fileName))
      {
         
      }
   }
}

void ossimGui::MainWindow::openImage( bool /*checked*/ )
{  
   QStringList fileNames = QFileDialog::getOpenFileNames(this);
   if (fileNames.size() > 0) 
   {
     // std::cout << "SIZE: " <<  << std::endl;
      for (int i = 0; i < fileNames.size(); ++i)
      {
         QUrl url(QUrl::fromLocalFile(fileNames.at(i)));
         OpenImageUrlJob* job = new OpenImageUrlJob(url);
         ossimImageOpenJobCallback* callback = new ossimImageOpenJobCallback();
         callback->m_object = this;
         job->setCallback(callback);
         m_stagerQueue->getJobQueue()->add(job);
      }
   }
}

void ossimGui::MainWindow::openJpip( bool /*checked*/ )
{
   bool ok;
   QString text = QInputDialog::getText(this, tr("JPIP stream of the form jpip://<url>/path"),
                                        tr("Url:"), QLineEdit::Normal,
                                        QDir::home().dirName(), &ok);
   if (ok && !text.isEmpty())
   {
      QUrl jpip(text);
      if(jpip.scheme() == "jpip")
      {
         OpenImageUrlJob* job = new OpenImageUrlJob(jpip);
         ossimImageOpenJobCallback* callback = new ossimImageOpenJobCallback();
         callback->m_object = this;
         job->setCallback(callback);
         m_stagerQueue->getJobQueue()->add(job);
      }
   }
}

void ossimGui::MainWindow::cascadeWindows( bool /*checked*/ )
{
   if(m_mdiArea)
   {
      m_mdiArea->setViewMode(QMdiArea::SubWindowView);
      
      m_mdiArea->cascadeSubWindows();
  }
}
void ossimGui::MainWindow::tileWindows( bool /*checked*/ )
{
   if(m_mdiArea)
   {
      m_mdiArea->setViewMode(QMdiArea::SubWindowView);
      m_mdiArea->tileSubWindows();
   }
}

void ossimGui::MainWindow::tabWindows(bool /* checked */)
{
   m_mdiArea->setViewMode(QMdiArea::TabbedView);
}

void ossimGui::MainWindow::closeAllWindows( bool /*checked*/ )
{
   QList<QMdiSubWindow *> subList = m_mdiArea->subWindowList();
   QList<QMdiSubWindow*>::iterator iter = subList.begin();
   while(iter != subList.end())
   {
      (*iter)->close();
      ++iter;
   }
}

bool ossimGui::MainWindow::loadProjectFile(const ossimString& projFile)
{
   bool loadOK = true;
   ossimFilename fileName(projFile);
   if(!m_dataManagerWidget->openDataManager(fileName))
   {
      loadOK = false;
   }
   return loadOK;
}

bool ossimGui::MainWindow::loadImageFileList(std::vector<ossimString>& ilist)
{  
   bool loadOK = true;
   QStringList fileNames;
   for (ossim_uint32 j = 0; j < ilist.size(); ++j)
   {
      fileNames.append(ilist[j].c_str());
   }

   if (fileNames.size() > 0) 
   {
      for (int i = 0; i < fileNames.size(); ++i)
      {
         QUrl url(QUrl::fromLocalFile(fileNames.at(i)));
         OpenImageUrlJob* job = new OpenImageUrlJob(url);
         ossimImageOpenJobCallback* callback = new ossimImageOpenJobCallback();
         callback->m_object = this;
         job->setCallback(callback);
         m_stagerQueue->getJobQueue()->add(job);
      }
   }
   return loadOK;
}

void ossimGui::MainWindow::createModeSelector(QToolBar* bar)
{
   m_exploitationOptions = bar->findChild<QComboBox*>("exploitationOptions");
   if(!m_exploitationOptions)
   {
      m_exploitationOptions = new QComboBox();
      m_exploitationOptions->setObjectName("exploitationOptions");
      m_exploitationOptions->addItem("<Select Exploitation Mode>");
      m_exploitationOptions->addItem("Registration");
      m_exploitationOptions->addItem("Geopositioning");
      m_exploitationOptions->addItem("Mensuration");
      bar->addWidget(m_exploitationOptions);
   }
   else 
   {
      m_exploitationOptions->setEnabled(true);
   }

   connect(m_exploitationOptions, SIGNAL(activated(int)), this, SLOT(exploitationModeChanged(int)));
   connect(m_dataManagerWidget, SIGNAL(resetMode()), this, SLOT(resetExploitationMode()));
}

void ossimGui::MainWindow::exploitationModeChanged(int idx)
{
   m_dataManager->setExploitationMode(idx);
   m_dataManagerWidget->miDialog(idx);
}

void ossimGui::MainWindow::resetExploitationMode()
{
   m_exploitationOptions->setCurrentIndex(0);
}

void ossimGui::MainWindow::about(bool)
{
   About* window = new About(this);
   
   QString version(OSSIM_VERSION);
   QString buildDate(OSSIM_BUILD_DATE);
   
   window->m_aboutText->setText(QString("OSSIM GeoCell\nVersion: " + version + " Date: " + buildDate));  
   window->exec();
}
