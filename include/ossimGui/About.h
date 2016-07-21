#ifndef ossimGuiAbout_HEADER
#define ossimGuiAbout_HEADER
#include <ui_About.h>

#include <QDialog>
#include <QObject>
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
