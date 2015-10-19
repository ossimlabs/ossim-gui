#include <ossimGui/HistogramRemapperEditor.h>

#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimHistogramSource.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

class ossimHistogramVisitor : public ossimVisitor
{
public:
   ossimHistogramVisitor()
   :ossimVisitor(ossimVisitor::VISIT_INPUTS | ossimVisitor::VISIT_CHILDREN)
   {
   }
   ossimHistogramVisitor(const ossimHistogramVisitor& src)
   :ossimVisitor(src),
   m_histogram(src.m_histogram)
   {
   }
   virtual ossimRefPtr<ossimVisitor> dup()const{return new ossimHistogramVisitor(*this);}
   virtual void visit(ossimConnectableObject* obj)
   {
      if(!hasVisited(obj))
      {
         ossimImageHandler* handler        = dynamic_cast<ossimImageHandler*>(obj);
         ossimHistogramSource* histoSource = dynamic_cast<ossimHistogramSource*>(obj);
         
         if(handler)
         {
            ossimFilename file = handler->createDefaultHistogramFilename();
            if(file.exists())
            {
               m_histogram = new ossimMultiResLevelHistogram();
               if(!m_histogram->importHistogram(file))
               {
                  stopTraversal();
                  m_histogram = 0;
               }
            }
         }
         else if(histoSource)
         {
            m_histogram = histoSource->getHistogram();
            if(m_histogram.valid())
            {
               stopTraversal();
            }
         }
      }
      ossimVisitor::visit(obj);
   }
   
   ossimRefPtr<ossimMultiResLevelHistogram> m_histogram;
};


const char* stretchModes[] = {"None", "linear_auto_min_max", "linear_1std_from_mean", "linear_2std_from_mean", "linear_3std_from_mean", "linear_one_piece", 0};

ossimGui::HistogramRemapperEditor::HistogramRemapperEditor(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
   connect(m_bandComboBox, SIGNAL(activated(int)), this, SLOT(bandActivated(int)));
   connect(m_stretchModeComboBox, SIGNAL(activated(int)), this, SLOT(stretchModeActivated(int)));
   connect(m_histogramFilePushButton, SIGNAL(clicked(bool)), this, SLOT(openHistogramButton(bool)));
   connect(m_enableButton, SIGNAL(clicked(bool)), this, SLOT(enableButtonClicked(bool)));
   connect(m_resetButton, SIGNAL(clicked(bool)), this, SLOT(resetButtonClicked(bool)));
   connect(m_okButton, SIGNAL(clicked(bool)), this, SLOT(okButtonClicked(bool)));
   connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancelButtonClicked(bool)));
   connect(m_histogramWidget, SIGNAL(clipPenetrationsAdjusted(double, double)), SLOT(clipPenetrationsAdjusted(double, double)));
   m_histogramWidget->setShowClipPointsFlag(true);

}

void ossimGui::HistogramRemapperEditor::setObject(ossimObject* obj)
{
   m_histogramRemapper = dynamic_cast<ossimHistogramRemapper*>(obj);
   m_cancelState.clear();
   if(m_histogramRemapper.valid()) m_histogramRemapper->saveState(m_cancelState);
   initializeUiValues();
}

void ossimGui::HistogramRemapperEditor::setHistogram(const ossimFilename& file)
{
   if(!m_histogramRemapper.valid()) return;
   
   if(m_histogramRemapper->openHistogram(file))
   {

      initializeUiValues();
      populateClipPoints();
      fireRefreshEvent();
   }
   else
   {
      QMessageBox::warning(this, 
                           tr("My Application"),
                           tr("Unable to open histogram file") + file.c_str(),
                           QMessageBox::Ok);
   }
}

