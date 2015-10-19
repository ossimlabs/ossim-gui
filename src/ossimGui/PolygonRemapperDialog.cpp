//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for simple text data, e.g. position information.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossimGui/PolygonRemapperDialog.h>
#include <ossimGui/ImageScrollView.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimDms.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimDrect.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimEllipsoidFactory.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimUsgsQuad.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossimGui/ImageScrollView.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimGeoPolyCutter.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimMgrs.h>
#include <ossim/projection/ossimUtmProjection.h>

#include <QMessageBox>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QStringList>
#include <QHeaderView>
#include <QListWidget>
#include <QWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>
#include <QLabel>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

ossimGui::PolygonRemapperDialog::PolygonRemapperDialog(QWidget* parent, Qt::WFlags f) 
   :
   QDialog( parent, f ),
   m_textEdit( new QTextEdit() ),
   m_widget(0),
   m_polyCutter(0),
   m_addPolygonBt(0),
   m_enableDisableCb(0),
   m_toggleCutTypeCb(0),
   m_fillValueLe(0),
   m_setfillBt(0),
   m_polygon(0),
   m_mode(ossimGui::PolygonRemapperDialog::UNKNOWN)
{
   // set title in Polygon Remapper Dialog
   this->setWindowTitle("Polygon Remapper");

   // setup vertical layout   
   QVBoxLayout* vbox = new QVBoxLayout();
   QHBoxLayout* hbox = new QHBoxLayout();

   // setup button for adding polygons
   m_addPolygonBt = new QPushButton("Add Polygon", this);
   vbox->addWidget(m_addPolygonBt);
   vbox->addSpacing(10);
   connect(m_addPolygonBt, SIGNAL(clicked(bool)), this, SLOT(getPolyPoint()));

   // setup checkbox for enabling / disabling polycutter
   m_enableDisableCb = new QCheckBox("Enable/Disable Polycutter", this);
   m_enableDisableCb->setCheckState(Qt::Checked);
   vbox->addWidget(m_enableDisableCb);
   vbox->addSpacing(10);
   connect(m_enableDisableCb, SIGNAL(clicked(bool)), this, SLOT(toggleEnableDisable()));

   // setup checkbox for nulling inside / outside polygon
   m_toggleCutTypeCb = new QCheckBox("Null Inside Polygon", this);
   m_toggleCutTypeCb->setCheckState(Qt::Checked);
   m_toggleCutTypeCb->setEnabled(false);
   vbox->addWidget(m_toggleCutTypeCb);
   vbox->addSpacing(10);
   connect(m_toggleCutTypeCb, SIGNAL(clicked(bool)), this, SLOT(toggleCutType()));

   // setup fill value polycutter
   QLabel *m_fillValueLb = new QLabel(tr("Fill Value:"));
   m_fillValueLe = new QLineEdit(this);
   m_fillValueLe->setFixedSize(60, 20);
   m_setfillBt = new QPushButton("Set Fill", this);
   hbox->addWidget(m_fillValueLb);
   hbox->addWidget(m_fillValueLe);
   hbox->addWidget(m_setfillBt);
   vbox->addLayout(hbox);
   connect(m_setfillBt, SIGNAL(clicked(bool)), this, SLOT(setFillType()));

   setLayout(vbox);
}

void ossimGui::PolygonRemapperDialog::setFillType()
{
   // set the fill type here
}

void ossimGui::PolygonRemapperDialog::toggleEnableDisable()
{
   if ( m_polyCutter.valid() && m_enableDisableCb )
   {
      bool oldState = m_polyCutter->getEnableFlag();
      bool newState = m_enableDisableCb->isChecked();
      if ( oldState != newState )
      {
         m_polyCutter->setEnableFlag( newState );
         fireRefreshEvent();
      }
   }
}

