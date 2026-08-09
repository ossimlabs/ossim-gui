#include <ossimGui/AdjustableParameterEditor.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimListenerManager.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMetaObject>
#include <QMouseEvent>
#include <QResizeEvent>
static const int NAME_INDEX       = 0;
static const int LOCK_INDEX       = 1;
static const int SIGMA_INDEX      = 2;
static const int PARAMETER_INDEX  = 3;
static const int SLIDER_INDEX     = 4;
static const int VALUE_INDEX      = 5;

namespace
{
void ensureEditableAdjustment(ossimAdjustableParameterInterface* interface)
{
   if(!interface)
   {
      return;
   }

   if((interface->getNumberOfAdjustments() < 1) ||
      (interface->getNumberOfAdjustableParameters() < 1))
   {
      interface->initAdjustableParameters();
   }
}
}

ossimGui::AdjustableParameterLockHeader::
AdjustableParameterLockHeader(int lockSection, QWidget* parent)
:QHeaderView(Qt::Horizontal, parent),
 m_lockSection(lockSection),
 m_lockState(Qt::Unchecked),
 m_lockControlEnabled(false),
 m_lockCheckBox(new QCheckBox(viewport()))
{
   setToolTip("Check to lock all adjustable parameters; clear to unlock all.");
   m_lockCheckBox->setTristate(true);
   m_lockCheckBox->setToolTip(toolTip());
   m_lockCheckBox->setEnabled(false);
   connect(m_lockCheckBox, &QCheckBox::clicked,
           this, [this](bool checked) {
              emit lockStateRequested(checked);
           });
   connect(this, &QHeaderView::sectionResized,
           this, [this]() { updateLockCheckBoxGeometry(); });
   connect(this, &QHeaderView::sectionMoved,
           this, [this]() { updateLockCheckBoxGeometry(); });
}

void ossimGui::AdjustableParameterLockHeader::setLockState(
   Qt::CheckState state)
{
   if(m_lockState != state)
   {
      m_lockState = state;
      m_lockCheckBox->blockSignals(true);
      m_lockCheckBox->setCheckState(state);
      m_lockCheckBox->blockSignals(false);
   }
}

void ossimGui::AdjustableParameterLockHeader::setLockControlEnabled(
   bool enabled)
{
   if(m_lockControlEnabled != enabled)
   {
      m_lockControlEnabled = enabled;
      m_lockCheckBox->setEnabled(enabled);
   }
}

void ossimGui::AdjustableParameterLockHeader::resizeEvent(
   QResizeEvent* event)
{
   QHeaderView::resizeEvent(event);
   updateLockCheckBoxGeometry();
}

void ossimGui::AdjustableParameterLockHeader::
updateLockCheckBoxGeometry()
{
   const int sectionPosition = sectionViewportPosition(m_lockSection);
   const QSize checkBoxSize = m_lockCheckBox->sizeHint();
   m_lockCheckBox->setGeometry(
      sectionPosition + 4,
      (viewport()->height() - checkBoxSize.height()) / 2,
      checkBoxSize.width(),
      checkBoxSize.height());
   m_lockCheckBox->raise();
}

void ossimGui::AdjustableParameterLockHeader::mousePressEvent(
   QMouseEvent* event)
{
   if(m_lockControlEnabled &&
      logicalIndexAt(event->pos()) == m_lockSection)
   {
      emit lockStateRequested(m_lockState != Qt::Checked);
      event->accept();
      return;
   }
   QHeaderView::mousePressEvent(event);
}

