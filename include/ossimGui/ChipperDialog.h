//---
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for chipping/exporting images.
//
//---
// $Id$

#ifndef ossimGuiChipperDialog_HEADER
#define ossimGuiChipperDialog_HEADER 1

#include <ossimGui/RoiRectAnnotator.h>
#include <ossim/base/ossimConnectableObject.h>
#include <ossim/base/ossimConnectableObjectListener.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimROIEvent.h>
#include <ossim/base/ossimROIEventListener.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageSource.h>
#include <QDialog>

class ossimDpt;
class ossimMapProjection;
class ossimProjection;
class ossimROIEvent;
class QComboBox;
class QLineEdit;
class QMouseEvent;
class QPushButton;
class QRubberBand;
class QTextEdit;

namespace ossimGui
{
   class ImageScrollView;
   class View;
   
   class ChipperDialog : public QDialog,
                         public ossimConnectableObjectListener,
                         public ossimROIEventListener
   {
   Q_OBJECT

   public:

      enum ErrorStatus
      {
         OK = 0,
         ERROR = 1
      };
      
      /** @brief default constructor */
      ChipperDialog( QWidget* parent=0, Qt::WFlags f = 0 );

      virtual ~ChipperDialog();

      ossimGui::ChipperDialog::ErrorStatus errorStatus() const;

      virtual void objectDestructingEvent(ossimObjectDestructingEvent& event);
      virtual void handleRectangleROIEvent( ossimROIEvent& event );

   public slots:
      void outputFilePushButtonClicked();
      void saveSpecFilePushButtonClicked();
      void runIgenPushButtonClicked();
      void gsdLineEditReturnPressed();
      void linesLineEditReturnPressed();
      void samplesLineEditReturnPressed();
      void outputFileLineEditReturnPressed();
      void sceneRectPushButtonClicked();
      void imageWidgetDestroyed();
      void editWriterPushButtonClicked();
      void outputTypeComboBoxActivated( const QString & type );
      void syncView();
      
   protected:

   private:

      void buildOutputTypeComboBox();
      QString      getWriterString() const;
      void createWriter(const QString& type);

      /**
       * This will set theOutputImageField to that of the writer output filename
       * provided it is not empty and not the same as the source file.
       */
      void updateOutputFilenameFromWriter();

      void updateOutputTypeFromWriter();

      void buildDialog();

      void setView();

      void setContainerView(ossimConnectableObject* container);
      
      void setSceneBoundingRect();
      void setWidgetRect(const ossimIrect& rect);
      void updateOutputGrect();
      void updateRoiRect();
      void recalculateRect();
      void updateDialog();

      ossim_uint32 getLines()        const;
      ossim_uint32 getSamples()      const;

      /** @brief sets m_lines and m_samples */
      void getBounds(ossim_uint32& lines, ossim_uint32& samples) const;

      /** @brief Removes output file on failure detection. */
      void removeFile() const;
      
      /**
       * @return true if outputFile is contained in the input's chain;
       * false, if not.
       */
      bool isInChain(const ossimFilename& outputFile) const;

      /**
       * @brief Duplicates the input chain for writing output.
       */
      ossimRefPtr<ossimConnectableObject> duplicate(
         const ossimConnectableObject* obj) const;

      ossimGui::ImageScrollView*   m_widget;
      ossimRefPtr<ossimImageSource> m_input;
      
      QLineEdit*    m_outputFileLineEdit;
      QPushButton*  m_outputFilePushButton;
      QComboBox*    m_outputTypeComboBox;
      QPushButton*  m_editWriterPushButton;
      QLineEdit*    m_gsdLineEdit;
      QLineEdit*    m_linesLineEdit;
      QLineEdit*    m_samplesLineEdit;
      QPushButton*  m_sceneRectPushButton;
      QPushButton*  m_saveSpecFilePushButton;
      QPushButton*  m_saveImagePushButton;
      QPushButton*  m_closePushButton;

      ossimFilename m_outputFile;
      ossimRefPtr<ossimImageFileWriter> m_writer;
      ossimRefPtr<ossimProjection> m_windowView;
      ossimRefPtr<ossimMapProjection> m_outputView;
      ossimDpt m_gsd;
      ossim_uint32 m_lines;
      ossim_uint32 m_samples;
      ossimGeoPolygon m_outputGeoPolygon;
      bool m_callBackDisabled;
      RoiRectAnnotator m_annotator;
   };
}

#endif /* #ifndef ossimGuiChipperDialog_HEADER */
