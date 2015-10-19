#ifndef ossimGuiAnnotationItem_HEADER
#define ossimGuiAnnotationItem_HEADER

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>
#include <ossimGui/Export.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>


namespace ossimGui
{
   class OSSIMGUI_DLL AnnotationItem : public QGraphicsItem
   {
      
   public:
      enum ItemDataType
      {
         DATA_ITEM_ID=0,
         DATA_OVERLAY_ID
      };
      
      AnnotationItem(const ossimString& overlayId, const ossimString& id = "");
      virtual ossimString getID()const {return m_id;}
      virtual ossimString getOverlayId()const {return m_overlayId;}
      virtual bool isUsable()const {return m_isEnabled;}
      
      virtual void setID(const ossimString& id);
      virtual void setUsable(const bool& enable);
      
   protected:
      virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
      virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
      virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) = 0;
      virtual QRectF boundingRect() const = 0;
      
      QPen m_annPen;
      
      ossimString m_id;
      bool m_isEnabled;
      ossimString m_overlayId;
   };

}

#endif // ossimGuiAnnotationItem_HEADER
