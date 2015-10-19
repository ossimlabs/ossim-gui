#include <ossimGui/MainWindow.h>
#include <ossimGui/MultiImageDialog.h>
#include <ossimGui/RegistrationOverlay.h>
#include <ossimGui/MetricOverlay.h>
#include <ossimGui/RegPoint.h>
#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/ImageScrollView.h>
#include <QtGui/QMenu>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <ossimGui/Event.h>
#include <ossimGui/Util.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/imaging/ossimImageHandler.h>


ossimGui::MultiImageDialog::MultiImageDialog(QWidget* parent)
: QDialog(parent),
  m_isActive(false),
  m_currentIdCounter(0)
{
   initDialog();
   this->setAttribute(Qt::WA_DeleteOnClose);
}


void ossimGui::MultiImageDialog::initDialog()
{
   setupUi(this);

   // Set primary connections
   connect(m_resetModeButton, SIGNAL(clicked()), this, SLOT(resetContent()));
   connect(m_hideButton, SIGNAL(clicked()), this, SLOT(resetContent()));
   connect(m_addPointButton, SIGNAL(clicked()), this, SLOT(addObsPoint()));
   connect(m_autoMeasButton, SIGNAL(clicked()), this, SLOT(autoMeas()));
   connect(m_registerButton, SIGNAL(clicked()), this, SLOT(registerImages()));
   connect(m_dropButton, SIGNAL(clicked()), this, SLOT(dropPoint()));
   connect(m_clearPointButton, SIGNAL(clicked()), this, SLOT(clearPoint()));
   connect(m_acceptRegButton, SIGNAL(clicked()), this, SLOT(acceptReg()));
   connect(m_resetRegButton, SIGNAL(clicked()), this, SLOT(resetReg()));

   // Point table
   connect(m_pointTable, SIGNAL(cellClicked(int, int)), this, SLOT(setPointCellClicked(int, int)));
   connect(m_pointTable->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(setPointRowClicked(int)));
   connect(m_pointTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(setPointColClicked(int)));
   m_pointTable->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_pointTable->verticalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayPointTableContextMenuRow(QPoint)));
   m_pointTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_pointTable->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayPointTableContextMenuCol(QPoint)));

   // Turn off row/col resizing
   m_pointTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);
   m_pointTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
   m_imageTable->verticalHeader()->setResizeMode(QHeaderView::Fixed);

   // Turn off selection
   m_pointTable->setSelectionMode(QAbstractItemView::NoSelection); 
   m_imageTable->setSelectionMode(QAbstractItemView::NoSelection);

   // Image table right-click context menu
   m_imageTable->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(m_imageTable->verticalHeader(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(displayImageTableContextMenu(QPoint)));

   // Load tooltips
   m_currentPointID->setToolTip("Active point ID");
   m_imageTable->verticalHeader()->setToolTip("Right click for context menu");
   m_pointTable->verticalHeader()->setToolTip("Right click for context menu");
   m_pointTable->horizontalHeader()->setToolTip("Left click to sync on point\nRight click for context menu");
   m_hideButton->setToolTip("Hide window (press 's' key to show)");
   m_resetModeButton->setToolTip("Reset exploitation mode");
}
void ossimGui::MultiImageDialog::displayClosing(QObject* obj)
{
   ImageMdiSubWindow* subWindow = (ImageMdiSubWindow*)(obj);
   if(subWindow)
   {

      RegistrationOverlay* regOverlay = subWindow->scrollWidget()->regOverlay();
      MetricOverlay* metOverlay = subWindow->scrollWidget()->metOverlay();
      std::vector<ossimGui::MetricOverlay*>::iterator metIter = std::find(m_overlaysMet.begin(), 
                                                                                    m_overlaysMet.end(), 
                                                                                    metOverlay);
      if(metIter != m_overlaysMet.end())
      {
         m_overlaysMet.erase(metIter);
      }
       std::vector<ossimGui::RegistrationOverlay*>::iterator regIter = std::find(m_overlaysReg.begin(), 
                                                                           m_overlaysReg.end(), 
                                                                           regOverlay);
      if(regIter != m_overlaysReg.end())
      {
        m_overlaysReg.erase(regIter);
      }
      DataManager::NodeListType::iterator iter = m_nodeList.begin();
      while(iter != m_nodeList.end())
      {
         ConnectableDisplayObject* displayObj = (*iter)->getObjectAs<ConnectableDisplayObject>();
         ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
         if((subWindow == obj) || (subWindow == 0))
         {
            iter = m_nodeList.erase(iter);
         }
         else
         {
            ++iter;
         }
      }
   }

}

