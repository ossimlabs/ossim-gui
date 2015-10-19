#ifndef ossimGuiBandSelectorEditor_HEADER
#define ossimGuiBandSelectorEditor_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/ui_BandSelectorEditor.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimConnectableObject.h>
#include <QtGui/QDialog>

namespace ossimGui {

   class OSSIMGUI_DLL BandSelectorEditor: public QDialog, public Ui::BandSelectorEditor
   {
      Q_OBJECT
   public:
      enum SelectionType 
      {
         SINGLE_BAND = 0,
         THREE_BAND  = 1,
         N_BAND      = 2
      };
      BandSelectorEditor(QWidget* parent=0, Qt::WindowFlags f = 0 );
      void setObject(ossimObject* obj);

   protected slots:
      void oneBandButtonClicked(bool checked=false);
      void threeBandButtonClicked(bool checked=false);
      void nBandButtonClicked(bool checked=false);
      void clearBandInputButtonClicked(bool checked=false);
      void inputBandListClicked(const QModelIndex & index);
      void enableButtonClicked(bool checked=false);
      void resetButtonClicked(bool checked=false);
      void okButtonClicked(bool checked=false);
      void cancelButtonClicked(bool checked=false);
      
   protected:
      void initializeUiValues();
      void setCurrentBandsToObject();
      void fireRefreshEvent();
      ossim_uint32 getNumberOfInputBands()const;
      void setBandInput();
      
      ossimRefPtr<ossimConnectableObject> m_object;
      ossimKeywordlist m_cancelState;
      SelectionType m_selectionType;
      ossim_uint32 m_currentIndex;
      std::vector<ossim_uint32> m_currentBandList;
      
   };
}
#endif
