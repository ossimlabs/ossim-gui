#ifndef ossimGuiAutoMeasurementDialog_HEADER
#define ossimGuiAutoMeasurementDialog_HEADER
#include <ossimGui/DataManager.h>
#include <ossimGui/Export.h>
#include <ossimGui/ui_AutoMeasurementDialog.h>
#include <QtGui/QDialog>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimProcessListener.h>
#include <ossim/base/ossimTieMeasurementGeneratorInterface.h>



namespace ossimGui
{
   class RegPoint;
   class ImageScrollView;

   class OSSIMGUI_DLL AutoMeasurementDialog : public QDialog, public Ui::AutoMeasurementDialog
   {
      Q_OBJECT

   public:
      AutoMeasurementDialog(QWidget* widget,
                            DataManager::NodeListType& nodeList,
                            ossimTieMeasurementGeneratorInterface* tGen);

   public slots:
      void setMeasurementReportContent(const ossimString&);

      // Button click slots
      void execMeas();
      void acceptMeas();
      void dismissMeas();
      void resetMeas();

      // Combo box slots
      void selectDetector(QString);
      void selectExtractor(QString);
      void selectMatcher(QString);

      // Grid definition slots
      void setUseGridChecked(bool);
      void setGridSizeX(int);
      void setGridSizeY(int);
      void setMaxMatches(int);

      // Initial bounding box slot
      void setBox(ImageScrollView*, const ossimDpt&, const ossimDpt&);


      void displayClosing(QObject* obj);

   signals:
      void acceptMeasExecuted(DataManager::NodeListType&);
      void dismissMeasExecuted();

   protected:
      DataManager::NodeListType m_nodeList;
      std::vector<ossimGui::ImageScrollView*> m_scrollViews;
      std::vector<ossimIrect> m_roiRects;

      ossimTieMeasurementGeneratorInterface* m_tGen;

      ostringstream m_report;

      void initDialog();
      void initContent();
      void updateCurrentAlgorithmFields();
   };
}   
#endif
