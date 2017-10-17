#ifndef ossimGuiMainWindow_HEADER
#define ossimGuiMainWindow_HEADER
#include <ui_MainWindow.h>
#include <QMainWindow>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <ossimGui/Export.h>
#include <ossimGui/DataManager.h>
#include <ossim/parallel/ossimJobMultiThreadQueue.h>
#include <ossimGui/DisplayTimerJobQueue.h>

class QMdiArea;
namespace ossimGui
{
   class ImageScrollWidget;
   class ImageMdiSubWindow;
   class OSSIMGUI_DLL MainWindow : public QMainWindow, public Ui::ossimGuiMainWindow
   {
      Q_OBJECT
   public:
      MainWindow(QWidget* parent=0);
      
      /**
       * This method will create and set a default menu bar.  This will destroy the
       * old menu bar and allocate a new one and set the new menu bar for the main window
       *
       */
      QMenuBar* createAndSetMenuBar();
      virtual void dropEvent ( QDropEvent * event );     
      virtual void dragEnterEvent(QDragEnterEvent *event);
      virtual bool event ( QEvent * e );      
      virtual void showNode(DataManager::Node* node);

      virtual bool loadProjectFile(const ossimString& projFile);
      virtual bool loadImageFileList(std::vector<ossimString>& ilist);
   
   public slots:
      void saveProject(bool checked = false);
      void saveProjectAs(bool checked = false);
      void openProject(bool checked = false);
      void openImage( bool checked = false );
      void openJpip( bool checked = false );
      void cascadeWindows( bool checked = false );
      void tileWindows( bool checked= false );
      void tabWindows(bool checked=false);
      void closeAllWindows( bool checked=false );
      void exploitationModeChanged(int index);
      void resetExploitationMode();
      
      void about(bool checked=false);
   protected:
      ImageMdiSubWindow* createImageWindow();
      //QMdiArea *mdiArea;
      std::shared_ptr<ossimJobMultiThreadQueue>       m_stagerQueue;
      std::shared_ptr<ossimGui::DisplayTimerJobQueue> m_displayQueue;
      
      ossimRefPtr<DataManager>                    m_dataManager;

      QComboBox* m_exploitationOptions;
      void createModeSelector(QToolBar* bar);
      
   };
}   
#endif
