#include <ossimGui/Image.h>

ossimGui::Image::Image(ossimRefPtr<ossimImageData> data, bool includeOffset)
:QImage()
{
   setImage(data, includeOffset);
}

void ossimGui::Image::setImage(ossimRefPtr<ossimImageData> data, bool includeOffset)
{
   if(data.valid())
   {
      int aWidth        = data->getWidth();
      int aHeight       = data->getHeight();
      *this = QImage(aWidth, aHeight, QImage::Format_RGB32);
      if(data->getBuf()&&(data->getScalarType() == OSSIM_UINT8))
      {
         ossim_uint8* buf[3];
         int numberOfBands = data->getNumberOfBands();
         int aWidth        = data->getWidth();
         int aHeight       = data->getHeight();
         int maxPixels     = aWidth*aHeight;
         int offset;
         ossim_uint32* resultBuffer = (ossim_uint32*)bits();
         if(numberOfBands >= 3)
         {
            buf[0] = static_cast<ossim_uint8*>(data->getBuf(0));
            buf[1] = static_cast<ossim_uint8*>(data->getBuf(1));
            buf[2] = static_cast<ossim_uint8*>(data->getBuf(2));
         }
         else 
         {
            buf[0] = static_cast<ossim_uint8*>(data->getBuf(0));
            buf[1] = static_cast<ossim_uint8*>(data->getBuf(0));
            buf[2] = static_cast<ossim_uint8*>(data->getBuf(0));
         }
         if(resultBuffer)
         {
            for(offset = 0; offset < maxPixels;++offset,++resultBuffer)
            {
               *resultBuffer = ((ossim_uint32)'0xff')<<24 |
               ((ossim_uint32)*buf[0])<<16 |
               ((ossim_uint32)*buf[1])<<8 |
               ((ossim_uint32)*buf[2]);
               
               ++buf[0];++buf[1];++buf[2];
            }
         }
         else
         {
            fill(0);
         }
      }
      else // empty buff or not 8-bit then blank out the tile
      {
         fill(0);
      }
      if(includeOffset)
      {
         ossimIpt origin = data->getOrigin();
         setOffset(QPoint(origin.x, origin.y));
      }
   }
}

ossimRefPtr<ossimImageData> ossimGui::Image::toOssimImage()
{
   ossimRefPtr<ossimImageData> result;
   ossimScalarType scalarType = OSSIM_SCALAR_UNKNOWN;
   switch(format())
   {
      case QImage::Format_RGB32:
      case QImage::Format_ARGB32:
      {
         result = new ossimImageData(0,
                                     OSSIM_UINT8,
                                     3,
                                     width(),
                                     height());
         result->setImageRectangle(ossimIrect(offset().x(), offset().y(), offset().x()+width()-1, offset().y()+height()-1));
         result->initialize();
         ossim_uint8* bands[3];
         bands[0] = static_cast<ossim_uint8*>(result->getBuf(0));
         bands[1] = static_cast<ossim_uint8*>(result->getBuf(1));
         bands[2] = static_cast<ossim_uint8*>(result->getBuf(2));
         ossim_uint32 area = width()*height();
         ossim_uint32 idx = 0;
         ossim_uint32* buffer = reinterpret_cast<ossim_uint32*>(bits());
         for(idx = 0; idx < area;++idx,bands[0]++,bands[1]++,bands[2]++,buffer++)
         {
            ossim_uint32 value = *buffer;
            if((value>>24)&0xff)
            {
               *bands[0] = static_cast<ossim_uint8>((value>>16)&0xff);
               *bands[1] = static_cast<ossim_uint8>((value>>8)&0xff);
               *bands[2] = static_cast<ossim_uint8>((value)&0xff);
            }
            else 
            {
               *bands[0] = 0;
               *bands[1] = 0;
               *bands[2] = 0;
            }
         }
         result->validate();
         break;
      }
      default:
      {
         ossimNotify(ossimNotifyLevel_WARN) << "ossimGui::Image::newOssimImage(): Unable to store image in the requested format.  Format not supported\n";
         break;
      }
   }
   
   return result.get();
}