void ossimGui::MultiImageDialog::initContent(DataManager::NodeListType& nodeList,
                                             const bool& amDialogAvailable)
{

   // Load overlays & set image point connections
   for(ossim_uint32 idx = 0; idx < nodeList.size(); ++idx)
   {
      ConnectableDisplayObject* displayObj = nodeList[idx]->getObjectAs<ConnectableDisplayObject>();
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      RegistrationOverlay* regOverlay = subWindow->scrollWidget()->regOverlay();
      m_overlaysReg.push_back(regOverlay);
      MetricOverlay* metOverlay = subWindow->scrollWidget()->metOverlay();
      m_overlaysMet.push_back(metOverlay);
      connect(m_overlaysReg[idx], SIGNAL(pointActivated(const ossimString&)), this, SLOT(setImagePointActive(const ossimString&)));
      connect(m_overlaysReg[idx], SIGNAL(pointDeactivated(const ossimString&)), this, SLOT(setImagePointInactive(const ossimString&)));
      connect(m_overlaysReg[idx], SIGNAL(pointRemoved(const ossimString&)), this, SLOT(setImagePointRemoved(const ossimString&)));
      
      // if a display closes make sure we clean up
      connect(subWindow, SIGNAL(destroyed(QObject*)), this, SLOT(displayClosing(QObject*)));
   }

   // Activate registration overlays
   for(ossim_uint32 idx = 0; idx < m_overlaysReg.size(); ++idx)
   {
      m_overlaysReg[idx]->setActive(true);
   }

   // Load image list
   std::vector<ossimString> ilist;
   std::vector<ossimString> tlist;
   for(ossim_uint32 idx = 0; idx < m_overlaysReg.size(); ++idx)
   {
      ossimConnectableObject* connectable = nodeList[idx]->getObjectAs<ossimConnectableObject>();
      ossimTypeNameVisitor visitor("ossimImageHandler");
      connectable->accept(visitor);
      ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
      
      if(input.valid())
      {
         ossimRefPtr<ossimImageGeometry> geom = input->getImageGeometry();
         if(geom.valid())
         {
            ossimRefPtr<ossimProjection> proj = geom->getProjection();
            if(proj.valid())
            {
               tlist.push_back(proj->getShortName());
            }
            else
            {
               tlist.push_back("UNKNOWN");
            }
         }
         ilist.push_back(input->getFilename());

         // Check for control image based on presence of AdjustableParameterInterface
         ossimAdjustableParameterInterface* iface =
            input->getImageGeometry()->getAdjustableParameterInterface();
         if (iface != NULL)
         {
            m_overlaysReg[idx]->setHasAdjParInterface(true);
         }
         else
         {
            m_overlaysReg[idx]->setAsControl(true);
         }
      }
   }

   // Populate dialog
   setImgList(ilist, tlist);
   setPtTable(m_currentIdCounter);

   // Save node list
   m_nodeList = nodeList;

   // If auto measurement is available, activate the button
   if (amDialogAvailable)
      m_autoMeasButton->setEnabled(true);
   else
      m_autoMeasButton->setEnabled(false);


   m_isActive = true;
}


