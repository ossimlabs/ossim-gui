#ifndef ossimGuiRegistrationOverlay_HEADER
#define ossimGuiRegistrationOverlay_HEADER

// #include <QtGui/QWidget>
#include <ossimGui/OverlayBase.h>
#include <ossimGui/Export.h>
#include <ossimGui/RegPoint.h>
#include <ossimGui/RoiSelection.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimDpt.h>
#include <map>


namespace ossimGui {
		
	class IvtGeomTransform;

	class OSSIMGUI_DLL RegistrationOverlay : public OverlayBase
	{
		Q_OBJECT

	public:
	   RegistrationOverlay(const ossimString& overlayId, QGraphicsScene* scene);
	   virtual void reset();

	   virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimString& id);
	   virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt);
	   virtual void removePoint(const ossimString& id);
	   virtual void togglePointActive(const ossimString& id);

      virtual void addRoi(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimDpt& widHgt, const ossimString& id);
      virtual void dragRoi(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimString& id);
      virtual void removeRoi(const ossimString& id);
      virtual ossimGui::RoiSelection* getRoiSelection(const ossimString& id);

      virtual bool getImgPoint(const ossimString& id, ossimDpt& imgPt, bool& isActive);
      virtual ossimGui::RegPoint* getRegPoint(const ossimString& id);
      virtual ossimString getCurrentId()const {return m_currentId;}
      virtual ossim_uint32 getNumPoints()const;
      virtual bool isControlImage()const {return m_isControlImage;}
      virtual bool hasAdjParInterface()const {return m_hasAdjParInterface;}

      virtual void setVisible(const bool& visible);
      virtual void setAsControl(const bool& controlImage);
      virtual void setHasAdjParInterface(const bool& hasAdjIface);
      virtual void setCurrentId(const ossimString& id) {m_currentId = id;}
      virtual void setView(ossimRefPtr<IvtGeomTransform> ivtg);
    
   signals:
   	void pointActivated(const ossimString&);
   	void pointDeactivated(const ossimString&);
   	void pointRemoved(const ossimString&);

	protected:
      ossimString m_currentId;
      bool			m_isControlImage;
      bool        m_hasAdjParInterface;

	};

}

#endif // ossimGuiRegistrationOverlay_HEADER
