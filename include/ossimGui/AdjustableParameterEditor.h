#ifndef ossimGuiAdjustableParameterEditor_HEADER
#define ossimGuiAdjustableParameterEditor_HEADER
#include <ui_AdjustableParameterEditor.h>
// #include <QtGui/QDialog>
#include <QCheckBox>
#include <QDialog>
#include <QSlider>
#include <ossimGui/Export.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimAdjustableParameterInterface.h>
#include <ossim/base/ossimFilename.h>
namespace ossimGui
{
   class AdjustableParameterSlider : public QSlider
   {
      Q_OBJECT
   public:
      AdjustableParameterSlider(int row, int col)
      :QSlider(Qt::Horizontal),
      m_row(row),
      m_col(col)
      {
         connect(this, SIGNAL(valueChanged(int)), SLOT(valueChanged(int)));
      }
    
   signals:
      void parameterChanged(int rowIdx, int colIdx);
      
      
   public slots:
      void valueChanged(int /* value */)
      {
         emit parameterChanged(m_row, m_col);
      }
   protected:
      int m_row;
      int m_col;
   };

   class AdjustableParameterLockCheckBox : public QCheckBox
   {
      Q_OBJECT
   public:
      AdjustableParameterLockCheckBox(int row, int col)
      :QCheckBox(),
      m_row(row),
      m_col(col)
      {
         connect(this, SIGNAL(toggled(bool)), SLOT(valueChanged(bool)));
      }

   signals:
      void parameterChanged(int rowIdx, int colIdx);

   public slots:
      void valueChanged(bool /* value */)
      {
         emit parameterChanged(m_row, m_col);
      }

   protected:
      int m_row;
      int m_col;
   };
   
   class OSSIMGUI_DLL AdjustableParameterEditor : public QDialog, public Ui::AdjustableParameterEditor
   {
      Q_OBJECT
   public:
      AdjustableParameterEditor(QWidget* parent=nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
      
      void setObject(ossimObject* obj);
      void setImageSource();
    
   signals:
      void sourceChanged(const QString&);

   public slots:
      void valueChanged(int row, int col);
      void resetTable();
      void reloadModelDefaults();
      void keepAdjustment();
      void saveAdjustment();
      void copyAdjustment();
      void deleteAdjustment();
      void selectionListChanged();
      void adjustmentDescriptionChanged(const QString&);
      void setSource(const QString&);
      
   protected:
      ossimFilename findDefaultFilename();
      void transferToDialog();
      void transferToTable();
      void transferToList();
      void fireRefreshEvent();
      
      
      ossimRefPtr<ossimObject>            m_object;
      ossimAdjustableParameterInterface*  m_interface;
      ossimFilename                       m_filename;
   };
}

#endif