void ossimGui::MultiImageDialog::resetContent()
{
   // Signal to DataManagerWidget
   emit resetModeExecuted(m_nodeList);
   //std::cout << "ossimGui::MultiImageDialog::resetContent(): ...........After emit!!!\n";

   for(ossim_uint32 idx = 0; idx < m_overlaysReg.size(); ++idx)
   {
      disconnect(m_overlaysReg[idx], SIGNAL(pointActivated(const ossimString&)), this, SLOT(setImagePointActive(const ossimString&)));
      disconnect(m_overlaysReg[idx], SIGNAL(pointDeactivated(const ossimString&)), this, SLOT(setImagePointInactive(const ossimString&)));
      disconnect(m_overlaysReg[idx], SIGNAL(pointRemoved(const ossimString&)), this, SLOT(setImagePointRemoved(const ossimString&)));
      m_overlaysReg[idx]->reset();
   }
   m_overlaysReg.clear();
   m_overlaysMet.clear();

   m_currentIdCounter = 0;
   setPtTable(m_currentIdCounter);

   m_regResultsBrowser->setText("Mode Reset");
   m_pointPositionBrowser->setText("Mode Reset");

   m_nodeList.clear();

   updateCurrentIdField();

   m_isActive = false;
   close();
}


void ossimGui::MultiImageDialog::setMode(const int& expMode)
{
   m_exploitationMode = static_cast<DataManager::ExploitationModeType> (expMode);

   if (m_exploitationMode == DataManager::REGISTRATION_MODE)
   {
      m_tabWidget->setTabEnabled(4, false); //m_mensurationTab
      m_tabWidget->setTabEnabled(3, false); //m_pointPositionTab
      m_tabWidget->setTabEnabled(2, true);  //m_RegistrationTab
      m_tabWidget->setTabEnabled(1, true);  //m_pointEditorTab
      m_acceptRegButton->setEnabled(false);
   }
   else if (m_exploitationMode == DataManager::GEOPOSITIONING_MODE)
   {
      m_tabWidget->setTabEnabled(4, false); //m_mensurationTab
      m_tabWidget->setTabEnabled(3, true);  //m_pointPositionTab
      m_tabWidget->setTabEnabled(2, false); //m_RegistrationTab
      m_tabWidget->setTabEnabled(1, false); //m_pointEditorTab
   }
   else if (m_exploitationMode == DataManager::MENSURATION_MODE)
   {
      m_tabWidget->setTabEnabled(4, true);  //m_mensurationTab
      m_tabWidget->setTabEnabled(3, false); //m_pointPositionTab
      m_tabWidget->setTabEnabled(2, false); //m_RegistrationTab
      m_tabWidget->setTabEnabled(1, false); //m_pointEditorTab
   }
   else
   {
      resetContent();
   }
}


void ossimGui::MultiImageDialog::registerImages()
{
   // Signal to DataManagerWidget
   emit registrationExecuted(m_nodeList);
   m_acceptRegButton->setEnabled(true);
}

void ossimGui::MultiImageDialog::dropPoint()
{
   // Signal to DataManagerWidget
   emit pointDropExecuted(m_nodeList);
}

void ossimGui::MultiImageDialog::clearPoint()
{
   // Signal to DataManagerWidget
   emit clearPointExecuted(m_nodeList);
   m_pointPositionBrowser->setText("Current measurements cleared");
}

void ossimGui::MultiImageDialog::acceptReg()
{
   // Signal to DataManagerWidget
   emit acceptRegExecuted(m_nodeList);
   m_acceptRegButton->setEnabled(false);
}

void ossimGui::MultiImageDialog::resetReg()
{
   QMessageBox confBox;
   confBox.setText("Confirm solution clear...");
   confBox.setWindowFlags(Qt::WindowStaysOnTopHint);
   confBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
   confBox.setDefaultButton(QMessageBox::Cancel);
   int ret = confBox.exec();
   switch (ret) {
      case QMessageBox::Cancel:
         break;
      case QMessageBox::Ok:
         // Signal to DataManagerWidget
         emit resetRegExecuted(m_nodeList);
         m_acceptRegButton->setEnabled(false);
         m_regResultsBrowser->setText("Registration solution cleared...");
         break;
      default:
         break;
   }
}

// Initiate auto measurement
void ossimGui::MultiImageDialog::autoMeas()
{
   // Signal to DataManagerWidget
   emit autoMeasInitiated(m_nodeList);
}

