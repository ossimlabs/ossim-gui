#ifndef ossimGuiHistogramRemapperEditor_HEADER
#define ossimGuiHistogramRemapperEditor_HEADER

#include <ossimGui/Export.h>
#include <ossimGui/ui_HistogramRemapperEditor.h>
#include <ossim/imaging/ossimHistogramRemapper.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossim/base/ossimKeywordlist.h>

namespace ossimGui
{
   class OSSIMGUI_DLL HistogramRemapperEditor : public QDialog, public Ui::HistogramRemapperEditor
   {
      Q_OBJECT
   public:
      HistogramRemapperEditor(QWidget* parent=0, Qt::WindowFlags f = 0 );
      void setObject(ossimObject* obj);
      void setHistogram(const ossimFilename& file);
      
   public slots:
      void okButtonClicked(bool);
      void resetButtonClicked(bool);
      void cancelButtonClicked(bool);
      void enableButtonClicked(bool);
      void bandActivated ( int index ); 
      void stretchModeActivated ( int index ); 
      void openHistogramButton(bool);
      
      void clipPenetrationsAdjusted(double minValue, double maxValue);
   protected:
      void initializeUiValues();
      void populateClipPoints();
      void calculateAverageHistogram();
      void fireRefreshEvent();
      
      
      ossimRefPtr<ossimHistogramRemapper> m_histogramRemapper;
      ossimRefPtr<ossimMultiResLevelHistogram> m_multiResHistogram;
      ossimRefPtr<ossimHistogram> m_averageHistogram;
      ossimKeywordlist m_cancelState;
      
   };
}

#endif
