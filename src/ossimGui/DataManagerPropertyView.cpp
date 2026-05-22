#include <ossimGui/DataManagerPropertyView.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimNumericProperty.h>

namespace ossimGui
{
   StringChoicePropertyWidget::StringChoicePropertyWidget(QWidget* parent)
   :QComboBox(parent),m_property(0), m_delegate(0)
   {
      setAutoFillBackground(true);
   }
   
   void StringChoicePropertyWidget::setDelegateInformation(DataManagerProperty* prop, const QAbstractItemDelegate* delegate)
   {
      m_property = prop;
      m_delegate = delegate;
   }
   
   void StringChoicePropertyWidget::valueChanged()
   {
      if(m_property&&m_delegate)
      {
         m_delegate->setModelData ( this, m_property->model(), m_property->index() ); 
      }
   }
   
   
   BooleanPropertyWidget::BooleanPropertyWidget(QWidget* parent)
   :QCheckBox(parent),m_property(0), m_delegate(0)
   {
      setAutoFillBackground(true);
   }
   
   void BooleanPropertyWidget::valueChanged()
   {
      if(m_property&&m_delegate)
      {
         m_delegate->setModelData ( this, m_property->model(), m_property->index() ); 
         //      m_property->model()->blockSignals(true);
         //      m_property->setData(QVariant(QString()), Qt::DisplayRole);
         //      m_property->model()->blockSignals(false);
      }
   }
   
   void BooleanPropertyWidget::setDelegateInformation(DataManagerProperty* prop, 
                                                                const QAbstractItemDelegate* delegate)
   {
      m_property = prop;
      m_delegate = delegate;
   }
   
   void	DataManagerPropertyDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const
   {
      QStandardItemModel* itemModel = (QStandardItemModel*)index.model();
      DataManagerProperty* propertyItem = dynamic_cast<DataManagerProperty*>(itemModel->itemFromIndex(index));
      if(!propertyItem)
      {
         QItemDelegate::setEditorData(editor, index);
         return;
      }

      if(dynamic_cast<BooleanPropertyWidget*> (editor))
      {
         BooleanPropertyWidget* w = dynamic_cast<BooleanPropertyWidget*> (editor);
         if(w)
         {
            if(propertyItem->property())
            {
               w->setProperty("checked", QVariant(propertyItem->property()->valueToString().toBool()));
            }
         }
      }
      else if(dynamic_cast<StringChoicePropertyWidget*> (editor))
      {
         StringChoicePropertyWidget* w = dynamic_cast<StringChoicePropertyWidget*> (editor);
         if(w)
         {
            if(propertyItem->property())
            {
               w->setProperty("currentText", QVariant(QString(propertyItem->property()->valueToString().c_str())));
            }
         }
      }
      else 
      {
         QItemDelegate::setEditorData(editor, index);
      }
      
   }
   
   void	DataManagerPropertyDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
   {
      // QStandardItemModel* itemModel = (QStandardItemModel*)model;
      // DataManagerProperty* propertyItem = dynamic_cast<DataManagerProperty*>(itemModel->itemFromIndex(index));
      if(dynamic_cast<BooleanPropertyWidget*> (editor))
      {
         BooleanPropertyWidget* w = dynamic_cast<BooleanPropertyWidget*> (editor);
         const QVariant value(w->isChecked());
         model->setData(index, value, Qt::UserRole);
         model->setData(index, QVariant(QString()), Qt::DisplayRole);
      }
      else if(dynamic_cast<StringChoicePropertyWidget*> (editor))
      {
         StringChoicePropertyWidget* w = dynamic_cast<StringChoicePropertyWidget*> (editor);
         const QVariant value = w->itemData(w->currentIndex()).isValid() ?
            w->itemData(w->currentIndex()) :
            QVariant(w->currentText());
         model->setData(index, value, Qt::UserRole);
         model->setData(index, QVariant(QString()), Qt::DisplayRole);
      }
      else 
      {
         QItemDelegate::setModelData(editor, model, index);
      }
      
   }
   