ossimGui::AdjustableParameterEditor::AdjustableParameterEditor(QWidget* parent, Qt::WindowFlags f)
:QDialog(parent, f),
m_interface(0),
m_lockHeader(0),
m_listener(new Listener(this))
{
   setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);
   m_adjustmentSelectionBox->setSizeAdjustPolicy(
      QComboBox::AdjustToMinimumContentsLengthWithIcon);
   m_adjustmentSelectionBox->setMinimumContentsLength(32);
   m_adjustmentSelectionBox->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
   m_lockHeader =
      new AdjustableParameterLockHeader(
         LOCK_INDEX, m_adjustableParameterTable);
   m_adjustableParameterTable->setHorizontalHeader(m_lockHeader);
   QTableWidgetItem* lockHeaderItem =
      m_adjustableParameterTable->horizontalHeaderItem(LOCK_INDEX);
   if(lockHeaderItem)
   {
      lockHeaderItem->setTextAlignment(Qt::AlignRight |
                                       Qt::AlignVCenter);
   }
   m_adjustableParameterTable->setColumnWidth(LOCK_INDEX, 80);
   connect(m_lockHeader,
           SIGNAL(lockStateRequested(bool)),
           this,
           SLOT(setAllParametersLocked(bool)));
   connect(m_adjustableParameterTable, SIGNAL(cellChanged(int, int)), this, SLOT(valueChanged(int, int)));
   connect(m_resetButton, SIGNAL(clicked()), this, SLOT(resetTable()));
   connect(m_modelDefaultsButton, SIGNAL(clicked()), this, SLOT(reloadModelDefaults()));
   connect(m_keepAdjustmentButton, SIGNAL(clicked()), this, SLOT(keepAdjustment()));
   connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveAdjustment()));
   connect(m_closeButton, SIGNAL(clicked()), this,SLOT(close()));
   connect(m_copyAdjustmentButton, SIGNAL(clicked()), this,SLOT(copyAdjustment()));
   connect(m_deleteAdjustmentButton, SIGNAL(clicked()), this,SLOT(deleteAdjustment()));
   connect(m_adjustmentSelectionBox, SIGNAL(activated(const QString&)), this,SLOT(selectionListChanged()));
   connect(m_adjustmentDescriptionInput, SIGNAL(textChanged(const QString&)), this, SLOT(adjustmentDescriptionChanged(const QString&)));
   connect(this, SIGNAL(sourceChanged(const QString&)), this, SLOT(setSource(const QString&)));
}

ossimGui::AdjustableParameterEditor::~AdjustableParameterEditor()
{
   removeObjectListener();
   delete m_listener;
   m_listener = 0;
   m_interface = 0;
   m_object = 0;
}

void ossimGui::AdjustableParameterEditor::Listener::refreshEvent(
   ossimRefreshEvent& event)
{
   if(m_editor &&
      (event.getRefreshType() & ossimRefreshEvent::REFRESH_GEOMETRY))
   {
      QMetaObject::invokeMethod(
         m_editor, "refreshFromObject", Qt::QueuedConnection);
   }
}

void ossimGui::AdjustableParameterEditor::setObject(ossimObject* obj)
{
   removeObjectListener();
   m_object = obj;
   resolveAdjustableInterface();
   addObjectListener();

   ensureEditableAdjustment(m_interface);
   setImageSource();

   transferToDialog();
}

void ossimGui::AdjustableParameterEditor::resolveAdjustableInterface()
{
   m_interface = 0;
   if(m_object.valid())
   {
      m_interface =
         dynamic_cast<ossimAdjustableParameterInterface*>(m_object.get());
      if(!m_interface)
      {
         ossimImageSource* isource =
            dynamic_cast<ossimImageSource*>(m_object.get());
         if(isource)
         {
            ossimRefPtr<ossimImageGeometry> geom = isource->getImageGeometry();
            if(geom.valid())
            {
               m_interface = geom->getAdjustableParameterInterface();
            }
         }
      }
   }
}

void ossimGui::AdjustableParameterEditor::addObjectListener()
{
   ossimListenerManager* manager =
      dynamic_cast<ossimListenerManager*>(m_object.get());
   if(manager && m_listener)
   {
      manager->addListener(m_listener);
   }
}

