#ifndef ossimGuiRegPoint_HEADER
#define ossimGuiRegPoint_HEADER

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

	class OSSIMGUI_DLL RegPoint : public AnnotationItem
	{

	public:
	   RegPoint(const ossimDpt& scenePos,
               const ossimDpt& imgPos,
               const ossimString& overlayId,
               const ossimString& id = "");
	   virtual ossimDpt getImgPos()const {return m_imgPos;}
	   virtual bool isControlPoint()const {return m_isControlPoint;}

	   virtual void setAsControlPoint(const bool& control) {m_isControlPoint = control;}
	   virtual void setUsable(const bool& active);

	protected:
		virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
		virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	   virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	   virtual QRectF boundingRect() const;

	   QLineF m_ver;
	   QLineF m_hor;
	   qreal  m_len;
	   QPen   m_pen;
	   QPen   m_savedPen;

	   ossimDpt m_imgPos;
	   bool m_isControlPoint;
	};

}

#endif // ossimGuiRegPoint_HEADER
