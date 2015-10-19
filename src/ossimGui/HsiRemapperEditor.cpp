#include <ossimGui/HsiRemapperEditor.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimNumericProperty.h>

static const char* HSI_REMAPPER_PROPERTY_PREFIX[] = { "hsi_red_", "hsi_yellow_", "hsi_green_", "hsi_cyan_", "hsi_blue_", "hsi_magenta_", "hsi_master_"};

enum {
   HSI_REMAPPER_ACTIVE_RED     = 0,
   HSI_REMAPPER_ACTIVE_YELLOW  = 1,
   HSI_REMAPPER_ACTIVE_GREEN   = 2,
   HSI_REMAPPER_ACTIVE_CYAN    = 3,
   HSI_REMAPPER_ACTIVE_BLUE    = 4,
   HSI_REMAPPER_ACTIVE_MAGENTA = 5,
   HSI_REMAPPER_ACTIVE_ALL     = 6
};

ossimGui::HsiRemapperEditor::HsiRemapperEditor(QWidget* parent, Qt::WindowFlags f )
:QDialog(parent, f)
{
   setupUi(this);
   
   m_hueOffsetSlider->setMinimum(0);
   m_hueOffsetSlider->setMaximum(360);
   m_hueLowRangeSlider->setMinimum(0);
   m_hueLowRangeSlider->setMaximum(400);
   m_hueHighRangeSlider->setMinimum(0);
   m_hueHighRangeSlider->setMaximum(400);
   m_hueBlendRangeSlider->setMinimum(0);
   m_hueBlendRangeSlider->setMaximum(400);
   m_intensityOffsetSlider->setMinimum(0);
   m_intensityOffsetSlider->setMaximum(400);
   m_saturationOffsetSlider->setMinimum(0);
   m_saturationOffsetSlider->setMaximum(400);
   m_lowIntensityClipSlider->setMinimum(0);
   m_lowIntensityClipSlider->setMaximum(400);
   m_highIntensityClipSlider->setMinimum(0);
   m_highIntensityClipSlider->setMaximum(400);
   m_whiteObjectClipSlider->setMinimum(0);
   m_whiteObjectClipSlider->setMaximum(400);
   
   setAttribute(Qt::WA_DeleteOnClose);
   connect(m_hueOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(hueOffsetChanged(int)));
   connect(m_hueLowRangeSlider, SIGNAL(valueChanged(int)), this, SLOT(hueLowChange(int)));
   connect(m_hueHighRangeSlider, SIGNAL(valueChanged(int)), this, SLOT(hueHighChange(int)));
   connect(m_hueBlendRangeSlider, SIGNAL(valueChanged(int)), this, SLOT(hueBlendChange(int)));
   connect(m_saturationOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(saturationOffsetChange(int)));
   
   connect(m_intensityOffsetSlider, SIGNAL(valueChanged(int)), this, SLOT(intensityOffsetChange(int)));
   connect(m_lowIntensityClipSlider, SIGNAL(valueChanged(int)), this, SLOT(lowIntensityClipChange(int)));
   connect(m_highIntensityClipSlider, SIGNAL(valueChanged(int)), this, SLOT(highIntensityClipChange(int)));
   connect(m_whiteObjectClipSlider, SIGNAL(valueChanged(int)), this, SLOT(whitObjectClipChange(int)));
   connect(m_redButton, SIGNAL(clicked(bool)), this, SLOT(redButtonClicked()));
   connect(m_yellowButton, SIGNAL(clicked(bool)), this, SLOT(yellowButtonClicked()));
   connect(m_greenButton, SIGNAL(clicked(bool)), this, SLOT(greenButtonClicked()));
   connect(m_cyanButton, SIGNAL(clicked(bool)), this, SLOT(cyanButtonClicked()));
   connect(m_blueButton, SIGNAL(clicked(bool)), this, SLOT(blueButtonClicked()));
   connect(m_magentaButton, SIGNAL(clicked(bool)), this, SLOT(magentaButtonClicked()));
   connect(m_allButton, SIGNAL(clicked(bool)), this, SLOT(allButtonClicked()));

   connect(m_resetAllButton, SIGNAL(clicked(bool)), this, SLOT(resetAllButtonClicked()));
   connect(m_resetGroupButton, SIGNAL(clicked(bool)), this, SLOT(resetGroupButtonClicked()));
   connect(m_okButton, SIGNAL(clicked(bool)), this, SLOT(okButtonClicked()));
   connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancelButtonClicked()));
  
   connect(m_enableButton, SIGNAL(clicked(bool)), this, SLOT(enableButtonClicked(bool)));
   
   m_allButton->setChecked(true);
   m_activeGroup = HSI_REMAPPER_ACTIVE_ALL;
}

