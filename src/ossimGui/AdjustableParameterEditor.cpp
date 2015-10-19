#include <ossimGui/AdjustableParameterEditor.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QFileDialog>
static const int NAME_INDEX       = 0;
static const int SIGMA_INDEX      = 1;
static const int PARAMETER_INDEX  = 2;
static const int SLIDER_INDEX     = 3;
static const int VALUE_INDEX      = 4;

ossimGui::AdjustableParameterEditor::AdjustableParameterEditor(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f),
m_interface(0)
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   connect(m_adjustableParameterTable, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged(int, int)));
   connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetTable()));
   connect(m_keepAdjustmentButton, SIGNAL(clicked()), this, SLOT(keepAdjustment()));
   connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveAdjustment()));
   connect(m_closeButton, SIGNAL(clicked()), this,SLOT(close()));
   connect(m_copyAdjustmentButton, SIGNAL(clicked()), this,SLOT(copyAdjustment()));
   connect(m_deleteAdjustmentButton, SIGNAL(clicked()), this,SLOT(deleteAdjustment()));
   connect(m_adjustmentSelectionBox, SIGNAL(activated(const QString&)), this,SLOT(selectionListChanged()));
   connect(m_adjustmentDescriptionInput, SIGNAL(textChanged(const QString&)), this, SLOT(adjustmentDescriptionChanged(const QString&)));
   connect(this, SIGNAL(sourceChanged(const QString&)), this, SLOT(setSource(const QString&)));
}

void ossimGui::AdjustableParameterEditor::setObject(ossimObject* obj)
{
   m_object = obj;
   if(m_object.valid())
   {
      m_interface = dynamic_cast<ossimAdjustableParameterInterface*>(obj);
      if(!m_interface)
      {
         ossimImageSource* isource = dynamic_cast<ossimImageSource*>(obj);
         if(isource)
         {
            ossimRefPtr<ossimImageGeometry> geom = isource->getImageGeometry();
            if(geom.valid())
            {
               m_interface = dynamic_cast<ossimAdjustableParameterInterface*>(geom->getProjection());
            }
         }
      }
   }

   setImageSource();

   transferToDialog();
}

void ossimGui::AdjustableParameterEditor::setImageSource()
{
   ossimString imageSource(" ");

   if(m_object.valid())
   {
      ossimTypeNameVisitor visitor("ossimImageHandler");
      m_object->accept(visitor);
      ossimImageHandler* handler = visitor.getObjectAs<ossimImageHandler>();
      if(handler)
      {
         imageSource = handler->getFilename();
      }
   }

   QString source(imageSource.data());
   emit sourceChanged(source);
}

void ossimGui::AdjustableParameterEditor::setSource(const QString& source)
{
   m_imageSourceLabel->setText(source);
}


ossimFilename ossimGui::AdjustableParameterEditor::findDefaultFilename()
{
   ossimFilename result;
   if(m_object.valid())
   {
      ossimTypeNameVisitor visitor("ossimImageHandler", true);
      m_object->accept(visitor);
      ossimImageHandler* handler = visitor.getObjectAs<ossimImageHandler>();
      
      if(handler)
      {
         result = handler->createDefaultGeometryFilename();
      }
   }
   
   return result;
}


void ossimGui::AdjustableParameterEditor::transferToDialog()
{
   transferToList();   
   transferToTable();
}

void ossimGui::AdjustableParameterEditor::transferToList()
{
   m_adjustmentSelectionBox->blockSignals(true);
   m_adjustmentDescriptionInput->blockSignals(true);
   m_adjustmentSelectionBox->clear();
   if(m_interface)
   {
      ossim_uint32 n = m_interface->getNumberOfAdjustments();
      ossim_int32 idx = 0;
      
      for(idx = 0; idx < (int)n; ++idx)
      {
         m_adjustmentSelectionBox->addItem(ossimString::toString(idx).c_str());
      }
      ossim_uint32 adjIdx = m_interface->getCurrentAdjustmentIdx();
      m_adjustmentSelectionBox->setCurrentIndex(adjIdx);
      m_adjustmentDescriptionInput->setText(m_interface->getAdjustmentDescription().c_str());
   }
   m_adjustmentSelectionBox->blockSignals(false);
   m_adjustmentDescriptionInput->blockSignals(false);
}

