#ifndef ossimGuiMultiImageDialog_HEADER
#define ossimGuiMultiImageDialog_HEADER
#include <ossimGui/DataManager.h>
#include <ossimGui/Export.h>
#include <ossimGui/ui_MultiImageDialog.h>
#include <QtGui/QDialog>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimProcessListener.h>

namespace ossimGui
{
   class RegistrationOverlay;
   class MetricOverlay;
   class RegPoint;

   class OSSIMGUI_DLL MultiImageDialog : public QDialog, public Ui::MultiImageDialog
   {
      Q_OBJECT

   public:
      MultiImageDialog(QWidget* widget);
      void initDialog();
      void initContent(DataManager::NodeListType& nodeList, const bool& amDialogAvailable);

      void setImgList(const vector<ossimString>& ilist, const vector<ossimString>& tlist);
      void setPtTable(const int& nPts);
      ossimString getCurrentId()const {return ossimString::toString(m_currentIdCounter);}
      ossim_uint32 getNumObs()const {return m_pointTable->columnCount();}
      ossimString getIdByIndex(const ossim_uint32& index);
      bool isActive()const {return m_isActive;}
      void setMode(const int& expMode);
      void updateCurrentIdField();

   public slots:
      void resetContent();
      void setImagePointActive(const ossimString&);
      void setImagePointInactive(const ossimString&);
      void setImagePointRemoved(const ossimString&);
      void setPointPositionContent(const ossimString&);
      void setRegistrationReportContent(const ossimString&);

      // Button click slots
      void autoMeas();
      void addObsPoint();
      void registerImages();
      void dropPoint();
      void clearPoint();
      void acceptReg();
      void resetReg();

      // Point table slots
      void setPointCellClicked(int, int);
      void setPointRowClicked(int);
      void setPointColClicked(int);
      void displayPointTableContextMenuRow(QPoint);
      void displayPointTableContextMenuCol(QPoint);

      // Image table slots
      void displayImageTableContextMenu(QPoint);

      void displayClosing(QObject* obj);

   signals:
      void registrationExecuted(DataManager::NodeListType&);
      void pointDropExecuted(DataManager::NodeListType&);
      void autoMeasInitiated(DataManager::NodeListType&);
      void syncExecuted(ossimGui::RegPoint*, ossimRefPtr<DataManager::Node>);
      void resetModeExecuted(DataManager::NodeListType&);
      void clearPointExecuted(DataManager::NodeListType&);
      void acceptRegExecuted(DataManager::NodeListType&);
      void resetRegExecuted(DataManager::NodeListType&);

   protected:
      bool m_isActive;
      std::vector<ossimGui::RegistrationOverlay*> m_overlaysReg;
      std::vector<ossimGui::MetricOverlay*> m_overlaysMet;
      DataManager::NodeListType m_nodeList;
      DataManager::ExploitationModeType m_exploitationMode;

      bool getRowColMeasPoint(const ossimString& id,
                              ossimGui::RegistrationOverlay* ov,
                              ossim_uint32& row,
                              ossim_uint32& col);

      ossim_uint32 m_currentIdCounter;
   };
}   
#endif
