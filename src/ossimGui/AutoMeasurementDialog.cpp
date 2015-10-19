#include <ossimGui/MainWindow.h>
#include <ossimGui/AutoMeasurementDialog.h>
#include <ossimGui/RegistrationOverlay.h>
#include <ossimGui/MetricOverlay.h>
#include <ossimGui/RegPoint.h>
#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/Event.h>
#include <ossimGui/Util.h>
#include <QtGui/QMenu>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QComboBox>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>


ossimGui::AutoMeasurementDialog::AutoMeasurementDialog(
   QWidget* parent,
   DataManager::NodeListType& nodeList,
   ossimTieMeasurementGeneratorInterface* tGen)
: QDialog(parent),
  m_tGen(tGen)
{
   initDialog();

   // Save node list
   m_nodeList = nodeList;

   initContent();
}


void ossimGui::AutoMeasurementDialog::initDialog()
{
   setupUi(this);

   // Detector selection
   m_detComboBox->addItem("ORB");
   m_detComboBox->addItem("BRISK");
   m_detComboBox->addItem("FAST");
   m_detComboBox->addItem("STAR");
   m_detComboBox->addItem("GFTT");
   m_detComboBox->addItem("MSER");

   // Descriptor-Extractor selection
   m_extComboBox->addItem("FREAK");
   m_extComboBox->addItem("ORB");
   m_extComboBox->addItem("BRIEF");
   m_extComboBox->addItem("BRISK");

   // Detector selection (uchar-based)
   m_matComboBox->addItem("BruteForce-Hamming");
   m_matComboBox->addItem("BruteForce-HammingLUT");
   m_matComboBox->addItem("FlannBased");

   // Set primary connections
   connect(m_execMeasButton,   SIGNAL(clicked()), this, SLOT(execMeas()));
   connect(m_acceptMeasButton, SIGNAL(clicked()), this, SLOT(acceptMeas()));
   connect(m_resetMeasButton,  SIGNAL(clicked()), this, SLOT(resetMeas()));
   connect(m_dismissButton,    SIGNAL(clicked()), this, SLOT(dismissMeas()));

   connect(m_detComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectDetector(QString)));
   connect(m_extComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectExtractor(QString)));
   connect(m_matComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(selectMatcher(QString)));

   connect(m_xGridSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setGridSizeX(int)));
   connect(m_yGridSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setGridSizeY(int)));

   connect(m_maxMatchSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaxMatches(int)));
   m_maxMatchSpinBox->setValue(5);

   connect(m_cbUseGrid, SIGNAL(toggled(bool)), this, SLOT(setUseGridChecked(bool)));
}

