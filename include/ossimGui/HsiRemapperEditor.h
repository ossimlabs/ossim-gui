#ifndef ossimGuiHsiRemapperEditor_HEADER
#define ossimGuiHsiRemapperEditor_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/ui_HsiRemapperEditor.h>
#include <QtGui/QDialog>
#include <ossim/imaging/ossimHsiRemapper.h>
namespace ossimGui{
   class OSSIMGUI_DLL HsiRemapperEditor: public QDialog, public Ui::HsiRemapperEditor
   {
      Q_OBJECT
   public:
      HsiRemapperEditor(QWidget* parent=0, Qt::WindowFlags f = 0 );
      
      void setObject(ossimObject* obj);

   public slots:
      void hueOffsetChanged(int value);
      void hueLowChange(int value);
      void hueHighChange(int value);
      void hueBlendChange(int value);
      void saturationOffsetChange(int value);
      void intensityOffsetChange(int value);
      void lowIntensityClipChange(int value);
      void highIntensityClipChange(int value);
      void whitObjectClipChange(int value);
      
      void redButtonClicked();
      void yellowButtonClicked();
      void greenButtonClicked();
      void cyanButtonClicked();
      void blueButtonClicked();
      void magentaButtonClicked();
      void allButtonClicked();
      
      void enableButtonClicked(bool checked);
      void resetGroupButtonClicked();
      void resetAllButtonClicked();
      void okButtonClicked();
      void cancelButtonClicked();
      
   protected:
      ossimRefPtr<ossimHsiRemapper> m_object;
      int m_activeGroup;
      double m_hueOffsetRange[2];
      double m_hueLowRange[2];
      double m_hueHighRange[2];
      double m_hueBlendRange[2];
      double m_saturationOffsetRange[2];
      double m_intensityOffsetRange[2];
      double m_lowIntensityClipRange[2];
      double m_highIntensityClipRange[2];
      double m_whiteObjectClipRange[2];
      ossimKeywordlist m_cancelState;
      
      ossimString getPropertyPrefix()const;
      double roundForDisplay(double value, int precision=100)const;
      void fireRefreshEvent();
      void initializeUiValues();
      void initializeUiRanges();
   };
   
}

#endif
