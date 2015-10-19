//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for simple text data, e.g. position information.
//
//----------------------------------------------------------------------------
// $Id$

#include <ossimGui/PositionInformationDialog.h>
#include <ossimGui/ImageScrollView.h>
#include <ossim/base/ossimCommon.h>
#include <ossim/base/ossimDms.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimEllipsoid.h>
#include <ossim/base/ossimEllipsoidFactory.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimUsgsQuad.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimMgrs.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>

#include <cstring>
#include <iomanip>
#include <sstream>


ossimGui::PositionInformationDialog::PositionInformationDialog(QWidget* parent, Qt::WFlags f)
   :
   QDialog( parent, f ),
   m_textEdit( new QTextEdit() ),
   m_widget(0)
{
   QVBoxLayout* layout = new QVBoxLayout();
   layout->addWidget( m_textEdit );
   setLayout( layout );
   
   // Set defaults:
   setModal( false ); // non blocking 
   m_textEdit->setReadOnly(true);
   resize(300,400);
}

void ossimGui::PositionInformationDialog::setWidget(ossimGui::ImageScrollView* widget)
{
   m_widget = widget;
}

void ossimGui::PositionInformationDialog::track( const ossimDpt& scenePt )
{
   std::ostringstream os;
   os << setiosflags(ios::fixed) << setiosflags(ios::left)
      << setw(12) << "scene:";
   scenePt.print(os, 2);

   if ( m_widget )
   {
      ossimDpt imagePt;
      sceneToImage( scenePt, imagePt );
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
               os << "nan\n";
            }
            else
            {
               os << geoidOffset << "\n";
            }
         }
         
      } // Matches: if ( geom.valid() )

      // Get the rbg pixel values sent to the viewport:
      ossim_uint8 r=0;
      ossim_uint8 g=0;
      ossim_uint8 b=0;
      m_widget->getRgb( ossimIpt(scenePt), r, g, b );
      os << "rgb( " << int(r) << ", " << int(g) << ", " << int(b) << " )\n";

      //---
      // Get the raw pixels values from the image.  Note this will only work if
      // a single image chain.
      //---
      std::vector<ossim_float64> values;
      m_widget->getRaw( ossimIpt(imagePt), values );
      if ( values.size() )
      {
         const ossim_uint32 BANDS = values.size();
         const ossim_uint32 MAX_BANDS_DISPLAYED = ossim::min<ossim_uint32>(BANDS, 16);
         for ( ossim_uint32 band = 0; band < MAX_BANDS_DISPLAYED; ++band )
         {
            os << "raw_image_pixel_value[ " << band << "]: " << values[band] << "\n";
         }
         if ( BANDS > MAX_BANDS_DISPLAYED )
         {
            os << "raw pixel output clamped to " << MAX_BANDS_DISPLAYED << " bands.\n";
         }
      }
   }
   
   m_textEdit->setText( QString( os.str().c_str() ) );
}


void ossimGui::PositionInformationDialog::sceneToImage(
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