void ossimGui::AdjustableParameterEditor::removeObjectListener()
{
   ossimListenerManager* manager =
      dynamic_cast<ossimListenerManager*>(m_object.get());
   if(manager && m_listener)
   {
      manager->removeListener(m_listener);
   }
}

void ossimGui::AdjustableParameterEditor::refreshFromObject()
{
   resolveAdjustableInterface();
   ensureEditableAdjustment(m_interface);
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
      for(ossim_uint32 idx = 0; idx < n; ++idx)
      {
         QString description =
            m_interface->getAdjustmentDescription(idx).c_str();
         if(description.trimmed().isEmpty())
         {
            description = tr("(unnamed)");
         }

         const QString label = tr("[%1] %2").arg(idx).arg(description);
         m_adjustmentSelectionBox->addItem(label, idx);
         m_adjustmentSelectionBox->setItemData(
            m_adjustmentSelectionBox->count() - 1,
            label,
            Qt::ToolTipRole);
      }
      ossim_uint32 adjIdx = m_interface->getCurrentAdjustmentIdx();
      const int selectedRow = m_adjustmentSelectionBox->findData(adjIdx);
      m_adjustmentSelectionBox->setCurrentIndex(selectedRow);
      m_adjustmentSelectionBox->setToolTip(
         selectedRow >= 0
            ? m_adjustmentSelectionBox->itemText(selectedRow)
            : QString());
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
      m_adjustableParameterTable->setRowCount(0);
      if(m_lockHeader)
      {
         m_lockHeader->setLockControlEnabled(false);
         m_lockHeader->setLockState(Qt::Unchecked);
      }
      return;
   }
   if(m_interface)
   {
      m_adjustableParameterTable->blockSignals(true);

      int numAdjustables = m_interface->getNumberOfAdjustableParameters();
      if(numAdjustables > 0)
      {
         bool anyLocked = false;
         bool anyUnlocked = false;
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
            bool lockFlag            = m_interface->getParameterLockFlag(idx);
            anyLocked = anyLocked || lockFlag;
            anyUnlocked = anyUnlocked || !lockFlag;
            
            if(!m_adjustableParameterTable->item(idx, NAME_INDEX))
            {
               m_adjustableParameterTable->setItem(idx, NAME_INDEX, new QTableWidgetItem(description.c_str()));
            }
            else 
            {
               m_adjustableParameterTable->item(idx, NAME_INDEX)->setText(description.c_str());
            }

            AdjustableParameterLockCheckBox* lockBox = 0;
            if(!m_adjustableParameterTable->cellWidget(idx, LOCK_INDEX))
            {
               lockBox = new AdjustableParameterLockCheckBox(idx, LOCK_INDEX);
               connect(lockBox, SIGNAL(parameterChanged(int, int)), this, SLOT(valueChanged(int, int)));
               m_adjustableParameterTable->setCellWidget(idx, LOCK_INDEX, lockBox);
            }
            else
            {
               lockBox = dynamic_cast<AdjustableParameterLockCheckBox*>(m_adjustableParameterTable->cellWidget(idx, LOCK_INDEX));
            }

            if(lockBox)
            {
               lockBox->blockSignals(true);
               lockBox->setChecked(lockFlag);
               lockBox->blockSignals(false);
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
         if(m_lockHeader)
         {
            m_lockHeader->setLockControlEnabled(true);
            m_lockHeader->setLockState(
               anyLocked && anyUnlocked ?
                  Qt::PartiallyChecked :
                  (anyLocked ? Qt::Checked : Qt::Unchecked));
         }
      }
      else 
      {
         m_adjustableParameterTable->clearContents();
         m_adjustableParameterTable->setRowCount(0);
         if(m_lockHeader)
         {
            m_lockHeader->setLockControlEnabled(false);
            m_lockHeader->setLockState(Qt::Unchecked);
         }
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

void ossimGui::AdjustableParameterEditor::reloadModelDefaults()
{
   if(!m_interface) return;

   m_interface->removeAllAdjustments();
   m_interface->initAdjustableParameters();
   ensureEditableAdjustment(m_interface);
   m_interface->setDirtyFlag(true);
   transferToDialog();
   fireRefreshEvent();
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
         m_filename = file.toStdString();
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
      if(m_interface->getNumberOfAdjustments() <= 1)
      {
         reloadModelDefaults();
         return;
      }
      else
      {
         m_interface->eraseAdjustment(true);
      }
      ensureEditableAdjustment(m_interface);
      m_interface->setDirtyFlag(true);
      transferToDialog();
      fireRefreshEvent();
   }   
}
void ossimGui::AdjustableParameterEditor::selectionListChanged()
{
   if(m_interface)
   {
      bool validIndex = false;
      const ossim_uint32 idx =
         m_adjustmentSelectionBox->currentData().toUInt(&validIndex);
      if(validIndex && idx < m_interface->getNumberOfAdjustments())
      {
         m_interface->setDirtyFlag(true);
         m_interface->setCurrentAdjustment(idx, true);
         transferToDialog();
         fireRefreshEvent();
      }
   }
}

void ossimGui::AdjustableParameterEditor::setAllParametersLocked(bool locked)
{
   if(!m_interface ||
      m_interface->getNumberOfAdjustableParameters() < 1)
   {
      return;
   }

   m_interface->setDirtyFlag(true);
   if(locked)
      m_interface->lockAllParametersCurrentAdjustment();
   else
      m_interface->unlockAllParametersCurrentAdjustment();
   transferToTable();
   fireRefreshEvent();
}

void ossimGui::AdjustableParameterEditor::valueChanged(int row, int col)
{
   if(!m_interface) return;

   m_adjustableParameterTable->blockSignals(true);

   if(col == LOCK_INDEX)
   {
      AdjustableParameterLockCheckBox* lockBox =
         dynamic_cast<AdjustableParameterLockCheckBox*>(m_adjustableParameterTable->cellWidget(row, LOCK_INDEX));
      if(lockBox)
      {
         m_interface->setDirtyFlag(true);
         m_interface->setParameterLockFlag(row, lockBox->isChecked());
         fireRefreshEvent();
         transferToTable();
      }
   }
   else if(col == SLIDER_INDEX)
   {
      QSlider* slider = (QSlider*)m_adjustableParameterTable->cellWidget(row, col);
      if(slider)
      {
         m_interface->setDirtyFlag(true);

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
   }
   else if(col == SIGMA_INDEX)
   {
      m_interface->setDirtyFlag(true);
      ossimString sigma = m_adjustableParameterTable->item(row,col)->text().toStdString();
      m_interface->setParameterSigma(row, sigma.toDouble(), true);
      fireRefreshEvent();
      transferToTable();
   }
   else if(col == PARAMETER_INDEX)
   {
      m_interface->setDirtyFlag(true);
      ossimString param = m_adjustableParameterTable->item(row,col)->text().toStdString();
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
      double value    = ossimString(m_adjustableParameterTable->item(row,col)->text().toStdString()).toDouble();
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
      m_interface->setAdjustmentDescription(value.toStdString());

      const int selectedRow = m_adjustmentSelectionBox->currentIndex();
      if(selectedRow >= 0)
      {
         QString description = value;
         if(description.trimmed().isEmpty())
         {
            description = tr("(unnamed)");
         }
         const ossim_uint32 adjustmentIndex =
            m_adjustmentSelectionBox->itemData(selectedRow).toUInt();
         const QString label =
            tr("[%1] %2").arg(adjustmentIndex).arg(description);
         m_adjustmentSelectionBox->setItemText(selectedRow, label);
         m_adjustmentSelectionBox->setItemData(
            selectedRow, label, Qt::ToolTipRole);
         m_adjustmentSelectionBox->setToolTip(label);
      }
   }   
}