// Add new point observation slot (column)
void ossimGui::MultiImageDialog::addObsPoint()
{
   int col = m_pointTable->columnCount();
   m_pointTable->insertColumn(col);

   m_currentIdCounter = col + 1;

   ossimString hdr = ossimString::toString(m_currentIdCounter);
   QTableWidgetItem *tblCol = new QTableWidgetItem(hdr.data());
   tblCol->setBackground(Qt::white);
   m_pointTable->setHorizontalHeaderItem(col, tblCol);

   for(ossim_int32 row = 0; row<m_pointTable->rowCount(); ++row)
   {
      QTableWidgetItem *cellItem = new QTableWidgetItem();
      cellItem->setBackgroundColor(Qt::lightGray);
      cellItem->setToolTip("Left click to toggle active/inactive");
      m_pointTable->setItem(row, col, cellItem);
   }
   m_pointTable->resizeColumnsToContents();

   for (ossim_uint32 i=0; i<m_overlaysReg.size(); ++i)
   {
      m_overlaysReg[i]->setCurrentId(getCurrentId());
   }

   updateCurrentIdField();
}

// Clicked row
void ossimGui::MultiImageDialog::setPointRowClicked(int /* row */)
{
}

// Clicked column syncs on selected point
void ossimGui::MultiImageDialog::setPointColClicked(int col)
{
   m_currentIdCounter = col + 1;
   ossimString id = getCurrentId();

   bool synced = false;

   for (ossim_uint32 i=0; i<m_overlaysReg.size(); ++i)
   {
      m_overlaysReg[i]->setCurrentId(id);

      ossimDpt imgPt;
      bool isActive;
      if (m_overlaysReg[i]->getImgPoint(id, imgPt, isActive))
      {
         if (!synced && isActive)
         {
            ossimGui::RegPoint* syncPt = m_overlaysReg[i]->getRegPoint(id);

            // Signal to DataManagerWidget
            emit syncExecuted(syncPt, m_nodeList[i]);
            synced = true;
         }
      }
   }

   updateCurrentIdField();
}

// Clicked cell
void ossimGui::MultiImageDialog::setPointCellClicked(int row, int col)
{
   QTableWidgetItem *cellItem = new QTableWidgetItem();
   cellItem->setBackgroundColor(Qt::red);
   m_pointTable->setItem(row, col, cellItem);

   ossimString id = getIdByIndex(col);
   m_overlaysReg[row]->togglePointActive(id);
}

// Activate measured image point
void ossimGui::MultiImageDialog::setImagePointActive(const ossimString& id)
{
   ossimGui::RegistrationOverlay* ov = (ossimGui::RegistrationOverlay*)sender();

   ossim_uint32 row;
   ossim_uint32 col;
   if (getRowColMeasPoint(id, ov, row, col))
   {
      QTableWidgetItem *cellItem = new QTableWidgetItem();
      cellItem->setBackgroundColor(Qt::yellow);
      m_pointTable->setItem(row, col, cellItem);
   }
}

// Deactivate measured image point
void ossimGui::MultiImageDialog::setImagePointInactive(const ossimString& id)
{
   ossimGui::RegistrationOverlay* ov = (ossimGui::RegistrationOverlay*)sender();

   ossim_uint32 row;
   ossim_uint32 col;
   if (getRowColMeasPoint(id, ov, row, col))
   {
      QTableWidgetItem *cellItem = new QTableWidgetItem();
      cellItem->setBackgroundColor(Qt::red);
      m_pointTable->setItem(row, col, cellItem);
   }
}

// Remove measured image point
void ossimGui::MultiImageDialog::setImagePointRemoved(const ossimString& id)
{
   ossimGui::RegistrationOverlay* ov = (ossimGui::RegistrationOverlay*)sender();

   ossim_uint32 row;
   ossim_uint32 col;
   if (getRowColMeasPoint(id, ov, row, col))
   {
      QTableWidgetItem *cellItem = new QTableWidgetItem();
      cellItem->setBackgroundColor(Qt::lightGray);
      m_pointTable->setItem(row, col, cellItem);
   }
}

