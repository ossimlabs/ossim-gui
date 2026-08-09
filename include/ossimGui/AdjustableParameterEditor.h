#ifndef ossimGuiAdjustableParameterEditor_HEADER
#define ossimGuiAdjustableParameterEditor_HEADER
#include <ui_AdjustableParameterEditor.h>
// #include <QtGui/QDialog>
#include <QCheckBox>
#include <QDialog>
#include <QHeaderView>
#include <QSlider>
#include <ossimGui/Export.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimAdjustableParameterInterface.h>
#include <ossim/base/ossimFilename.h>
class QMouseEvent;
class QResizeEvent;
namespace ossimGui
{
   class AdjustableParameterLockHeader : public QHeaderView
   {
      Q_OBJECT
   public:
      AdjustableParameterLockHeader(int lockSection, QWidget* parent = 0);
      void setLockState(Qt::CheckState state);
      void setLockControlEnabled(bool enabled);

   signals:
      void lockStateRequested(bool locked);

   protected:
      void mousePressEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void updateLockCheckBoxGeometry();

      int m_lockSection;
      Qt::CheckState m_lockState;
      bool m_lockControlEnabled;
      QCheckBox* m_lockCheckBox;
   };

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
      ~AdjustableParameterEditor() override;
      
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
      void setAllParametersLocked(bool locked);
      void adjustmentDescriptionChanged(const QString&);
      void setSource(const QString&);
      void refreshFromObject();
      
   protected:
      class Listener : public ossimConnectableObjectListener
      {
      public:
         explicit Listener(AdjustableParameterEditor* editor)
         :m_editor(editor)
         {
         }

         void refreshEvent(ossimRefreshEvent& event) override;

      private:
         AdjustableParameterEditor* m_editor;
      };

      ossimFilename findDefaultFilename();
      void addObjectListener();
      void removeObjectListener();
      void resolveAdjustableInterface();
      void transferToDialog();
      void transferToTable();
      void transferToList();
      void fireRefreshEvent();
      
      
      ossimRefPtr<ossimObject>            m_object;
      ossimAdjustableParameterInterface*  m_interface;
      ossimFilename                       m_filename;
      AdjustableParameterLockHeader*      m_lockHeader;
      Listener*                           m_listener;
   };
}

#endif
