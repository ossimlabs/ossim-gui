#ifndef ossimGuiDataManagerPropertyView_HEADER
#define ossimGuiDataManagerPropertyView_HEADER

#include <QtGui/QTreeView>
#include <QtGui/QMouseEvent>
#include <QtGui/QStandardItemModel>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QItemDelegate>
#include <ossimGui/DataManager.h>
#include <ossim/base/ossimPropertyInterface.h>
#include <ossim/base/ossimRefreshEvent.h>

namespace ossimGui
{
   class DataManagerProperty;
   
   class OSSIMGUI_DLL StringChoicePropertyWidget : public QComboBox
   {
      Q_OBJECT
   public:
      StringChoicePropertyWidget(QWidget* parent);
      void setDelegateInformation(DataManagerProperty* prop, const QAbstractItemDelegate* delegate);
      public slots:
      void valueChanged();
      
   protected:
      DataManagerProperty*         m_property;
      const QAbstractItemDelegate* m_delegate;
      
   };
   
   class OSSIMGUI_DLL BooleanPropertyWidget : public QCheckBox
   {
      Q_OBJECT
   public:
      BooleanPropertyWidget(QWidget* parent=0);
      void setDelegateInformation(DataManagerProperty* prop, const QAbstractItemDelegate* delegate);
      
      public slots:
      void valueChanged();
      
   protected:
      DataManagerProperty*         m_property;
      const QAbstractItemDelegate* m_delegate;
   };
   
   class OSSIMGUI_DLL DataManagerPropertyDelegate : public QItemDelegate
   {
   public:
      DataManagerPropertyDelegate(QObject *parent):QItemDelegate(parent){};
      virtual QWidget *	createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
      virtual void	setEditorData ( QWidget * editor, const QModelIndex & index ) const;
      virtual void	setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;
   };

   class OSSIMGUI_DLL DataManagerProperty : public QStandardItem
   {
   public:
      DataManagerProperty():QStandardItem(){}
      DataManagerProperty(const QString& value, ossimRefPtr<ossimProperty> property=ossimRefPtr<ossimProperty>()):QStandardItem(value){setProperty(property.get());}
      virtual void setProperty(ossimRefPtr<ossimProperty> property);
      virtual void populateChildren();
      ossimProperty* property(){return m_property.get();}
      const ossimProperty* property()const{return m_property.get();}
      DataManagerProperty* rootProperty();
   protected:
      void addProperty(ossimRefPtr<ossimProperty> property);
      ossimRefPtr<ossimProperty>  m_property;
   };

   class OSSIMGUI_DLL DataManagerPropertyView : public QTreeView
   {
      Q_OBJECT
   public:
      DataManagerPropertyView(QWidget* parent=0);
      virtual void setObject(ossimObject* obj)
      {
         m_node = obj;
         DataManager::Node* node = dynamic_cast<DataManager::Node*>(obj);
         if(node)
         {
            m_node = node->getObject();
         }

         populateChildren();
      }
      
      virtual void populateChildren();
      void	reloadProperties();
      public slots:
      void	expanded ( const QModelIndex & idx );
      void	collapsed ( const QModelIndex & index );
      
   protected:
      virtual void	mousePressEvent ( QMouseEvent * event );
      virtual void	dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
      ossimPropertyInterface* propertyInterface()
      {
         return dynamic_cast<ossimPropertyInterface*> (m_node.get());
      }
      void fireRefresh(int type = ossimRefreshEvent::REFRESH_PIXELS);
      QStandardItemModel* m_model;
//      ossimRefPtr<DataManager::Node> m_node;
      ossimRefPtr<ossimObject> m_node;
   };
}
#endif