void ossimGui::PolygonRemapperDialog::toggleCutType()
{
   if ( m_polyCutter.valid() && m_toggleCutTypeCb )
   {
      ossimGeoPolyCutter::ossimPolyCutterCutType oldType = m_polyCutter->getCutType();

      // Checked == inside:
      ossimGeoPolyCutter::ossimPolyCutterCutType newType =
         ( m_toggleCutTypeCb->isChecked() ? ossimGeoPolyCutter::OSSIM_POLY_NULL_INSIDE :
           ossimGeoPolyCutter::OSSIM_POLY_NULL_OUTSIDE );
      
      if ( oldType != newType )
      {
         m_polyCutter->setCutType( newType );
         fireRefreshEvent();
      }
   }
}

void ossimGui::PolygonRemapperDialog::getPolyPoint( )
{
  m_addPolygonBt->setEnabled(false);
  m_toggleCutTypeCb->setEnabled(true);

  m_polygon.clear();
  m_mode = ossimGui::PolygonRemapperDialog::ACCEPTING_POINTS;

  
  QString caption("Usage:");
  QString text = "Left click to drop points, right click on final point.";
  QMessageBox::warning( this,
                        caption,
                        text,
                        QMessageBox::Ok,
                        Qt::NoButton);
  
}

void ossimGui::PolygonRemapperDialog::mousePress(QMouseEvent* e, const ossimDpt& scenePt)
{
   if( e && (m_mode == ACCEPTING_POINTS) )
   {
      ossimRefPtr<ossimImageGeometry> geom = m_widget->getGeometry();
      if( geom.valid() && geom->getProjection() )
      {
         ossimGpt worldPt;
         geom->localToWorld( scenePt, worldPt );
         
         if( e->buttons() & Qt::LeftButton )   
         {
            m_polygon.push_back (worldPt);
         }
         else if(e->buttons() & Qt::RightButton)
         //if(e->modifiers() == Qt::ShiftModifier)
         {
            m_polygon.push_back( worldPt );
            m_polygon.push_back( m_polygon[0] );
                
            if(m_polyCutter.valid())
            {
               m_polyCutter->addPolygon(m_polygon);
               m_addPolygonBt->setEnabled(true);
               m_polygon.clear();
               m_mode = ossimGui::PolygonRemapperDialog::UNKNOWN;
               fireRefreshEvent();
            }
         }
      }
   }
   
} // End: PolygonRemapperDialog::mousePress(...)

void ossimGui::PolygonRemapperDialog::setPolyCutter( ossimGeoPolyCutter* polygon )
{
  m_polyCutter = polygon;
}

void ossimGui::PolygonRemapperDialog::setWidget(ossimGui::ImageScrollView* widget)
{
  m_widget = widget;
}

