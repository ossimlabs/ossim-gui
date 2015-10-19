#ifndef ossimGuiAbout_HEADER
#define ossimGuiAbout_HEADER
#include <ossimGui/ui_About.h>
#include <QtGui/QDialog>
#include <ossimGui/Export.h>

namespace ossimGui 
{
   class OSSIMGUI_DLL About : public QDialog, public Ui::About
   {
      Q_OBJECT
   public:
      About(QWidget* parent);
      
   };
}
#endif
