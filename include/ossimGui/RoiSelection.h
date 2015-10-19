#ifndef ossimGuiRoiSelection_HEADER
#define ossimGuiRoiSelection_HEADER

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>
#include <ossimGui/Export.h>
#include <ossimGui/AnnotationItem.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/base/ossimString.h>


namespace ossimGui
{
   class OSSIMGUI_DLL RoiSelection : public AnnotationItem
   {
      
   public:
      RoiSelection(const ossimDpt& scenePos,
                   const ossimDpt& imagePos,
                   const ossimDpt& widHgt,
                   const ossimString& overlayId,
                   const ossimString& rid = "");
      virtual void drag(const ossimDpt& scenePos, const ossimDpt& imagePos);
      virtual void redefine(ossimDpt* scenePos, ossimDpt* imagePos);
      virtual ossimIrect getRectImg() const;
      
   protected:
      virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
      virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
      virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
      virtual QRectF boundingRect() const;
      
      QRectF m_rect;
      QPen   m_pen;
      QPen   m_savedPen;
      
      // Scene coordinates
      ossimDpt m_scnPos[2];
      
      // Image coordinates
      ossimDpt m_imgPos[2];
   };

}

#endif // ossimRoiSelection_HEADER
