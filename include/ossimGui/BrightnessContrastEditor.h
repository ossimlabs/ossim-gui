#ifndef ossimGuiBrightnessContrastEditor_HEADER
#define ossimGuiBrightnessContrastEditor_HEADER
#include <ossimGui/ui_BrightnessContrastEditor.h>
#include <ossimGui/Export.h>
#include <QtGui/QDialog>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimRefPtr.h>
namespace ossimGui
{

   class OSSIMGUI_DLL BrightnessContrastEditor : public QDialog, public Ui::BrightnessContrastEditor
   {
      Q_OBJECT
      public:
         BrightnessContrastEditor(QWidget* parent=0, Qt::WindowFlags f = 0 );
      
         void setObject(ossimObject* obj);
      
      public slots:
        void ok();
        void cancel();
        void brightnessSliderChanged();
        void contrastSliderChanged();
        void enabledChanged();
      
   protected:
      ossimRefPtr<ossimConnectableObject> m_brightnessContrast;
      ossim_float64 m_savedBrightness;
      ossim_float64 m_savedContrast;
      
   };
}   
#endif