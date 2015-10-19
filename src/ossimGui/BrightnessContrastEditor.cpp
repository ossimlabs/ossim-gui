#include <ossimGui/BrightnessContrastEditor.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/base/ossimRefreshEvent.h>

ossimGui::BrightnessContrastEditor::BrightnessContrastEditor(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   m_savedBrightness = 0.0;
   m_savedContrast   = 1.0;
   m_brightnessSlider->setMinimum(-100);
   m_brightnessSlider->setMaximum(100);
   m_contrastSlider->setMinimum(-200);
   m_contrastSlider->setMaximum(200);
   connect(m_okButton, SIGNAL(clicked(bool)), this, SLOT(ok()));
   connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancel()));
   connect(m_brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged()));
   connect(m_contrastSlider, SIGNAL(valueChanged(int)), this, SLOT(contrastSliderChanged()));
   connect(m_enabled, SIGNAL(clicked(bool)), this, SLOT(enabledChanged()));
}


void ossimGui::BrightnessContrastEditor::setObject(ossimObject* obj)
{
   m_brightnessContrast = dynamic_cast<ossimConnectableObject*>(obj);
   if(m_brightnessContrast.valid())
   {
      m_savedBrightness = m_brightnessContrast->getPropertyValueAsString("brightness").toDouble();
      m_savedContrast   = m_brightnessContrast->getPropertyValueAsString("contrast").toDouble();
      ossim_int32 brightness = static_cast<ossim_int32>(m_savedBrightness*100);
      ossim_int32 contrast   = static_cast<ossim_int32>((m_savedContrast-1)*100);
      
      m_brightnessSlider->setValue(brightness);
      m_contrastSlider->setValue(contrast);
      ossimString contrastValue = ossimString::toString(m_savedContrast);
      ossimString brightnessValue = ossimString::toString(m_savedBrightness);
      ossimString enabledValue = m_brightnessContrast->getPropertyValueAsString("enabled");
      m_contrastEdit->setText(contrastValue.c_str());
      m_brightnessEdit->setText(brightnessValue.c_str());
      m_enabled->setCheckState(enabledValue.toBool()?Qt::Checked:Qt::Unchecked);
   }
}


void ossimGui::BrightnessContrastEditor::ok()
{
   close();
}

void ossimGui::BrightnessContrastEditor::cancel()
{
   if(m_brightnessContrast.valid())
   {
      m_brightnessContrast->setProperty(ossimString("brightness"), ossimString::toString(m_savedBrightness));
      m_brightnessContrast->setProperty(ossimString("contrast"),   ossimString::toString(m_savedContrast));
      ossimEventVisitor event(new ossimRefreshEvent(m_brightnessContrast.get()));
      m_brightnessContrast->accept(event);
   }
   close();
}

void ossimGui::BrightnessContrastEditor::brightnessSliderChanged()
{
   if(m_brightnessContrast.valid())
   {
      ossimString value = ossimString::toString(m_brightnessSlider->value()/100.0);
      m_brightnessContrast->setProperty("brightness", value);
      m_brightnessEdit->setText(value.c_str());
   
      ossimEventVisitor event(new ossimRefreshEvent(m_brightnessContrast.get()));
      m_brightnessContrast->accept(event);
   }
}

void ossimGui::BrightnessContrastEditor::contrastSliderChanged()
{
   if(m_brightnessContrast.valid())
   {
      ossimString value = ossimString::toString(1 + m_contrastSlider->value()/100.0);
      
      m_brightnessContrast->setProperty("contrast",  value);
      m_contrastEdit->setText(value.c_str());
      ossimEventVisitor event(new ossimRefreshEvent(m_brightnessContrast.get()));
      m_brightnessContrast->accept(event);
   }
}

void ossimGui::BrightnessContrastEditor::enabledChanged()
{
   if(m_brightnessContrast.valid())
   {
      m_brightnessContrast->setProperty("enabled", (m_enabled->checkState() == Qt::Unchecked)?"false":"true");
      ossimEventVisitor event(new ossimRefreshEvent(m_brightnessContrast.get()));
      m_brightnessContrast->accept(event);
   }
}
