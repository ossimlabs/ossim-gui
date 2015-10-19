#ifndef ossimGuiImageViewManipulator_HEADER
#define ossimGuiImageViewManipulator_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimIrect.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QResizeEvent>


namespace ossimGui
{
	/*
	class OSSIMGUI_DLL SceneItemUpdate : public QObject
	{
		Q_OBJECT
	public:
		SceneItemUpdate(){}

	public slots:
			void sceneRectChanged ( const QRectF & rect )
			{
				std::cout << "Iterating!!!!!\n";
			} 
	};
	*/
	class ImageScrollView;
	class OSSIMGUI_DLL ImageViewManipulator : public ossimReferenced
	{
	public:
		ImageViewManipulator(ImageScrollView*);

		virtual void setImageScrollView(ImageScrollView*);
		ImageScrollView* getImageScrollView();

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
		virtual void fit();
      void setFullResScale(const ossimDpt& scale);
      ossimDpt getFullResScale() {return m_fullResolutionScale;}
      void fullRes();
      virtual void zoomIn(double factor=2.0);
      virtual void zoomOut(double factor=2.0);
		virtual void initializeToCurrentView();
		bool isAffine()const;

      virtual void   resizeEvent(QResizeEvent* event);
      virtual void   scrollContentsBy( int dx, int dy );
		virtual void	keyPressEvent ( QKeyEvent * event, bool& consumeEvent );
		virtual void	keyReleaseEvent ( QKeyEvent * event, bool& consumeEvent);
		virtual void	mouseDoubleClickEvent ( QMouseEvent * event, bool& consumeEvent );
		virtual void	mouseMoveEvent ( QMouseEvent * event, bool& consumeEvent );
		virtual void	mousePressEvent ( QMouseEvent * event, bool& consumeEvent );
		virtual void	mouseReleaseEvent ( QMouseEvent * event, bool& consumeEvent );
		virtual void	resizeEvent ( QResizeEvent * event, bool& consumeEvent );
		virtual void	wheelEvent ( QWheelEvent * event, bool& consumeEvent );	
		virtual void	enterEvent ( QEvent * event, bool& consumeEvent );      
		virtual void	leaveEvent ( QEvent * event, bool& consumeEvent );     

	protected:
		struct Range
		{
			Range(ossim_float64 minValue=ossim::nan(), ossim_float64 maxValue=ossim::nan())
			:m_min(minValue),
			m_max(maxValue)
			{
			}
			bool isValid()const{return (!ossim::isnan(m_min)&&!ossim::isnan(m_max));}
			bool withinRange(ossim_float64 v){return (v>=m_min&&v<=m_max);}
			ossim_float64 m_min;
			ossim_float64 m_max;
		};
		void setViewToChains();
		void setCommonCenter();
		ossimDpt sceneToLocal(const ossimDpt& scenePoint);

		virtual void fit(const ossimIrect& input, const ossimIrect& target);
      ossimImageGeometry*      asGeometry();

		ImageScrollView*          m_scrollView;
		ossimDpt                  m_centerPoint;	
		ossimDpt						  m_fullResolutionScale;		 
      ossimRefPtr<ossimObject>  m_obj;
      mutable bool              m_leftButtonPressed;
      Range				           m_scaleRange;
    //  SceneItemUpdate*				m_sceneItemUpdate;

	};
}

#endif