   QWidget* DataManagerPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
   {
      QWidget* result = 0;
      if(index.column() == 1)
      {
         DataManagerProperty* itemProperty = dynamic_cast<DataManagerProperty*> (((QStandardItemModel*)index.model())->itemFromIndex(index));
         if(itemProperty)
         {
            ossimRefPtr<ossimProperty> property = itemProperty->property();
            if(property.valid())
            {
               if(dynamic_cast<ossimBooleanProperty*>(property.get()))
               {
                  BooleanPropertyWidget* checkbox = new BooleanPropertyWidget(parent);
                  checkbox->setDelegateInformation(itemProperty, this);
                  checkbox->setChecked(property->valueToString().toBool());
                  
                  result = checkbox;
                  connect(checkbox,
                          &QCheckBox::clicked,
                          const_cast<DataManagerPropertyDelegate*>(this),
                          [this, checkbox]()
                          {
                             DataManagerPropertyDelegate* self =
                                const_cast<DataManagerPropertyDelegate*>(this);
                             emit self->commitData(checkbox);
                             emit self->closeEditor(checkbox);
                          });
                  itemProperty->setEditorActive(true);
                  connect(result,
                          &QObject::destroyed,
                          [itemProperty]()
                          {
                             if(itemProperty && itemProperty->model())
                             {
                                itemProperty->setEditorActive(false);
                                itemProperty->model()->blockSignals(true);
                                itemProperty->setData(QVariant(),
                                                      Qt::UserRole);
                                itemProperty->setData(
                                   QVariant(itemProperty->property() ?
                                      itemProperty->property()->
                                         valueToString().c_str() :
                                      ""),
                                   Qt::DisplayRole);
                                itemProperty->model()->blockSignals(false);
                             }
                          });
                  result->setFocusPolicy(Qt::StrongFocus);
                  itemProperty->model()->blockSignals(true);
                  itemProperty->setData(QVariant(QString()), Qt::DisplayRole);
                  itemProperty->model()->blockSignals(false);
               }
               else if(dynamic_cast<ossimStringProperty*> (property.get()))
               {
                  ossimStringProperty* stringProperty = dynamic_cast<ossimStringProperty*> (property.get());
                  if(stringProperty->hasConstraints())
                  {
                     StringChoicePropertyWidget* stringChoice = new StringChoicePropertyWidget(parent);
                     const std::vector<ossimString>& constraints = stringProperty->getConstraints();
                     ossim_uint32 idx = 0;
                     ossim_uint32 currentIdx = 0;
                     ossimString value = property->valueToString();
                     for(idx = 0; idx < constraints.size(); ++idx)
                     {
                        if(value == constraints[idx]) currentIdx = idx;
                        stringChoice->addItem(QString(tr(constraints[idx].c_str())), QVariant(constraints[idx].c_str()));
                     }
                     stringChoice->setCurrentIndex(currentIdx);
                     stringChoice->setDelegateInformation(itemProperty, this);
                     result = stringChoice;
                     connect(stringChoice,
                             static_cast<void (QComboBox::*)(int)>(
                                &QComboBox::activated),
                             const_cast<DataManagerPropertyDelegate*>(this),
                             [this, stringChoice](int)
                             {
                                DataManagerPropertyDelegate* self =
                                   const_cast<DataManagerPropertyDelegate*>(
                                      this);
                                emit self->commitData(stringChoice);
                                emit self->closeEditor(stringChoice);
                             });
                     itemProperty->setEditorActive(true);
                     connect(result,
                             &QObject::destroyed,
                             [itemProperty]()
                             {
                                if(itemProperty && itemProperty->model())
                                {
                                  itemProperty->setEditorActive(false);
                                  itemProperty->model()->blockSignals(true);
                                  itemProperty->setData(QVariant(),
                                                        Qt::UserRole);
                                  itemProperty->setData(
                                     QVariant(itemProperty->property() ?
                                        itemProperty->property()->
                                            valueToString().c_str() :
                                         ""),
                                      Qt::DisplayRole);
                                   itemProperty->model()->blockSignals(false);
                                }
                             });
                     result->setFocusPolicy(Qt::StrongFocus);
                     itemProperty->model()->blockSignals(true);
                     itemProperty->setData(QVariant(QString()), Qt::DisplayRole);
                     itemProperty->model()->blockSignals(false);
                  }
                  else 
                  {
                     result =  QItemDelegate::createEditor(parent, option, index);
                  }
                  
               }
               else if(dynamic_cast<ossimNumericProperty*> (property.get()))
               {
                  result =  QItemDelegate::createEditor(parent, option, index);
               }
               else 
               {
                  result =  QItemDelegate::createEditor(parent, option, index);
               }
            }
         }
         else
         {
            result =  QItemDelegate::createEditor(parent, option, index);
         }
         
      }
      
      return result;
   }
   
