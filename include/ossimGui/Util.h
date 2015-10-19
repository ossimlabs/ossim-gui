#ifndef ossimGuiUtil_HEADER
#define ossimGuiUtil_HEADER

#include <ossimGui/Export.h>
#include <QtGui/QWidget>
#include <QtCore/QStringList>
#include <ossim/base/ossimString.h>

namespace ossimGui 
{
   class OSSIMGUI_DLL Util
   {
   public:
      template <class T>
      static T findParentOfType(QObject* startChild)
      {
         T result = dynamic_cast<T> (startChild);
         QObject* current = startChild;
         while(current&&!result)
         {
            current = current->parent();
            result = dynamic_cast<T> (current);
         }
         
         return result;
      }
      static void imageWriterTypes(QStringList& result);
      static void imageWriterTypes(std::vector<ossimString>& result);
   };
   
   
}
#endif
