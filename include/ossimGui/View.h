#ifndef ossimGuiView_HEADER
#define ossimGuiView_HEADER
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/base/ossimReferenced.h>
#include <ossimGui/Export.h>

namespace ossimGui{
   class OSSIMGUI_DLL View : public ossimReferenced
   {
   public:
      enum GeomType
      {
         GEOM_TYPE_UNKNOWN        = 0,
         GEOM_TYPE_MAP_PROJECTION = 1,
         GEOM_TYPE_SENSOR_MODEL   = 2
      };
      enum SyncType
      {
         SYNC_TYPE_NONE     = 0,
         SYNC_TYPE_CURSOR   = 1, // paints cursor
         SYNC_TYPE_POSITION = 2, // will adjust scrolls
         SYNC_TYPE_GEOM     = 4, // will propagate geometry
         SYNC_TYPE_RESAMPLER= 8,
         SYNC_TYPE_ALL      = (SYNC_TYPE_CURSOR|SYNC_TYPE_POSITION|SYNC_TYPE_GEOM|SYNC_TYPE_RESAMPLER)
      };
      
      
      View()
      :m_geomType(GEOM_TYPE_UNKNOWN),
       m_syncType(SYNC_TYPE_NONE)
      {
         m_lookPosition.makeNan();
      }
      View (int sync, ossimImageGeometry* geom)
      :m_geomType(GEOM_TYPE_UNKNOWN),
      m_syncType(static_cast<SyncType>(sync&SYNC_TYPE_ALL)),
      m_view(geom)

      {
         m_lookPosition.makeNan();
         if(geom&&geom->getProjection())
         {
            if(geom->getProjection()->canCastTo("ossimMapProjection"))    m_geomType = GEOM_TYPE_MAP_PROJECTION;
            else if(geom->getProjection()->canCastTo("ossimSensorModel")) m_geomType = GEOM_TYPE_SENSOR_MODEL;
         }
      }
      View(int sync, const ossimDpt& position)
      :m_geomType(GEOM_TYPE_UNKNOWN),
      m_syncType(static_cast<SyncType>(sync&SYNC_TYPE_ALL)),
      m_lookPosition(position)
      {
      }
      View(int sync, const ossimDpt& position, ossimImageGeometry* geom)
      :m_geomType(GEOM_TYPE_UNKNOWN),
      m_syncType(static_cast<SyncType>(sync&SYNC_TYPE_ALL)),
      m_lookPosition(position),
      m_view(geom)
      {
         if(geom&&geom->getProjection())
         {
            if(geom->getProjection()->canCastTo("ossimMapProjection"))    m_geomType = GEOM_TYPE_MAP_PROJECTION;
            else if(geom->getProjection()->canCastTo("ossimSensorModel")) m_geomType = GEOM_TYPE_SENSOR_MODEL;
         }
      }
      View(const View& src)
      :m_syncType(src.m_syncType),
      m_lookPosition(src.m_lookPosition),
      m_view(src.m_view.valid()?src.m_view->dup():0),
      m_resamplerType(src.m_resamplerType)
      {
      }
      virtual View* dup()const{return new View(*this);}
      
      void setSyncType(int type, bool flag=true)
      {
         if(flag) m_syncType = static_cast<SyncType>((m_syncType|type)&SYNC_TYPE_ALL);
         else m_syncType = static_cast<SyncType>((~type)&m_syncType);

      }
      GeomType geomType()const{return m_geomType;}
      SyncType syncType()const{return m_syncType;}
      bool lookPositionValid(){return !m_lookPosition.hasNans();}
      const ossimDpt& lookPosition()const{return m_lookPosition;}
      ossimGpt lookPositionAsGpt()const
      {
         ossimGpt result;
         result.makeNan();
         if(geometry()&&!m_lookPosition.hasNans())
         {
            geometry()->localToWorld(m_lookPosition, result);
         }
         return result;
      }
      const ossimString& resamplerType()const{return m_resamplerType;}
      void setResamplerType(const ossimString& value){m_resamplerType = value;}
      virtual const ossimImageGeometry*       geometry()const{return viewAs<ossimImageGeometry>();}
      virtual ossimImageGeometry*       geometry(){return viewAs<ossimImageGeometry>();}
      
      /**
       * This might move to a base class later.  For now until we figure out 3-D views 
       * we will leave it here
       */ 
      template<class T>
      T* viewAs(){return dynamic_cast<T*>(m_view.get());}
      template<class T>
      const T* viewAs()const{return dynamic_cast<const T*>(m_view.get());}
   protected:
      GeomType                        m_geomType;
      SyncType                        m_syncType;
      // ossimGpt                        m_lookPosition;
      ossimDpt                        m_lookPosition;
      ossimRefPtr<ossimObject>        m_view;
      ossimString                     m_resamplerType;
   };
}
#endif
