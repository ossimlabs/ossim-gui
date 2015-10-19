#include <ossimGui/BandSelectorEditor.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageSource.h>

ossimGui::BandSelectorEditor::BandSelectorEditor(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);

   m_currentIndex  = 0;
   m_selectionType = N_BAND;
   m_nBandButton->setChecked(true);
   m_bandInput->setReadOnly(true);
   
   connect(m_oneBandButton, SIGNAL(clicked(bool)), this, SLOT(oneBandButtonClicked(bool)));
   connect(m_threeBandButton, SIGNAL(clicked(bool)), this, SLOT(threeBandButtonClicked(bool)));
   connect(m_nBandButton, SIGNAL(clicked(bool)), this, SLOT(nBandButtonClicked(bool)));
   connect(m_clearBandInputButton, SIGNAL(clicked(bool)), this, SLOT(clearBandInputButtonClicked(bool)));
   connect(m_inputBandList, SIGNAL(	clicked ( const QModelIndex &)), this, SLOT(inputBandListClicked ( const QModelIndex & )));
   connect(m_enableButton, SIGNAL(clicked(bool)), this, SLOT(enableButtonClicked(bool)));
   connect(m_resetButton, SIGNAL(clicked(bool)), this, SLOT(resetButtonClicked(bool)));
   connect(m_okButton, SIGNAL(clicked(bool)), this, SLOT(okButtonClicked(bool)));
   connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancelButtonClicked(bool)));
   
}

void ossimGui::BandSelectorEditor::setObject(ossimObject* obj)
{
   m_object = dynamic_cast<ossimImageSource*>(obj); // must be at least of type osismImageSource
   if(m_object.valid())
   {
      m_cancelState.clear();
      m_object->saveState(m_cancelState);
   }
   initializeUiValues();
}

void ossimGui::BandSelectorEditor::oneBandButtonClicked(bool)
{
   if(!m_object.valid()) return;
   std::vector<ossim_uint32> bands;
   ossim::toSimpleVector(bands, m_object->getPropertyValueAsString("bands"));
   m_currentBandList.resize(1);
   if(bands.size())
   {
      m_currentBandList[0] = bands[0];
   }
   else 
   {
      m_currentBandList[0] = 0;
   }

   m_currentIndex = 0;
   m_selectionType = SINGLE_BAND;
   setBandInput();
   setCurrentBandsToObject();
}   

void ossimGui::BandSelectorEditor::threeBandButtonClicked(bool)
{
   if(!m_object.valid()) return;
   m_currentBandList.resize(3);
   
   ossim_uint32 nbands = getNumberOfInputBands();
   if(nbands >= 3)
   {
      m_currentBandList[0] = 0;
      m_currentBandList[1] = 1;
      m_currentBandList[2] = 2;
      
      m_currentIndex = 0;
   }
   else 
   {
      m_currentBandList[0] = 0;
      m_currentBandList[1] = 0;
      m_currentBandList[2] = 0;
   }

   m_selectionType = THREE_BAND;
   setBandInput();
   setCurrentBandsToObject();
}

void ossimGui::BandSelectorEditor::nBandButtonClicked(bool)
{
   m_currentIndex = 0;
   m_selectionType = N_BAND;
}

void ossimGui::BandSelectorEditor::clearBandInputButtonClicked(bool)
{
   m_currentIndex = 0;
   ossim_uint32 nbands = getNumberOfInputBands();
   switch(m_selectionType)
   {
      case SINGLE_BAND:
      {
         m_currentBandList.resize(3);
         m_currentBandList[0] = 0;
         break;
      }
      case THREE_BAND:
      {
         m_currentBandList.resize(3);
         if(nbands >= 3)
         {
            m_currentBandList[0] = 0;
            m_currentBandList[1] = 1;
            m_currentBandList[2] = 2;
            
            m_currentIndex = 0;
         }
         else 
         {
            m_currentBandList[0] = 0;
            m_currentBandList[1] = 0;
            m_currentBandList[2] = 0;
         }
         
         break;
      }
      case N_BAND:
      {
         m_currentBandList.resize(1);
         m_currentBandList[0] = 0;
         break;
      }
   }
   setBandInput();
   setCurrentBandsToObject();
}