// Load registration text browser
void ossimGui::MultiImageDialog::setRegistrationReportContent(const ossimString& report)
{
   QFont f( "courier", 12 );
   m_regResultsBrowser->setFont(f);
   m_regResultsBrowser->setLineWrapMode(QTextEdit::NoWrap);
   m_regResultsBrowser->setText(report.data());
}

// Load point drop results
void ossimGui::MultiImageDialog::setPointPositionContent(const ossimString& report)
{
   QFont f( "courier", 12 );
   m_pointPositionBrowser->setFont(f);
   m_pointPositionBrowser->setLineWrapMode(QTextEdit::NoWrap);
   m_pointPositionBrowser->setText(report.data());
}

void ossimGui::MultiImageDialog::setImgList(const vector<ossimString>& ilist,
                                            const vector<ossimString>& tlist)
{
   ossim_uint32 nImgs = ilist.size();

   m_imageTable->setRowCount(nImgs);
   m_imageTable->setColumnCount(2);

   m_imageTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Image Source"));
   m_imageTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));

   m_imageTable->setColumnWidth( 0, m_imageTable->width()-210 );
   m_imageTable->setColumnWidth( 1, 200 );

   QStringList labels;
   for(ossim_uint32 row = 0; row<nImgs; ++row)
   {
      QTableWidgetItem* cellItemImg = new QTableWidgetItem();
      cellItemImg->setText(ilist[row].data());
      m_imageTable->setItem(row, 0, cellItemImg);

      QTableWidgetItem* cellItemTyp = new QTableWidgetItem();
      cellItemTyp->setText(tlist[row].data());
      m_imageTable->setItem(row, 1, cellItemTyp);

      ossimString hdr = ossimString::toString(row+1);
      if (m_overlaysReg[row]->isControlImage())
      {
         hdr += "C";
      }
      labels << hdr.data();
   }

   m_imageTable->setVerticalHeaderLabels(labels);
}

void ossimGui::MultiImageDialog::setPtTable(const int& nPts)
{
   ossim_int32 nImgs = m_imageTable->rowCount();

   m_pointTable->setRowCount(nImgs);
   m_pointTable->setColumnCount(nPts);

   QStringList labels;
   for(ossim_int32 row = 0; row<nImgs; ++row)
   {
      for(ossim_int32 col = 0; col<nPts; ++col)
      {
         QTableWidgetItem *item = new QTableWidgetItem();

         // Load cell
         m_pointTable->setItem(row, col, item);

      }
      // Check for control indicator
      ossimString hdr = ossimString::toString(row+1);
      if (m_overlaysReg[row]->isControlImage())
      {
         hdr += "C";
      }
      labels << hdr.data();
   }
   m_pointTable->resizeColumnsToContents();
   m_pointTable->setVerticalHeaderLabels(labels);

   // Set tab 0 to front
   m_tabWidget->setCurrentWidget(m_imageSummaryTab);
 }


// Determine row,col of image measurement point
bool ossimGui::MultiImageDialog::getRowColMeasPoint(const ossimString& id,
                                                    ossimGui::RegistrationOverlay* ov,
                                                    ossim_uint32& row,
                                                    ossim_uint32& col)
{
   bool ptFound = false;
   bool overlayFound = false;

   for (ossim_uint32 idx=0; idx<m_overlaysReg.size(); ++idx)
   {
      if (ov == m_overlaysReg[idx])
      {
         row = idx;
         overlayFound = true;
      }
   }

   if (overlayFound)
   {
      for (int idx=0; idx<m_pointTable->columnCount(); ++idx)
      {
         if (id == getIdByIndex(idx))
         {
            col = idx;
            ptFound = true;
         }
      }
   }

   return ptFound;
}

