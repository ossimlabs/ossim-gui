#ifndef ossimGuiOpenImageUrlJob_HEADER
#define ossimGuiOpenImageUrlJob_HEADER
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/parallel/ossimJob.h>
#include <ossimGui/Common.h>
#include <ossimGui/Export.h>
#include <QtCore/QUrl>
namespace ossimGui 
{
   class OSSIMGUI_DLL OpenImageUrlJob : public ossimJob
   {
   public:
      
      OpenImageUrlJob(const OpenImageUrlJob& src):m_url(src.m_url), m_handlers(src.m_handlers){}
      OpenImageUrlJob(const QUrl& url)
      :m_url(url)
      {
      }
      virtual void start();
      ossimGui::HandlerList& handlerList()
      {
         return m_handlers;
      }
      const ossimGui::HandlerList& handlerList()const
      {
         return m_handlers;
      }
   protected:
      QUrl        m_url;
      ossimGui::HandlerList m_handlers;
   };
}

#endif