void ossimGui::PolygonRemapperDialog::track( const ossimDpt& /* scenePt */ )
{
#if 0
   std::ostringstream os;
   os << setiosflags(ios::fixed) << setiosflags(ios::left)
      << setw(12) << "scene:";
   scenePt.print(os, 2);

   if ( m_widget )
   {
      ossimDpt imagePt;
      sceneToImage( scenePt, imagePt );
      ConnectableImageObject* inputConnection = m_widget->connectableObject();
      if ( !imagePt.hasNans() )
      {
         os << setw(12) << "\nimage:";
         imagePt.print(os, 2);
      }
      
      ossimRefPtr<ossimImageGeometry> geom = m_widget->getGeometry();
      if ( geom.valid() )
      {
         if ( geom->getProjection() )
         {
            ossimGpt worldPt;
            geom->localToWorld( scenePt, worldPt );

            ossimDms latDms( worldPt.latd() );
            ossimDms lonDms( worldPt.lond(), false );
            
            os << setw(12) << "\nLat:"
               << latDms.toString("dd@mm'ss.ssss\" C").c_str()
               << setw(12) << "\nLon:"
               << lonDms.toString("ddd@mm'ss.ssss\" C").c_str()
               << setprecision(15) << setw(12)
               << "\nLat:" << worldPt.latd()
               << setw(12)
               << "\nLon:" << worldPt.lond();
                        
            ossimUtmProjection utm( *ossimEllipsoidFactory::instance()->wgs84(),
                                    worldPt );
            long zone = utm.getZone();
            char hemisphere = utm.getHemisphere();
            ossimDpt eastingNorthing = utm.forward( worldPt );
            
            os << setiosflags(ios::left) << setw(12)
               << "\nUTM zone:" << utm.getZone();
            
            if(!eastingNorthing.hasNans())
            {
               os << setprecision(3) << setw(12)
                  << "\nEasting:"
                  << eastingNorthing.x
                  << setw(12)
                  << "\nNorthing:"
                  << eastingNorthing.y;

               char mgrsString[64];
               std::memset( mgrsString, 0, 64 );
               
               if ( Convert_UTM_To_OSSIM_MGRS ( zone,
                                                hemisphere,
                                                eastingNorthing.x,
                                                eastingNorthing.y,
                                                5, // precision,
                                                mgrsString ) == OSSIM_MGRS_NO_ERROR ) 
               {
                  os << setw(12) << "\nMGRS:" << mgrsString;
               }
            }


            // ossimUsgsQuad quadName(worldPt);
            
            // os << "\nUSGS Q.Q. name:     " << quadName.quarterQuadSegName();

            // os << setiosflags(ios::left) << setw(20) << "x, y:"
            //    << rawImgPt.toString() << std::endl;
            
            ossim_float64 hgtAboveMsl =
               ossimElevManager::instance()->getHeightAboveMSL(worldPt);
            ossim_float64 hgtAboveEllipsoid =
               ossimElevManager::instance()->getHeightAboveEllipsoid(worldPt);
            ossim_float64 geoidOffset =
               ossimGeoidManager::instance()->offsetFromEllipsoid(worldPt);

            os << setw(20) << "\nHeight MSL:";
            if ( ossim::isnan( hgtAboveMsl ) )
            {
               os  << "nan";
            }
            else
            {
               os << hgtAboveMsl;
            }
            
            os << setw(20) << "\nHeight above ellipsoid:"; 
            if ( ossim::isnan( hgtAboveEllipsoid ) )
            {
               os << "nan";
            }
            else
            {
               os << hgtAboveEllipsoid;
            }
            
            os << setw(20) << "\nGeoid offset: "; 
            if ( ossim::isnan( geoidOffset ) )
            {
               os << "nan";
            }
            else
            {
               os << geoidOffset;
            }
         }
      }
   }
   
   m_textEdit->setText( QString( os.str().c_str() ) );
#endif
}

void ossimGui::PolygonRemapperDialog::sceneToImage(
   const ossimDpt& scenePt, ossimDpt& imagePt ) const
{
   imagePt.makeNan();
   
   ConnectableImageObject* inputConnection = m_widget->connectableObject();
   if ( inputConnection )
   {
      ossimTypeNameVisitor visitor(ossimString("ossimImageRenderer"),
                                   true,
                                   ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);

      inputConnection->accept(visitor);
      
      // If there are multiple renderers, e.g. a mosaic do not use.
      if ( visitor.getObjects().size() == 1 )
      {
         ossimRefPtr<ossimImageRenderer> renderer =
            visitor.getObjectAs<ossimImageRenderer>( 0 );
         
         if ( renderer.valid() )
         {
            ossimRefPtr<ossimImageViewProjectionTransform> trans =
               PTR_CAST(ossimImageViewProjectionTransform,
                        renderer->getImageViewTransform());
            if( trans.valid() )
            {
               trans->viewToImage(scenePt, imagePt);
            }
         }
      }
   }
}

void ossimGui::PolygonRemapperDialog::fireRefreshEvent()
{
   if(m_polyCutter.valid())
   {
      ossimRefPtr<ossimRefreshEvent> refreshEvent =
         new ossimRefreshEvent(ossimRefreshEvent::REFRESH_PIXELS);
      ossimEventVisitor visitor(refreshEvent.get());
      m_polyCutter->accept(visitor);
   }
}