void ossimGui::AdjustableParameterEditor::transferToTable()
{
   if(!m_interface)
   {
      m_adjustableParameterTable->clearContents();
      return;
   }
   if(m_interface)
   {
      m_adjustableParameterTable->blockSignals(true);

      int numAdjustables = m_interface->getNumberOfAdjustableParameters();
      if(numAdjustables > 0)
      {
         if(m_adjustableParameterTable->rowCount() != numAdjustables)
         {
            m_adjustableParameterTable->setRowCount(numAdjustables);
         }
         for(int idx = 0; idx < numAdjustables; ++idx)
         {
            ossimString description = m_interface->getParameterDescription(idx).c_str();
            double sigma            = m_interface->getParameterSigma(idx);
            double parameter         = m_interface->getAdjustableParameter(idx);
            double offset           = m_interface->computeParameterOffset(idx);
            
            if(!m_adjustableParameterTable->item(idx, NAME_INDEX))
            {
               m_adjustableParameterTable->setItem(idx, NAME_INDEX, new QTableWidgetItem(description.c_str()));
            }
            else 
            {
               m_adjustableParameterTable->item(idx, NAME_INDEX)->setText(description.c_str());
            }
            
            if(!m_adjustableParameterTable->item(idx, SIGMA_INDEX))
            {
               m_adjustableParameterTable->setItem(idx, SIGMA_INDEX, new QTableWidgetItem(QString().setNum(sigma)));
            }
            else 
            {
               m_adjustableParameterTable->item(idx, SIGMA_INDEX)->setText(QString().setNum(sigma));
               
            }
            if(!m_adjustableParameterTable->item(idx, PARAMETER_INDEX))
            {
               m_adjustableParameterTable->setItem(idx, PARAMETER_INDEX, new QTableWidgetItem(QString().setNum(parameter)));
            }
            else 
            {
               m_adjustableParameterTable->item(idx, PARAMETER_INDEX)->setText(QString().setNum(parameter));
            }
            if(!m_adjustableParameterTable->item(idx, VALUE_INDEX))
            {
               m_adjustableParameterTable->setItem(idx, VALUE_INDEX, new QTableWidgetItem(QString().setNum(offset)));
            }
            else 
            {
               m_adjustableParameterTable->item(idx, VALUE_INDEX)->setText(QString().setNum(offset));
            }
//            if(!m_adjustableParameterTable->item(idx, SLIDER_INDEX))
//            {
//               m_adjustableParameterTable->setItem(idx, SLIDER_INDEX, new QTableWidgetItem(QString().setNum(value)));
//            }
//            else 
//            {
//               m_adjustableParameterTable->item(idx, SLIDER_INDEX)->setText(QString().setNum(m_interface->getAdjustableParameter(idx)));
//            }
            
            AdjustableParameterSlider* slider = 0;
            if(!m_adjustableParameterTable->cellWidget(idx, SLIDER_INDEX))
            {
               slider = new AdjustableParameterSlider(idx, SLIDER_INDEX);
               connect(slider, SIGNAL(parameterChanged(int, int)), this, SLOT(valueChanged(int, int)));
               m_adjustableParameterTable->setCellWidget(idx, SLIDER_INDEX, slider);
            }
            else 
            {
               slider = dynamic_cast<AdjustableParameterSlider*>(m_adjustableParameterTable->cellWidget(idx, SLIDER_INDEX));
            }

            if(slider)
            {
               slider->blockSignals(true);
               slider->setTracking(true);
               slider->setMinimum(-100);
               slider->setMaximum(100);
               slider->setValue((int)(parameter*100.0));
               slider->blockSignals(false);
            }
         }
      }
      else 
      {
         m_adjustableParameterTable->clearContents();
      }

      m_adjustableParameterTable->blockSignals(false);
   }
   
}

void ossimGui::AdjustableParameterEditor::resetTable()
{
   if(!m_interface) return;
   int numAdjustables = m_interface->getNumberOfAdjustableParameters();
   if(numAdjustables > 0)
   {
      m_interface->setDirtyFlag(true);
      m_interface->resetAdjustableParameters(true);
      transferToTable();
      fireRefreshEvent();
   }
}

void ossimGui::AdjustableParameterEditor::keepAdjustment()
{
   if(m_interface)
   {
      m_interface->setDirtyFlag(true);
      m_interface->keepAdjustment();
      transferToDialog();
   }
}

void ossimGui::AdjustableParameterEditor::saveAdjustment()
{
   if(!m_interface) return;
   
   if(m_filename == "")
   {
      m_filename = findDefaultFilename();
      
      QString file = QFileDialog::getSaveFileName((QWidget*)this, tr("Save Geometry"), tr(m_filename.c_str()), tr("*.geom"));
      if(file != "")
      {
         m_filename = file.toAscii().data();
         ossimKeywordlist kwl;
         m_interface->getBaseObject()->saveState(kwl);
         
         if(kwl.write(m_filename))
         {
            m_interface->setDirtyFlag(false);
         }
      }
   }
}