   void DataManagerProperty::setProperty(ossimRefPtr<ossimProperty> property)
   {
      m_property = property.get();
      
      if(m_property.valid())
      {
         setEditable(!m_property->isReadOnly());
      }
      if(column() != 1)
      {
         setEditable(false);
      }
   }
   
   
   void DataManagerProperty::populateChildren()
   {
      ossimContainerProperty* container = dynamic_cast<ossimContainerProperty*> (m_property.get());
      // QStandardItemModel* itemModel = (QStandardItemModel*)index().model();
      if(container)
      {
         std::vector<ossimRefPtr<ossimProperty> > currentProperties;
         container->getPropertyList(currentProperties);
         ossim_uint32 idx = 0;
         for(idx = 0; idx < currentProperties.size(); ++idx)
         {
            DataManagerProperty* nameItem = new DataManagerProperty(currentProperties[idx]->getName().c_str());
            DataManagerProperty* valueItem = new DataManagerProperty(currentProperties[idx]->valueToString().c_str(), currentProperties[idx]);
            QList<QStandardItem*> items;
            items.push_back(nameItem);
            items.push_back(valueItem);
            appendRow(items);
            nameItem->setProperty(currentProperties[idx].get());
            valueItem->setProperty(currentProperties[idx].get());
            nameItem->populateChildren();
         }
      }
   }
   
   DataManagerProperty* DataManagerProperty::rootProperty()
   {
      DataManagerProperty* current = this;
      DataManagerProperty* parentItem = dynamic_cast<DataManagerProperty*>(current->parent());
      while(parentItem)
      {
         parentItem = dynamic_cast<DataManagerProperty*>(current->parent());
         if(parentItem) current = parentItem;
      }
      return current;
   }

   DataManagerPropertyView::DataManagerPropertyView(QWidget* parent):QTreeView(parent)
   {
      QList<QString> labels;
      labels.push_back("Name");
      labels.push_back("Value");
      m_model= new QStandardItemModel();
      m_model->setColumnCount(2);
      m_model->setHorizontalHeaderLabels(labels);
      setModel(m_model);
      setFrameStyle(QFrame::NoFrame);
      setAttribute(Qt::WA_MacShowFocusRect, false);
      setAlternatingRowColors(true);
      setItemDelegateForColumn(1, new DataManagerPropertyDelegate(this));
      setEditTriggers(QAbstractItemView::AllEditTriggers);
      connect(m_model,
              SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
              this,
              SLOT(commitModelDataChanged(const QModelIndex&,
                                          const QModelIndex&)));
      connect(this, SIGNAL(expanded ( const QModelIndex &)), this, SLOT(expanded ( const QModelIndex &)));
      connect(this, SIGNAL(collapsed ( const QModelIndex &)), this, SLOT(collapsed ( const QModelIndex &)));
   }
   