void ossimGui::HistogramRemapperEditor::initializeUiValues()
{
   m_multiResHistogram = 0;
   if(m_histogramRemapper.valid())
   {
      m_stretchModeComboBox->blockSignals(true);
      
      m_enableButton->setChecked(m_histogramRemapper->ossimConnectableObject::getPropertyValueAsString("enabled").toBool());
      m_multiResHistogram = m_histogramRemapper->getHistogram();

      if(!m_multiResHistogram.valid())
      {
         ossimHistogramVisitor visitor;
         m_histogramRemapper->accept(visitor);
         
         m_multiResHistogram = visitor.m_histogram;
         m_histogramRemapper->setHistogram(m_multiResHistogram.get());
      }
      m_stretchModeComboBox->clear();
      m_stretchModeComboBox->addItem("none");
      m_stretchModeComboBox->addItem("auto");
      m_stretchModeComboBox->addItem("1 STD");
      m_stretchModeComboBox->addItem("2 STD");
      m_stretchModeComboBox->addItem("3 STD");
      m_stretchModeComboBox->addItem("linear");
      
      ossim_uint32 nBands = m_histogramRemapper->getNumberOfOutputBands();
      ossim_uint32 idx    = 0;
      m_bandComboBox->clear();
      m_bandComboBox->addItem("master");
      for(idx = 0; idx < nBands;++idx)
      {
         m_bandComboBox->addItem(ossimString::toString(idx).c_str());
      }
      calculateAverageHistogram();
      m_histogramWidget->setHistogram(m_averageHistogram.get());
      
      ossimString stretchMode = m_histogramRemapper->getStretchModeString();
      if(stretchMode == "linear_auto_min_max")
      {
         m_stretchModeComboBox->setCurrentIndex(1);
      }
      else if(stretchMode == "linear_1std_from_mean")
      {
         m_stretchModeComboBox->setCurrentIndex(2);
     }
      else if(stretchMode == "linear_2std_from_mean")
      {
         m_stretchModeComboBox->setCurrentIndex(3);
      }
      else if(stretchMode == "linear_3std_from_mean")
      {
         m_stretchModeComboBox->setCurrentIndex(4);
      }
      else if(stretchMode == "linear_one_piece")
      {
         m_stretchModeComboBox->setCurrentIndex(5);
      }
      else 
      {
         m_stretchModeComboBox->setCurrentIndex(0);
      }
      
      m_histogramFileLineEdit->setText(m_histogramRemapper->getHistogramFile().c_str());
      m_stretchModeComboBox->blockSignals(false);
   }
   else 
   {
      
   }
   populateClipPoints();
   
}

void ossimGui::HistogramRemapperEditor::populateClipPoints()
{
   QString currentText = m_bandComboBox->currentText();
   bool masterFlag = currentText=="master";
   double value =0.0;
   if(m_histogramRemapper.valid())
   {
      value = masterFlag?m_histogramRemapper->getLowNormalizedClipPoint():m_histogramRemapper->getLowNormalizedClipPoint(currentText.toLong());
      m_lowClipPercentLineEdit->setText(QString().setNum(value, 'g', 8));
      value = masterFlag?m_histogramRemapper->getHighNormalizedClipPoint():m_histogramRemapper->getHighNormalizedClipPoint(currentText.toLong());
      m_highClipPercentLineEdit->setText(QString().setNum(value, 'g', 8));
      
      value = masterFlag?m_histogramRemapper->getLowClipPoint():m_histogramRemapper->getLowClipPoint(currentText.toLong());
      m_lowClipValueLineEdit->setText(QString().setNum(value, 'g', 8));
      value = masterFlag?m_histogramRemapper->getHighClipPoint():m_histogramRemapper->getHighClipPoint(currentText.toLong());
      m_highClipValueLineEdit->setText(QString().setNum(value, 'g', 8));
      
      value = masterFlag?m_histogramRemapper->getMidPoint():m_histogramRemapper->getMidPoint(currentText.toLong());
      m_midPointLineEdit->setText(QString().setNum(value, 'g', 8));

      value = masterFlag?m_histogramRemapper->getMinOutputValue():m_histogramRemapper->getMinOutputValue(currentText.toLong());
      m_outputMinValue->setText(QString().setNum(value, 'g', 8));
      value = masterFlag?m_histogramRemapper->getMaxOutputValue():m_histogramRemapper->getMaxOutputValue(currentText.toLong());
      m_outputMaxValue->setText(QString().setNum(value, 'g', 8));
      
      m_histogramWidget->blockSignals(true);
      m_histogramWidget->setPenetration(masterFlag?m_histogramRemapper->getLowNormalizedClipPoint():m_histogramRemapper->getLowNormalizedClipPoint(currentText.toLong()),
                                          1.0-(masterFlag?m_histogramRemapper->getHighNormalizedClipPoint():m_histogramRemapper->getHighNormalizedClipPoint(currentText.toLong()))
                                          );
      m_histogramWidget->blockSignals(false);
  }
}

void	ossimGui::HistogramRemapperEditor::bandActivated ( int index )
{
   if(!m_histogramRemapper.valid()) return;
   if(index == 0)
   {
      m_histogramWidget->setHistogram(m_averageHistogram.get());
   }
   else if(index > 0)
   {
      m_histogramWidget->setHistogram(m_multiResHistogram->getHistogram(index-1).get());
   }
   else 
   {
      m_histogramWidget->setHistogram(0);
   }
   populateClipPoints();
}

void ossimGui::HistogramRemapperEditor::stretchModeActivated(int index)
{
   if(!m_histogramRemapper.valid()) return;
   
   m_histogramRemapper->setStretchModeAsString(stretchModes[index], true);
   if(stretchModes[index] == "linear_one_piece")
   {
      m_histogramWidget->setReadOnly(false);
   }
   else 
   {
      m_histogramWidget->setReadOnly(true);
   }

   populateClipPoints();
   fireRefreshEvent();
}

