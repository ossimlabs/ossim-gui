#ifndef ossimQtHistogramWidget_HEADER
#define ossimQtHistogramWidget_HEADER

#include <QtGui/QGraphicsView>
#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimMultiBandHistogram.h>
#include <ossim/base/ossimMultiResLevelHistogram.h>
#include <ossimGui/Export.h>

namespace ossimGui{
   
   class OSSIMGUI_DLL HistogramWidget : public QGraphicsView
   {
      Q_OBJECT
      
   public:
      HistogramWidget(QWidget* parent = 0);//,
                             //Qt::WindowFlags f = 0);
      
      virtual ~HistogramWidget();
      
      /*!
       * We need a pointer to the remapper as it's the only one who can map
       * it's bands to the histogram file bands.
       */
      void setHistogram(ossimHistogram* histogram);
      
      void setPenetration(double minValue, double maxValue);
      void setClipPoints(double minValue, double maxValue);
      void getClipPoints(double& minValue, double& maxValue);
      
      void setReadOnly(bool flag){m_readOnly = flag;}
      void setShowClipPointsFlag(bool flag);
      
      virtual void paintEvent(QPaintEvent *event);
      virtual void	resizeEvent ( QResizeEvent * event ); 
protected:

   signals:
      void clipPenetrationsAdjusted(double minValue, double maxValue);
      
   private:
      void mousePressEvent(QMouseEvent *e);
      void mouseMoveEvent(QMouseEvent *e);
      void adjustClips(const QPoint& point);
      /*!
       * A band of zero tells paintHistogram to paint average of all bands...
       */
      double getMaxBinCount();
      void updateScaledHistogram();
      ossimRefPtr<ossimHistogram> m_histogram;
      ossimRefPtr<ossimHistogram> m_scaledHistogram;
      ossim_float64               m_scaledNormalizer;
      bool                        m_showClipPointsFlag;
      double                      m_minFraction;
      double                      m_maxFraction;
      bool                        m_readOnly;
   };
}

#endif /* #ifndef ossimQtHistogramWidget_HEADER */
