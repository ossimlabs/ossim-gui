#ifndef ossimGuiExportImageDialog_HEADER
#define ossimGuiExportImageDialog_HEADER
#include <ossimGui/Export.h>
#include <ossimGui/ui_ExportImageDialog.h>
#include <QtGui/QDialog>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/base/ossimProcessListener.h>

namespace ossimGui
{
   class OSSIMGUI_DLL ExportImageDialog :public QDialog, public Ui::ExportImageDialog
   {
      Q_OBJECT
   public:
      ExportImageDialog(QWidget* widget);
      
      void setObject(ossimObject* obj);
      
   public slots:
      void exportAbortClicked(bool);   
      void closeClicked(bool);  
      void fileTypeActivated(int);
      void openFileSaveDialog();
      
   protected:
      void populateFileTypes();
      void populatePropertyView();
      void populateGeneralInformation();
      ossimRefPtr<ossimConnectableObject> m_connectable;
      ossimRefPtr<ossimImageFileWriter>   m_writer;
      bool                                m_exportingFlag;
   };
}   
#endif