void ossimGui::HistogramRemapperEditor::openHistogramButton(bool)
{
   if(!m_histogramRemapper.valid()) return;
   QFileDialog* fd = new QFileDialog( this );
   ossimFilename tempFile = m_histogramRemapper->getHistogramFile();
   if(!tempFile.empty())
   {
      fd->setDirectory(tempFile.path().c_str());
   }
   
   QString file;
   if (fd->exec() == QDialog::Accepted )
   {
      QStringList files = fd->selectedFiles();
      if(!files.empty())
      {
         ossimFilename f = (*(files.begin())).toAscii().data();
         setHistogram(f);
         
      }
   }
   
   delete fd;
   fd = NULL;
   
}

void ossimGui::HistogramRemapperEditor::calculateAverageHistogram()
{
   if(!m_multiResHistogram.valid()) return;
   m_averageHistogram = 0;
   ossim_uint32 band = 0;
   ossim_uint32 nBands = m_histogramRemapper->getNumberOfOutputBands();
   std::vector<ossimRefPtr<ossimHistogram> > h;
   if(nBands<1) return;
   h.resize(nBands);
   
   if(!h.size()) return;
   bool validDrawFlag = true;
   for(band = 0; band < nBands; ++ band)
   {
      h[band] = m_multiResHistogram->getHistogram(band);
   }
   
   float minValue = 99999.0;
   float maxValue = -99999.0;
   float max_count = 0.0;
   
   ossim_uint32 maxBins   = 0;  
   for (band = 0; band < nBands; ++band)
   {
      if(h[band].valid())
      {
         float hmin       = (h[band]->GetRangeMin());
         float hmax       = (h[band]->GetRangeMax());
         float hmax_count = h[band]->GetMaxCount();
         ossim_uint32 res = h[band]->GetRes();
         maxBins = res > maxBins?res:maxBins;
         minValue = hmin < minValue ? hmin : minValue;
         maxValue = hmax > maxValue ? hmax : maxValue;
         // max_count = hmax_count > max_count ? hmax_count : max_count;
         max_count += hmax_count;
      }
   }
   m_averageHistogram = new ossimHistogram(maxBins, minValue, maxValue);
   ossim_uint32 binIdx = 0;
   float delta = (maxValue-minValue)/maxBins;
   float* sumCounts = m_averageHistogram->GetCounts();
   ossim_int32 sumIndex = 0;
   float value = 0.0;
   memset(sumCounts, '\0', sizeof(float)*maxBins);
   value = minValue+delta*0.5;
   for(binIdx = 0; binIdx < maxBins;++binIdx)
   {
     // value = minValue+delta*binIdx;
      sumIndex = m_averageHistogram->GetIndex(value);
      if(sumIndex >= 0)
      {
         for(band = 0; band < nBands; ++band)
         {
            if(h[band].valid())
            {
               sumCounts[sumIndex] += h[band]->GetCount(value);
            }
         }
      }
      value+=delta;
   }
 //    ossimKeywordlist kwl;
 //     m_averageHistogram->saveState(kwl);
 //     std::cout << "#################################\n"<<kwl<< std::endl;
 
}

void ossimGui::HistogramRemapperEditor::clipPenetrationsAdjusted(double minValue, double maxValue)
{
   QString currentText = m_bandComboBox->currentText();
   bool masterFlag = currentText=="master";

   if(masterFlag)
   {
      m_histogramRemapper->setLowNormalizedClipPoint(minValue);
      m_histogramRemapper->setHighNormalizedClipPoint(1.0-maxValue);
      
   }
   else 
   {
      int idx = currentText.toLong();
      m_histogramRemapper->setLowNormalizedClipPoint(minValue, idx);
      m_histogramRemapper->setHighNormalizedClipPoint(1.0-maxValue, idx);
   }
   populateClipPoints();
   fireRefreshEvent();
}

void ossimGui::HistogramRemapperEditor::enableButtonClicked(bool checked)
{
   if(m_histogramRemapper.valid())
   {
      m_histogramRemapper->ossimConnectableObject::setProperty("enabled", ossimString::toString(checked));
      fireRefreshEvent();
   }
}

void ossimGui::HistogramRemapperEditor::resetButtonClicked(bool)
{
   if(m_histogramRemapper.valid()) m_histogramRemapper->loadState(m_cancelState);
   initializeUiValues();
   fireRefreshEvent();
}

void ossimGui::HistogramRemapperEditor::okButtonClicked(bool)
{
   close();
}

void ossimGui::HistogramRemapperEditor::cancelButtonClicked(bool)
{
   if(m_histogramRemapper.valid()) m_histogramRemapper->loadState(m_cancelState);
   fireRefreshEvent();
   close();
}

void ossimGui::HistogramRemapperEditor::fireRefreshEvent()
{
   if(m_histogramRemapper.valid())
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_PIXELS);
      ossimEventVisitor visitor(refreshEvent.get());
      m_histogramRemapper->accept(visitor);
   }
}