void ossimGui::AutoMeasurementDialog::displayClosing(QObject* obj)
{
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

void ossimGui::AutoMeasurementDialog::initContent()
{
   // Set up ROI selector connections
   // ossim_uint32 idxLayer = 0;
   ConnectableDisplayObject* displayObj;
   ImageMdiSubWindow* subWindow;
   std::vector<ossimImageSource*> src;
   DataManager::NodeListType::iterator iter = m_nodeList.begin();

   while(iter != m_nodeList.end())
   {
      displayObj = (*iter)->getObjectAs<ConnectableDisplayObject>();
      subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      m_scrollViews.push_back(subWindow->scrollWidget());
      connect(m_scrollViews.back(), SIGNAL(mouseBox(ImageScrollView*, const ossimDpt&, const ossimDpt&)),
              this, SLOT(setBox(ImageScrollView*, const ossimDpt&, const ossimDpt&)));
      connect(subWindow, SIGNAL(destroyed(QObject*)), this, SLOT(displayClosing(QObject*)));
      ++iter;
   }

   // Initialize the measurement generator
   m_tGen->init(m_report);

   // Default to show the OpenCV match window
   // TODO: Checkbox should be added to the GUI
   m_tGen->setShowCvWindow(true);

   updateCurrentAlgorithmFields();

   m_acceptMeasButton->setEnabled(false);
   m_execMeasButton->setEnabled(true);

   // Disable grid because check box is unchecked
   m_xGridSpinBox->setEnabled(false);
   m_yGridSpinBox->setEnabled(false);
}


// Initiate auto measurement
void ossimGui::AutoMeasurementDialog::execMeas()
{
   if (m_tGen->isValidCollectionBox())
   {
      m_acceptMeasButton->setEnabled(true);
      m_execMeasButton->setEnabled(false);

      // Run the OpenCV detector/extractor/matcher
      m_tGen->run();

      setMeasurementReportContent(m_report.str());
   
//==== TODO hardcopy output
// ofstream myfile;
// myfile.open ("/Users/dhicks/work/reg.txt");
// myfile << m_report.str() << std::flush;
// myfile.close();
//==== TODO hardcopy output

   }
   else
   {
      ossimString noBox("Collection box not defined....");
      setMeasurementReportContent(noBox);
   }
}

// Accept auto measurement
void ossimGui::AutoMeasurementDialog::acceptMeas()
{
   // Close openCV window
   m_tGen->closeCvWindow();

   // Signal to DataManagerWidget
   emit acceptMeasExecuted(m_nodeList);
}

// Reset auto measurement
void ossimGui::AutoMeasurementDialog::resetMeas()
{

   m_scrollViews.clear();
   DataManager::NodeListType::iterator iter = m_nodeList.begin();
   while(iter != m_nodeList.end())
   {
      ConnectableDisplayObject* displayObj = (*iter)->getObjectAs<ConnectableDisplayObject>();
      ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
      if(subWindow)
      {
         ossimGui::ImageScrollView* scrollWidget = subWindow->scrollWidget();
         disconnect(scrollWidget, SIGNAL(mouseBox(ImageScrollView*, const ossimDpt&, const ossimDpt&)),
                 this, SLOT(setBox(ImageScrollView*, const ossimDpt&, const ossimDpt&)));
         disconnect(subWindow, SIGNAL(destroyed(QObject*)), this, SLOT(displayClosing(QObject*)));
      }
      ++iter;
   }

   initContent();
   m_measResultsBrowser->setText("Current measurements cleared");

   // Close openCV window
   m_tGen->closeCvWindow();
}

// Dismiss auto measurement
void ossimGui::AutoMeasurementDialog::dismissMeas()
{
   // Signal to DataManagerWidget
   emit dismissMeasExecuted();
   
   // Close openCV window
   m_tGen->closeCvWindow();
}

// Select detector combobox
void ossimGui::AutoMeasurementDialog::selectDetector(QString str)
{
   ossimString det = str.toStdString();
   if (m_tGen->setFeatureDetector(det))
      m_detName->setText(det.data());
   else
      m_detName->setText("...");
}

// Select extractor combobox
void ossimGui::AutoMeasurementDialog::selectExtractor(QString str)
{
   ossimString ext = str.toStdString();
   if (m_tGen->setDescriptorExtractor(ext))
      m_extName->setText(ext.data());
   else
      m_extName->setText("...");
}

// Select matcher combobox
void ossimGui::AutoMeasurementDialog::selectMatcher(QString str)
{
   ossimString mat = str.toStdString();
   if (m_tGen->setDescriptorMatcher(mat))
      m_matName->setText(mat.data());
   else
      m_matName->setText("...");
}

// Set collection bounding box
void ossimGui::AutoMeasurementDialog::setBox(
   ImageScrollView* sptr, const ossimDpt& sp, const ossimDpt& ep)
{

   int primeIndex = -1;
   int numOfScrollViews = m_scrollViews.size();
   ossimGpt centerG;
   centerG.makeNan();
   ossimGpt cornG[4];
   ossimRefPtr<IvtGeomTransform> ivtg;

   // TODO not currently used
   bool roiOK = true;

   // Load ossimImageSource vector from the node list
   std::vector<ossimImageSource*> src;
   ossim_uint32 idxLayer = 0;
   for (int i=0; i<numOfScrollViews; ++i)
   {
      src.push_back(m_scrollViews[i]->layers()->layer(idxLayer)->chain());
   }
   m_roiRects.resize(numOfScrollViews);

   // Define ROI in primary image
   for (int i=0; i<numOfScrollViews; ++i)
   {
      if (sptr == m_scrollViews[i])
      {
         primeIndex = i;

         // Set to full resolution before defining ROI
         ImageViewManipulator* ivm = m_scrollViews[i]->manipulator();
         if (ivm)
         {
            ivm->fullRes();
         }
         else
         {
            roiOK = false;
         }

         // Box size (sp,ep image)
         int dxI = abs(ep.x - sp.x);
         int dyI = abs(ep.y - sp.y);
         ossimIpt patchDimI(dxI, dyI);

         // Box center (image)
         ossimDpt centerI;
         centerI.x = sp.x + dxI/2;
         centerI.y = sp.y + dyI/2;
         ossimIrect rectA(centerI, dxI, dyI);

         // Center on patch (view)
         ossimDpt centerV;
         ossimGui::GatherImageViewProjTransVisitor visitor;
         src[i]->accept(visitor);
         if (visitor.getTransformList().size() == 1)
         {
            ivtg = visitor.getTransformList()[0].get();
            ivtg->imageToView(centerI, centerV);
            ivtg->imageToGround(centerI, centerG);
            if (centerG.isHgtNan())
            {
               ossim_float64 hgt = ossimElevManager::instance()->getHeightAboveEllipsoid(centerG);
               centerG.height(hgt);
            }
         }
         QPointF pt(centerV.x,centerV.y);
         m_scrollViews[i]->centerOn(pt);

         // Get patch corners in ground space
         ossimDpt primPtsI[4];
         ossimDpt primPtsV[4];
         primPtsI[0] = rectA.ul();
         primPtsI[1] = rectA.ur();
         primPtsI[2] = rectA.lr();
         primPtsI[3] = rectA.ll();
         for (int k=0; k<4; ++k)
         {
            ivtg->imageToGround(primPtsI[k], cornG[k]);
            if (cornG[k].isHgtNan())
            {
               ossim_float64 hgt = ossimElevManager::instance()->getHeightAboveEllipsoid(cornG[k]);
               cornG[k].height(hgt);
            }
            ivtg->imageToView(primPtsI[k], primPtsV[k]);
         }

         // Load view coordinate rectangle for primary image
         ossimIrect rectAv(primPtsV[0], primPtsV[1], primPtsV[2], primPtsV[3]);
         m_roiRects[primeIndex] = rectAv;

         // We could get patch to insert into imageSource vector....
         //  Note: currently don't have to do this, the later 
         //        ossimTieMeasurementGenerator getTile will pull the patch
         //        out of the whole source
         // ossimRefPtr<ossimImageData> idA = src[i]->getTile(rectAv);
         // cout<<"idA size = "<<idA->getWidth()<<"X"<<idA->getHeight()<<endl;
         // Load sources (TODO how ossimImageData->ossimImageSource?)
      }
   }

   // Define ROI in remaining images
   for (int i=0; i<numOfScrollViews; ++i)
   {
      if (sptr != m_scrollViews[i])
      {

         // Set to full resolution before defining ROI
         ImageViewManipulator* ivm = m_scrollViews[i]->manipulator();
         if (ivm)
         {
            ivm->fullRes();
         }
         else
         {
            roiOK = false;
         }

         // Center on patch (view)
         ossimDpt centerI;
         ossimDpt centerV;
         ossimGui::GatherImageViewProjTransVisitor visitor;
         src[i]->accept(visitor);
         if (visitor.getTransformList().size() == 1)
         {
            ivtg = visitor.getTransformList()[0].get();
            ivtg->groundToImage(centerG, centerI);
            ivtg->imageToView(centerI, centerV);
         }
         QPointF pt(centerV.x,centerV.y);
         m_scrollViews[i]->centerOn(pt);

         // Define this patch by projection from primary patch
         ossimDpt xferPtsI[4];
         ossimDpt xferPtsV[4];
         for (int k=0; k<4; ++k)
         {
            ivtg->groundToImage(cornG[k], xferPtsI[k]);
            ivtg->imageToView(xferPtsI[k], xferPtsV[k]);
         }

         // Load view coordinate rectangle for this image
         ossimIrect rectXfer(xferPtsV[0],xferPtsV[1],xferPtsV[2],xferPtsV[3]);
         m_roiRects[i] = rectXfer;
      }
   }


   m_tGen->setBox(m_roiRects, primeIndex, src);

   // return roiOK;
}

// Set grid parameters
void ossimGui::AutoMeasurementDialog::setUseGridChecked(bool isChecked)
{
   m_tGen->setUseGrid(isChecked);

   m_xGridSpinBox->setEnabled(isChecked);
   m_yGridSpinBox->setEnabled(isChecked);
}
void ossimGui::AutoMeasurementDialog::setGridSizeX(int xDim)
{
   ossimIpt gs = m_tGen->getGridSize();
   gs.x = xDim;
   m_tGen->setGridSize(gs);
}
void ossimGui::AutoMeasurementDialog::setGridSizeY(int yDim)
{
   ossimIpt gs = m_tGen->getGridSize();
   gs.y = yDim;
   m_tGen->setGridSize(gs);
}

void ossimGui::AutoMeasurementDialog::setMaxMatches(int maxMatches)
{
   m_tGen->setMaxMatches(maxMatches);
}

// Load registration text browser
void ossimGui::AutoMeasurementDialog::setMeasurementReportContent(const ossimString& report)
{
   QFont f( "courier", 12 );
   m_measResultsBrowser->setFont(f);
   m_measResultsBrowser->setLineWrapMode(QTextEdit::NoWrap);
   m_measResultsBrowser->setText(report.data());
}

void ossimGui::AutoMeasurementDialog::updateCurrentAlgorithmFields()
{
   m_detName->setText(m_tGen->getFeatureDetector().data());
   m_extName->setText(m_tGen->getDescriptorExtractor().data());
   m_matName->setText(m_tGen->getDescriptorMatcher().data());
}