void ossimGui::AdjustableParameterEditor::copyAdjustment()
{
   if(m_interface)
   {
      m_interface->setDirtyFlag(true);
      m_interface->copyAdjustment(true);
      transferToDialog();
   }   
}

void ossimGui::AdjustableParameterEditor::deleteAdjustment()
{
   if(m_interface)
   {
      m_interface->setDirtyFlag(true);
      m_interface->eraseAdjustment(true);
      if(m_interface->getNumberOfAdjustments() < 1)
      {
         m_interface->initAdjustableParameters();
      }
      transferToDialog();
      fireRefreshEvent();
   }   
}
void ossimGui::AdjustableParameterEditor::selectionListChanged()
{
   if(m_interface)
   {
      m_interface->setDirtyFlag(true);
      ossim_uint32 idx = m_adjustmentSelectionBox->currentText().toUInt();
      m_interface->setCurrentAdjustment(idx, true);
      transferToDialog();
      fireRefreshEvent();
   }
}

void ossimGui::AdjustableParameterEditor::valueChanged(int row, int col)
{
   m_adjustableParameterTable->blockSignals(true);
   QSlider* slider = dynamic_cast<QSlider*>(m_adjustableParameterTable->cellWidget(row, SLIDER_INDEX));
   if(!slider) return;
   if(col == SLIDER_INDEX)
   {
      m_interface->setDirtyFlag(true);
      QSlider* slider = (QSlider*)m_adjustableParameterTable->cellWidget(row, col);
      
      int value = slider->value();
      double multiplier = (double)value/100.0;
      m_interface->setAdjustableParameter(row, multiplier, true);
      ossimString parameterValue = ossimString::toString(m_interface->getAdjustableParameter(row));
      if(parameterValue == ".") parameterValue = "0";
      
      m_adjustableParameterTable->item(row, PARAMETER_INDEX)->setText(parameterValue.c_str());
      
      ossimString valueOffset    = ossimString::toString(m_interface->computeParameterOffset(row));
      
      //if(valueOffset == ".") valueOffset = "0";
      
      m_adjustableParameterTable->item(row, VALUE_INDEX)->setText(valueOffset.c_str());
      fireRefreshEvent();
   }
   else if(col == SIGMA_INDEX)
   {
      m_interface->setDirtyFlag(true);
      ossimString sigma = m_adjustableParameterTable->item(row,col)->text().toAscii().data();
      m_interface->setParameterSigma(row, sigma.toDouble(), true);
      fireRefreshEvent();
      transferToTable();
   }
   else if(col == PARAMETER_INDEX)
   {
      m_interface->setDirtyFlag(true);
      ossimString param = m_adjustableParameterTable->item(row,col)->text().toAscii().data();
      m_interface->setAdjustableParameter(row, param.toDouble(), true);
      fireRefreshEvent();
      transferToTable();
   }
   else if(col == VALUE_INDEX)
   {
      m_interface->setDirtyFlag(true);
      double center   = m_interface->getParameterCenter(row);
      double sigma    = m_interface->getParameterSigma(row);
      double minValue = center - sigma;
      double maxValue = center + sigma;
      double value    = ossimString(m_adjustableParameterTable->item(row,col)->text().toAscii().data()).toDouble();
      double x = 0.0;
      
      if(sigma != 0.0)
      {
         //
         // sigma*x + center = value;
         // x = (value - center)/sigma
         x = (value - center)/sigma;
         
         value = center + x*sigma;
         
         if(value < minValue)
         {
            x = -1;
         }
         else if(value >maxValue)
         {
            x = 1.0;
         }
         m_interface->setAdjustableParameter(row, x, true);
         fireRefreshEvent();
         transferToTable();
      }
   }
   m_adjustableParameterTable->blockSignals(false);
}
void ossimGui::AdjustableParameterEditor::fireRefreshEvent()
{
   if(m_object.valid())
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimEventVisitor visitor(refreshEvent.get());
      m_object->accept(visitor);
   }
}

void ossimGui::AdjustableParameterEditor::adjustmentDescriptionChanged(const QString& value)
{
   if(m_interface)
   {
      m_interface->setDirtyFlag(true);
      m_interface->setAdjustmentDescription(value.toAscii().data());
   }   
}
