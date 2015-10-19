#ifndef ossimGuiMarkPoint_HEADER
#define ossimGuiMarkPoint_HEADER

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>
#include <ossimGui/Export.h>
#include <ossimGui/AnnotationItem.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>


namespace ossimGui {

	class OSSIMGUI_DLL MarkPoint : public AnnotationItem
	{

	public:
	   MarkPoint(const ossimDpt& scenePos,
                const ossimDpt& imgPos,
                const ossimString& overlayId,
                const ossimString& id = "");
	   virtual ossimDpt getImgPos()const {return m_imgPos;}

	protected:
		virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
		virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	   virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	   virtual QRectF boundingRect() const;

	   QRectF m_rect;
	   QLineF m_ver;
	   QLineF m_hor;
	   qreal  m_len;
	   QPen   m_pen;
	   QPen   m_savedPen;

	   ossimDpt m_imgPos;
	};

}

#endif // ossimMarkPoint_HEADER
