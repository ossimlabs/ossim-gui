#ifndef ossimGuiImage_HEADER
#define ossimGuiImage_HEADER
#include <QtGui/QImage>
#include <ossim/imaging/ossimImageData.h>
#include <ossimGui/Export.h>
namespace ossimGui{

   
   class OSSIMGUI_DLL Image : public QImage
   {
   public:
      Image()
      :QImage()
      {}
      Image ( const QSize & size, Format format )
      :QImage(size, format)
      {}
      Image ( const QImage & image )
      :QImage(image){}
      Image(ossimRefPtr<ossimImageData> data, bool includeOffset = false);
      
      void setImage(ossimRefPtr<ossimImageData> data, bool includeOffset=false);
      ossimRefPtr<ossimImageData> toOssimImage();
   };
}

#endif