   void DataManagerPropertyView::populateChildren()
   {
      blockSignals(true);
      QList<QString> labels;
      labels.push_back("Name");
      labels.push_back("Value");
      m_model->clear();
      m_model->setHorizontalHeaderLabels(labels);
      if(m_node.valid())
      {
         ossimPropertyInterface* propInterface = propertyInterface();
         if(propInterface)
         {
            std::vector<ossimRefPtr<ossimProperty> > currentProperties;
            propInterface->getPropertyList(currentProperties);
            ossim_uint32 idx = 0;
            for(idx = 0; idx < currentProperties.size(); ++idx)
            {
               DataManagerProperty* nameItem = new DataManagerProperty(currentProperties[idx]->getName().c_str());
               DataManagerProperty* valueItem = new DataManagerProperty(currentProperties[idx]->valueToString().c_str(), currentProperties[idx]);
               QList<QStandardItem*> items;
               items.push_back(nameItem);
               items.push_back(valueItem);
               m_model->blockSignals(true);
               m_model->appendRow(items);
               nameItem->setProperty(currentProperties[idx].get());
               valueItem->setProperty(currentProperties[idx].get());
               nameItem->populateChildren();
               m_model->blockSignals(false);
            }
         }
      }
      resizeColumnToContents(0);

      blockSignals(false);
   }
   
   void	DataManagerPropertyView::reloadProperties()
   {
      
   }

   void DataManagerPropertyView::commitModelDataChanged(
      const QModelIndex& topLeft,
      const QModelIndex& bottomRight)
   {
      dataChanged(topLeft, bottomRight);
   }
   
   void	DataManagerPropertyView::dataChanged( const QModelIndex & topLeft, const QModelIndex & /* bottomRight */)
   {
      if((topLeft.column() == 1)&&m_node.valid()&&propertyInterface())
      {
         QStandardItemModel* itemModel = (QStandardItemModel*)topLeft.model();
         DataManagerProperty* propertyItem = dynamic_cast<DataManagerProperty*>(itemModel->itemFromIndex(topLeft));
         if(propertyItem)
         {
            ossimRefPtr<ossimProperty> property = propertyItem->property();
            if(property.valid())
            {
               QVariant v = propertyItem->data(Qt::UserRole);
               if(!v.isValid())
                  v = propertyItem->data(Qt::EditRole);
               if(v.isValid())
               {
                  property->setValue(v.toString().toStdString());
#if 0 /* warning C4065: switch statement contains 'default' but no 'case' labels (drb) */
                  switch(v.type())
                  {
                     default:
                     {
                        property->setValue(v.toString().toAscii().data());
                        break;
                     }
                  }
#endif
                  
                  DataManagerProperty* rootItem = propertyItem->rootProperty();
                  if(rootItem)
                  {
                     if(propertyInterface())
                     {
                        propertyInterface()->setProperty(rootItem->property());
                        ossimRefPtr<ossimProperty> acceptedProperty =
                           propertyInterface()->getProperty(
                              property->getName());
                        if(acceptedProperty.valid() &&
                           !propertyItem->editorActive())
                        {
                           propertyItem->setProperty(acceptedProperty.get());
                           m_model->blockSignals(true);
                           propertyItem->setData(QVariant(), Qt::UserRole);
                           propertyItem->setData(
                              QVariant(acceptedProperty->valueToString().c_str()),
                              Qt::EditRole);
                           propertyItem->setData(
                              QVariant(acceptedProperty->valueToString().c_str()),
                              Qt::DisplayRole);
                           m_model->blockSignals(false);
                        }
                     }
                  }
                  if(property->isCacheRefresh())
                  {
                     fireRefresh(ossimRefreshEvent::REFRESH_PIXELS);
                  }
                  else if(property->isFullRefresh())
                  {
                     fireRefresh(ossimRefreshEvent::REFRESH_GEOMETRY);
                  }
                  if(property->affectsOthers())
                  {
                     reloadProperties();
                  }
               }
            }
         }
      }
   }
   
   void	DataManagerPropertyView::mousePressEvent ( QMouseEvent * event )
   {
      QTreeView::mousePressEvent(event);
   }
   
   void DataManagerPropertyView::fireRefresh(int type)
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(static_cast<ossimRefreshEvent::RefreshType>(type));
      ossimEventVisitor visitor(refreshEvent.get());
      if(m_node.valid())
      {
         m_node->accept(visitor);
      }
   }
   
   
   void	DataManagerPropertyView::expanded (const QModelIndex& /* idx */)
   {
      resizeColumnToContents(0);
   }
   
   void	DataManagerPropertyView::collapsed (const QModelIndex& /* idx */)
   {
      resizeColumnToContents(0);
   }
} // end namespace ossimGui