void ossimGui::BandSelectorEditor::inputBandListClicked(const QModelIndex & index)
{
   ossim_uint32 band = index.data().toInt();
   if(m_currentIndex == m_currentBandList.size())
   {
      m_currentBandList.push_back(band-1);
   }
   else
   {
      m_currentBandList[m_currentIndex] = band-1;
   }
   switch(m_selectionType)
   {
      case SINGLE_BAND:
      {
         break;
      }
      case THREE_BAND:
      {
         m_currentIndex = (m_currentIndex + 1)%3;
         break;
      }
      case N_BAND:
      {
         ++m_currentIndex;
         break;
      }
   }

   m_inputBandList->clearSelection();
   setBandInput();
   setCurrentBandsToObject();
}


void ossimGui::BandSelectorEditor::enableButtonClicked(bool checked)
{
   if(m_object.valid())
   {
      m_object->setProperty("enabled", ossimString::toString(checked));
      fireRefreshEvent();
   }
}

void ossimGui::BandSelectorEditor::resetButtonClicked(bool)
{
   if(m_object.valid()) m_object->loadState(m_cancelState);
   initializeUiValues();
   fireRefreshEvent();
}

void ossimGui::BandSelectorEditor::okButtonClicked(bool)
{
   close();
}

void ossimGui::BandSelectorEditor::cancelButtonClicked(bool)
{
   if(m_object.valid()) m_object->loadState(m_cancelState);
   fireRefreshEvent();
   close();
}

void ossimGui::BandSelectorEditor::initializeUiValues()
{
   if(!m_object) return;
   m_oneBandButton->blockSignals(true);
   m_threeBandButton->blockSignals(true);
   m_nBandButton->blockSignals(true);
   m_enableButton->blockSignals(true);
   m_currentBandList.clear();
   m_enableButton->setChecked(m_object->getPropertyValueAsString("enabled").toBool());
   ossim::toSimpleVector(m_currentBandList, m_object->getPropertyValueAsString("bands"));
   
   if(m_currentBandList.size() == 1)
   {
      m_oneBandButton->setChecked(true);
      m_selectionType = SINGLE_BAND;
   }
   else if(m_currentBandList.size() == 3)
   {
      m_threeBandButton->setChecked(true);
      m_selectionType = THREE_BAND;
   }
   else 
   {
      m_nBandButton->setChecked(true);
      m_selectionType = N_BAND;
   }
   m_inputBandList->clear();
   ossim_uint32 nInputBands = getNumberOfInputBands();
   if(nInputBands > 0)
   {
      ossimString bandValue;
      ossim_uint32 idx = 0;
      for(idx = 0; idx < nInputBands;++idx)
      {
         bandValue = ossimString::toString(idx+1);
         m_inputBandList->addItem(bandValue.c_str());
      }
   }
   m_currentIndex  = 0;
   setBandInput();
   
   m_enableButton->blockSignals(false);
   m_oneBandButton->blockSignals(false);
   m_threeBandButton->blockSignals(false);
   m_nBandButton->blockSignals(false);
}

void ossimGui::BandSelectorEditor::setBandInput()
{
   ossimString bandList;
   ossim_uint32 idx = 0;
   for(idx = 0; idx < m_currentBandList.size();++idx)
   {
      bandList += ossimString::toString(m_currentBandList[idx]+1);
      if(idx+1 != m_currentBandList.size()) bandList += ",";
   }
   m_bandInput->setText(bandList.c_str());
   
}

ossim_uint32 ossimGui::BandSelectorEditor::getNumberOfInputBands()const
{
   const ossimImageSource* is = dynamic_cast<const ossimImageSource*>(m_object.get());
   if(is)
   {
      return is->getNumberOfInputBands();
   }
   
   return 0;
}

void ossimGui::BandSelectorEditor::setCurrentBandsToObject()
{
   ossimString bandsString;
   ossim::toSimpleStringList(bandsString,
                             m_currentBandList);
   m_object->setProperty("bands", bandsString);
   
   fireRefreshEvent();
}

void ossimGui::BandSelectorEditor::fireRefreshEvent()
{
   if(m_object.valid())
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_PIXELS);
      ossimEventVisitor visitor(refreshEvent.get());
      m_object->accept(visitor);
   }
}
