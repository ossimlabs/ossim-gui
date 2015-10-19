#ifndef ossimGuiViewManipulator_HEADER
#define ossimGuiViewManipulator_HEADER
#include <ossimGui/Export.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimDrect.h>

namespace ossimGui{
   class OSSIMGUI_DLL ViewManipulator : public ossimReferenced
   {
   public:
      ViewManipulator();
      ViewManipulator(ossimObject* obj);
      void setObject(ossimObject* geom);
      template<class T>
      T* getObjectAs()
      {
         return dynamic_cast<T*>(m_obj.get());
      }
      template<class T>
      const T* getObjectAs()const
      {
         return dynamic_cast<T*>(m_obj.get());
      }
      void setFullResScale(const ossimDpt& scale);
      void fullRes(ossimDpt& center);
      void fit(const ossimDrect& inputRect,
               const ossimDrect& targetRect);
      void zoomIn(ossimDpt& center, double factor=2.0);
      void zoomOut(ossimDpt& center, double factor=2.0);
      
   protected:
      ossimImageGeometry*      asGeometry();
      ossimDpt                 m_fullResolutionScale;
      ossimRefPtr<ossimObject> m_obj;
   };
}
#endif