void ossimGui::MultiImageDialog::displayPointTableContextMenuRow(QPoint pos)
{
   QPoint posV = m_pointTable->verticalHeader()->viewport()->mapToGlobal(pos);

   ossim_uint32 ht = m_pointTable->rowHeight(0);
   ossim_int32 row = pos.y()/ht;

   QAction* toggleActive = 0;
   QAction* choice2      = 0;
   QAction* mexec        = 0;

   if (row < m_pointTable->rowCount())
   {
      QMenu menu(this);
      if (m_exploitationMode == DataManager::REGISTRATION_MODE)
      {
         toggleActive = menu.addAction("Toggle all points status");
         // choice2 = menu.addAction("C 2");
      }
      
      mexec = menu.exec(posV);

      if (toggleActive == mexec)
      {
         for (int col=0; col<m_pointTable->columnCount(); ++col)
         {
            setPointCellClicked(row, col);
         }
      }
      else if (choice2 == mexec)
      {
      }
   }
}

void ossimGui::MultiImageDialog::displayPointTableContextMenuCol(QPoint pos)
{
   QPoint posH = m_pointTable->horizontalHeader()->viewport()->mapToGlobal(pos);

   ossim_uint32 wid = m_pointTable->columnWidth(0);
   ossim_int32 col = pos.x()/wid;

   QAction* toggleActive = 0;
   QAction* choice2      = 0;
   QAction* mexec        = 0;

   if (col < m_pointTable->columnCount())
   {
      QMenu menu(this);
      if (m_exploitationMode == DataManager::REGISTRATION_MODE)
      {
         toggleActive = menu.addAction("Toggle point status");
         // choice2 = menu.addAction("C 2");
      }
      
      mexec = menu.exec(posH);

      if (toggleActive == mexec)
      {
         for (int row=0; row<m_pointTable->rowCount(); ++row)
         {
            setPointCellClicked(row, col);
         }
      }
      else if (choice2 == mexec)
      {
      }
   }
}

void ossimGui::MultiImageDialog::displayImageTableContextMenu(QPoint pos)
{
   QPoint posG = m_imageTable->verticalHeader()->viewport()->mapToGlobal(pos);
   ossim_uint32 ht = m_imageTable->rowHeight(0);
   ossim_int32 row = pos.y()/ht;

   QAction* toggleControl = 0;
   QAction* viewAdjPar    = 0;
   QAction* mexec         = 0;

   if (row < m_imageTable->rowCount())
   {
      QMenu menu(this);
      if (m_overlaysReg[row]->hasAdjParInterface())
      {
         if (m_exploitationMode == DataManager::REGISTRATION_MODE)
         {
            toggleControl = menu.addAction("Toggle control image");
         }
         viewAdjPar = menu.addAction("View adjustable parameters");
      }
      
      mexec = menu.exec(posG);

      if (toggleControl == mexec)
      {
         QStringList labels;

         for (ossim_int32 i=0; i<m_imageTable->rowCount(); ++i)
         {
            ossimString hdr = ossimString::toString(i+1);
            if (m_overlaysReg[i]->hasAdjParInterface())
            {
               if (i==row)
               {
                     bool isCurrentlyControl = m_overlaysReg[i]->isControlImage();
                     m_overlaysReg[i]->setAsControl(!isCurrentlyControl);
                     if (m_overlaysReg[i]->isControlImage())
                     {
                        hdr += "C";
                     }
               }
               else
               {
                     m_overlaysReg[i]->setAsControl(false);
               }
            }
            else
            {
               hdr += "C";
            }
            labels << hdr.data();
         }

         m_pointTable->setVerticalHeaderLabels(labels);
         m_imageTable->setVerticalHeaderLabels(labels);
      }
      else if (viewAdjPar == mexec)
      {
         ConnectableDisplayObject* displayObj = m_nodeList[row]->getObjectAs<ConnectableDisplayObject>();
         ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
         subWindow->activateWindow();
         subWindow->setFocus();
         subWindow->getImageActions()->editGeometryAdjustments();
      }
   }
}

ossimString ossimGui::MultiImageDialog::getIdByIndex(const ossim_uint32& index)
{
   ossimString id = m_pointTable->horizontalHeaderItem(index)->text().toStdString();
   return id;
}

void ossimGui::MultiImageDialog::updateCurrentIdField()
{
   ossimString id;

   if (m_currentIdCounter == 0)
   {
      id = "-";
   }
   else
   {
      id = ossimString::toString(m_currentIdCounter);
   }

   m_currentPointID->setText(id.data());
}
