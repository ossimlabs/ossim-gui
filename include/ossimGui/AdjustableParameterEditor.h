#ifndef ossimGuiAdjustableParameterEditor_HEADER
#define ossimGuiAdjustableParameterEditor_HEADER
#include <ossimGui/ui_AdjustableParameterEditor.h>
#include <QtGui/QDialog>
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
   
   class OSSIMGUI_DLL AdjustableParameterEditor : public QDialog, public Ui::AdjustableParameterEditor
   {
      Q_OBJECT
   public:
      AdjustableParameterEditor(QWidget* parent=0, Qt::WindowFlags f = 0 );
      
      void setObject(ossimObject* obj);
      void setImageSource();
    
   signals:
      void sourceChanged(const QString&);

   public slots:
      void valueChanged(int row, int col);
      void resetTable();
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