void ossimGui::HsiRemapperEditor::setObject(ossimObject* obj)
{
   m_object = dynamic_cast<ossimHsiRemapper*>(obj);
   if(m_object.valid())
   {
      m_cancelState.clear();
      m_object->saveState(m_cancelState);
   }
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::hueOffsetChanged(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_hueOffsetRange[0] + (m_hueOffsetRange[1] - m_hueOffsetRange[0])*static_cast<double>(value)/m_hueOffsetSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"hue_offset", ossimString::toString(propValue));
   m_hueOffsetValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::hueLowChange(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_hueLowRange[0] + (m_hueLowRange[1] - m_hueLowRange[0])*static_cast<double>(value)/m_hueLowRangeSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"hue_low_range", ossimString::toString(propValue));
   m_hueLowRangeValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::hueHighChange(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_hueHighRange[0] + (m_hueHighRange[1] - m_hueHighRange[0])*static_cast<double>(value)/m_hueHighRangeSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"hue_high_range", ossimString::toString(propValue));
   m_hueHighRangeValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::hueBlendChange(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_hueBlendRange[0] + (m_hueBlendRange[1] - m_hueBlendRange[0])*static_cast<double>(value)/m_hueBlendRangeSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"hue_blend_range", ossimString::toString(propValue));
   m_hueBlendRangeValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::saturationOffsetChange(int value)
{
   
   if(!m_object.valid()) return;
   double propValue = (m_saturationOffsetRange[0] + (m_saturationOffsetRange[1] - m_saturationOffsetRange[0])*static_cast<double>(value)/m_saturationOffsetSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"saturation_offset", ossimString::toString(propValue));
   m_saturationOffsetValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
   
}

void ossimGui::HsiRemapperEditor::intensityOffsetChange(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_intensityOffsetRange[0] + (m_intensityOffsetRange[1] - m_intensityOffsetRange[0])*static_cast<double>(value)/m_intensityOffsetSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"intensity_offset", ossimString::toString(propValue));
   m_intensityOffsetValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::lowIntensityClipChange(int value)
{
   if(!m_object.valid()) return;
   
   double propValue = (m_lowIntensityClipRange[0] + (m_lowIntensityClipRange[1] - m_lowIntensityClipRange[0])*static_cast<double>(value)/m_lowIntensityClipSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"intensity_low_clip", ossimString::toString(propValue));
   m_lowIntensityClipValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::highIntensityClipChange(int value)
{
   if(!m_object.valid()) return;
   
   double propValue = (m_highIntensityClipRange[0] + (m_highIntensityClipRange[1] - m_highIntensityClipRange[0])*static_cast<double>(value)/m_highIntensityClipSlider->maximum());
   m_object->ossimConnectableObject::setProperty(getPropertyPrefix()+"intensity_high_clip",ossimString::toString(propValue));
   m_highIntensityClipValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::whitObjectClipChange(int value)
{
   if(!m_object.valid()) return;
   double propValue = (m_whiteObjectClipRange[0] + (m_whiteObjectClipRange[1] - m_whiteObjectClipRange[0])*static_cast<double>(value)/m_whiteObjectClipSlider->maximum());
   m_object->ossimConnectableObject::setProperty("hsi_white_object_clip", ossimString::toString(propValue));
   m_whiteObjectClipValueLabel->setText(ossimString::toString(roundForDisplay(propValue)).c_str());
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::redButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_RED;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::yellowButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_YELLOW;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::greenButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_GREEN;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::cyanButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_CYAN;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::blueButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_BLUE;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::magentaButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_MAGENTA;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::allButtonClicked()
{
   m_activeGroup = HSI_REMAPPER_ACTIVE_ALL;
   initializeUiValues();
}

void ossimGui::HsiRemapperEditor::enableButtonClicked(bool checked)
{
   if(!m_object.valid()) return;
   static_cast<ossimConnectableObject*>(m_object.get())->setProperty("enabled", ossimString::toString(checked));
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::resetGroupButtonClicked()
{
   if(!m_object.valid()) return;
   m_object->resetGroup(m_activeGroup);
   initializeUiValues();
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::resetAllButtonClicked()
{
   if(!m_object.valid()) return;
   m_object->resetAll();
   initializeUiValues();
   fireRefreshEvent();
}

void ossimGui::HsiRemapperEditor::okButtonClicked()
{
   close();
}

void ossimGui::HsiRemapperEditor::cancelButtonClicked()
{
   if(m_object.valid()) m_object->loadState(m_cancelState);
   fireRefreshEvent();
   close();
}

ossimString ossimGui::HsiRemapperEditor::getPropertyPrefix()const
{
   return ossimString(HSI_REMAPPER_PROPERTY_PREFIX[m_activeGroup]);
}
       
void ossimGui::HsiRemapperEditor::fireRefreshEvent()
{
   if(m_object.valid())
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_PIXELS);
      ossimEventVisitor visitor(refreshEvent.get());
      m_object->accept(visitor);
   }
}

double ossimGui::HsiRemapperEditor::roundForDisplay(double value, int precision)const
{
   return ( static_cast<ossim_int32>(value*precision)/static_cast<double>(precision) );
}

void ossimGui::HsiRemapperEditor::initializeUiValues()
{
   if(!m_object.valid()) return;
   m_hueOffsetSlider->blockSignals(true);
   m_hueLowRangeSlider->blockSignals(true);
   m_hueHighRangeSlider->blockSignals(true);
   m_hueBlendRangeSlider->blockSignals(true);
   m_saturationOffsetSlider->blockSignals(true);
   m_lowIntensityClipSlider->blockSignals(true);
   m_highIntensityClipSlider->blockSignals(true);
   m_whiteObjectClipSlider->blockSignals(true);
   m_enableButton->blockSignals(true);
   
   m_enableButton->setChecked(m_object->isSourceEnabled());
   
   ossimString prefix = getPropertyPrefix();
   ossimRefPtr<ossimNumericProperty> hueOffset;
   ossimRefPtr<ossimNumericProperty> lowRange;
   ossimRefPtr<ossimNumericProperty> highRange;
   ossimRefPtr<ossimNumericProperty> blendRange;
   ossimRefPtr<ossimNumericProperty> saturationOffset;
   ossimRefPtr<ossimNumericProperty> intensityOffset;
   ossimRefPtr<ossimNumericProperty> intensityLowClip;
   ossimRefPtr<ossimNumericProperty> intensityHighClip;
   ossimRefPtr<ossimNumericProperty> whiteObjectClip;
   
   switch(m_activeGroup)
   {
      case HSI_REMAPPER_ACTIVE_ALL:
      {
         m_hueOffsetSlider->setEnabled(true);
         m_hueLowRangeSlider->setEnabled(false);
         m_hueHighRangeSlider->setEnabled(false);
         m_hueBlendRangeSlider->setEnabled(false);
         m_saturationOffsetSlider->setEnabled(true);
         m_lowIntensityClipSlider->setEnabled(true);
         m_highIntensityClipSlider->setEnabled(true);
         m_whiteObjectClipSlider->setEnabled(true);
         
         
         break;
      }
      default:
      {
         m_hueOffsetSlider->setEnabled(true);
         m_hueLowRangeSlider->setEnabled(true);
         m_hueHighRangeSlider->setEnabled(true);
         m_hueBlendRangeSlider->setEnabled(true);
         m_saturationOffsetSlider->setEnabled(true);
         m_lowIntensityClipSlider->setEnabled(false);
         m_highIntensityClipSlider->setEnabled(false);
         m_whiteObjectClipSlider->setEnabled(false);
         
         break;
      }
   }
   hueOffset         = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"hue_offset").get());
   lowRange          = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"hue_low_range").get());
   highRange         = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"hue_high_range").get());
   blendRange        = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"hue_blend_range").get());
   saturationOffset  = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"saturation_offset").get());
   intensityOffset   = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"intensity_offset").get());
   intensityLowClip  = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"intensity_low_clip").get());
   intensityHighClip = dynamic_cast<ossimNumericProperty*>(m_object->getProperty(prefix+"intensity_high_clip").get());
   whiteObjectClip  = dynamic_cast<ossimNumericProperty*>(m_object->getProperty("hsi_white_object_clip").get());
   
   m_hueOffsetRange[0] = 0.0;
   m_hueOffsetRange[1] = 1.0;
   m_hueLowRange[0] = 0.0;
   m_hueLowRange[1] = 1.0;
   m_hueHighRange[0] = 0.0;
   m_hueHighRange[1] = 1.0;
   m_hueBlendRange[0] = 0.0;
   m_hueBlendRange[1] = 1.0;
   m_saturationOffsetRange[0] = 0.0;
   m_saturationOffsetRange[1] = 1.0;
   m_intensityOffsetRange[0] = 0.0;
   m_intensityOffsetRange[1] = 1.0;
   m_lowIntensityClipRange[0] = 0.0;
   m_lowIntensityClipRange[1] = 1.0;
   m_highIntensityClipRange[0] = 0.0;
   m_highIntensityClipRange[1] = 1.0;
   m_whiteObjectClipRange[0] = 0.0;
   m_whiteObjectClipRange[1] = 1.0;
   
   
   if(hueOffset.valid())
   {
      ossimString value = hueOffset->ossimProperty::valueToString();
      m_hueOffsetRange[0] = hueOffset->getMinValue();
      m_hueOffsetRange[1] = hueOffset->getMaxValue();
      double range        = m_hueOffsetRange[1] - m_hueOffsetRange[0];
      double relative     = value.toDouble() - m_hueOffsetRange[0];
      double t            = relative/range;
      m_hueOffsetSlider->setValue(t*m_hueOffsetSlider->maximum());
      m_hueOffsetValueLabel->setText(value.c_str());
   }
   else
   {
      m_hueOffsetValueLabel->setText("");
   }
   if(lowRange.valid())
   {
      ossimString value = lowRange->ossimProperty::valueToString();
      m_hueLowRange[0] = lowRange->getMinValue();
      m_hueLowRange[1] = lowRange->getMaxValue();
      double range        = m_hueLowRange[1] - m_hueLowRange[0];
      double relative     = value.toDouble() - m_hueLowRange[0];
      double t            = relative/range;
      m_hueLowRangeSlider->setValue(t*m_hueLowRangeSlider->maximum());
      m_hueLowRangeValueLabel->setText(value.c_str());
   }
   else
   {
      m_hueLowRangeValueLabel->setText("");
   }
   if(highRange.valid())
   {
      ossimString value = highRange->ossimProperty::valueToString();
      m_hueHighRange[0] = highRange->getMinValue();
      m_hueHighRange[1] = highRange->getMaxValue();
      double range        = m_hueHighRange[1] - m_hueHighRange[0];
      double relative     = value.toDouble() - m_hueHighRange[0];
      double t            = relative/range;
      m_hueHighRangeSlider->setValue(t*m_hueHighRangeSlider->maximum());
      m_hueHighRangeValueLabel->setText(value.c_str());
   }
   else
   {
      m_hueHighRangeValueLabel->setText("");
   }
   if(blendRange.valid())
   {
      ossimString value = blendRange->ossimProperty::valueToString();
      m_hueBlendRange[0] = blendRange->getMinValue();
      m_hueBlendRange[1] = blendRange->getMaxValue();
      double range        = m_hueBlendRange[1] - m_hueBlendRange[0];
      double relative     = value.toDouble()   - m_hueBlendRange[0];
      double t            = relative/range;
      m_hueBlendRangeSlider->setValue(t*m_hueBlendRangeSlider->maximum());
      m_hueBlendRangeValueLabel->setText(value.c_str());
   }
   else
   {
      m_hueBlendRangeValueLabel->setText("");
   }
   if(saturationOffset.valid())
   {
      ossimString value = saturationOffset->ossimProperty::valueToString();
      m_saturationOffsetRange[0] = saturationOffset->getMinValue();
      m_saturationOffsetRange[1] = saturationOffset->getMaxValue();
      double range        = m_saturationOffsetRange[1] - m_saturationOffsetRange[0];
      double relative     = value.toDouble() - m_saturationOffsetRange[0];
      double t            = relative/range;
      m_saturationOffsetSlider->setValue(t*m_saturationOffsetSlider->maximum());
      m_saturationOffsetValueLabel->setText(value.c_str());
   }
   else
   {
      m_saturationOffsetValueLabel->setText("");
   }

   if(intensityOffset.valid())
   {
      ossimString value = intensityOffset->ossimProperty::valueToString();
      m_intensityOffsetRange[0] = intensityOffset->getMinValue();
      m_intensityOffsetRange[1] = intensityOffset->getMaxValue();
      double range        = m_intensityOffsetRange[1] - m_intensityOffsetRange[0];
      double relative     = value.toDouble() - m_intensityOffsetRange[0];
      double t            = relative/range;
      m_intensityOffsetSlider->setValue(t*m_intensityOffsetSlider->maximum());
      m_intensityOffsetValueLabel->setText(value.c_str());
   }
   else 
   {
      m_intensityOffsetValueLabel->setText("");
   }

   if(intensityLowClip.valid())
   {
      ossimString value = intensityLowClip->ossimProperty::valueToString();
      m_lowIntensityClipRange[0] = intensityLowClip->getMinValue();
      m_lowIntensityClipRange[1] = intensityLowClip->getMaxValue();
      double range        = m_lowIntensityClipRange[1] - m_lowIntensityClipRange[0];
      double relative     = value.toDouble() - m_lowIntensityClipRange[0];
      double t            = relative/range;
      m_lowIntensityClipSlider->setValue(t*m_lowIntensityClipSlider->maximum());
      m_lowIntensityClipValueLabel->setText(value.c_str());
   }
   else
   {
      m_lowIntensityClipValueLabel->setText("");
   }
   if(intensityHighClip.valid())
   {
      ossimString value = intensityHighClip->ossimProperty::valueToString();
      m_highIntensityClipRange[0] = intensityHighClip->getMinValue();
      m_highIntensityClipRange[1] = intensityHighClip->getMaxValue();
      double range        = m_highIntensityClipRange[1] - m_highIntensityClipRange[0];
      double relative     = value.toDouble() - m_highIntensityClipRange[0];
      double t            = relative/range;
      m_highIntensityClipSlider->setValue(t*m_highIntensityClipSlider->maximum());
      m_highIntensityClipValueLabel->setText(value.c_str());
   }
   else
   {
      m_highIntensityClipValueLabel->setText("");
   }
   if(whiteObjectClip.valid())
   {
      ossimString value = whiteObjectClip->ossimProperty::valueToString();
      m_whiteObjectClipRange[0] = whiteObjectClip->getMinValue();
      m_whiteObjectClipRange[1] = whiteObjectClip->getMaxValue();
      double range        = m_whiteObjectClipRange[1] - m_whiteObjectClipRange[0];
      double relative     = value.toDouble() - m_whiteObjectClipRange[0];
      double t            = relative/range;
      m_whiteObjectClipSlider->setValue(t*m_whiteObjectClipSlider->maximum());
      m_whiteObjectClipValueLabel->setText(value.c_str());
   }
   else
   {
      m_whiteObjectClipValueLabel->setText("");
   }
   
   m_hueOffsetSlider->blockSignals(false);
   m_hueLowRangeSlider->blockSignals(false);
   m_hueHighRangeSlider->blockSignals(false);
   m_hueBlendRangeSlider->blockSignals(false);
   m_saturationOffsetSlider->blockSignals(false);
   m_lowIntensityClipSlider->blockSignals(false);
   m_highIntensityClipSlider->blockSignals(false);
   m_whiteObjectClipSlider->blockSignals(false);
   m_enableButton->blockSignals(false);
}


