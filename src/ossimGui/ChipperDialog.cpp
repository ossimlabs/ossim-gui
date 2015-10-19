//---
//
// License:  See top level LICENSE.txt file.
//
// Description:  Description: Dialog box for chipping/exporting images.
//
//---
// $Id$

#include <ossimGui/ChipperDialog.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/ProgressDialog.h>
#include <ossimGui/ProgressWidget.h>
#include <ossimGui/PropertyEditorDialog.h>
#include <ossimGui/View.h>
#include <ossim/base/ossimIdManager.h>
#include <ossim/base/ossimKeywordNames.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/base/ossimProperty.h>
#include <ossim/base/ossimROIEvent.h>
#include <ossim/base/ossimTrace.h>
#include <ossim/imaging/ossimGeoPolyCutter.h>
#include <ossim/imaging/ossimIgenGenerator.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>

#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPoint>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

static ossimTrace traceDebug("ChipperDialog:debug");

ossimGui::ChipperDialog::ChipperDialog(QWidget* parent, Qt::WFlags f) 
   :
   QDialog( parent, f ),
   ossimConnectableObjectListener(),
   ossimROIEventListener(),
   m_widget(0),
   m_input(0),   
   m_outputFileLineEdit(0),
   m_outputFilePushButton(0),
   m_outputTypeComboBox(0),
   m_editWriterPushButton(0),
   m_gsdLineEdit(0),
   m_linesLineEdit(0),
   m_samplesLineEdit(0),
   m_sceneRectPushButton(0),
   m_saveSpecFilePushButton(0),
   m_saveImagePushButton(0),
   m_closePushButton(0),
   m_outputFile(),
   m_writer(0),
   m_windowView(0),
   m_outputView(0),
   m_gsd(),
   m_lines(0),
   m_samples(0),
   m_outputGeoPolygon(),
   m_callBackDisabled(false),
   m_annotator()
{
   m_widget = dynamic_cast<ossimGui::ImageScrollView*>(parent);

   if ( m_widget )
   {
      // Listen for roi events from the annotator.
      m_annotator.addListener((ossimROIEventListener*)this);
      
      m_outputGeoPolygon.clear();

      // Annotator will connect slots up to widget.
      m_annotator.setImageWidget( m_widget );

      buildDialog();

      setView();

      connect(m_widget, SIGNAL(viewChanged()), this, SLOT(syncView()));
   }
}

ossimGui::ChipperDialog::~ChipperDialog()
{
   m_annotator.removeListener((ossimROIEventListener*)this);
}

void ossimGui::ChipperDialog::objectDestructingEvent(ossimObjectDestructingEvent& /* event */)
{
}

void ossimGui::ChipperDialog::handleRectangleROIEvent( ossimROIEvent& event)
{
   if (event.getEventType() == ossimROIEvent::OSSIM_RECTANGLE_ROI)
   {
      if (event.getMovingFlag() == false)
      {
         // Mouse drag, update lines and samples.
         getBounds( m_lines, m_samples );
      }
      
      // Update the geo rect from ROI rect.
      updateOutputGrect();
      updateDialog();
   }
}

ossimGui::ChipperDialog::ErrorStatus ossimGui::ChipperDialog::errorStatus() const
{
   return ( m_widget &&  m_input.valid() && m_windowView.valid() )
      ? ossimGui::ChipperDialog::OK : ossimGui::ChipperDialog::ERROR;
}

void ossimGui::ChipperDialog::buildDialog()
{
   // set title in Polygon Remapper Dialog
   this->setWindowTitle( tr("chipper") );

   this->setModal( false );
   
   // setup vertical layout   
   QVBoxLayout* vbox0 = new QVBoxLayout();
   
   // Row 1:
   QGroupBox* outputImageGroupBox = new QGroupBox(tr("output image"));
   outputImageGroupBox->setAlignment(Qt::AlignHCenter);
   QHBoxLayout* hboxR1 = new QHBoxLayout();

   // Line edit:
   m_outputFileLineEdit = new QLineEdit();
   hboxR1->addWidget(m_outputFileLineEdit);

   // File dialog:
   m_outputFilePushButton = new QPushButton( tr("file") );
   m_outputFilePushButton->setAutoDefault(false);   
   hboxR1->addWidget(m_outputFilePushButton);

   // Output type/writer:
   m_outputTypeComboBox = new QComboBox();
   hboxR1->addWidget(m_outputTypeComboBox);
   buildOutputTypeComboBox(); // Populate the options.

   // Edit writer:
   m_editWriterPushButton = new QPushButton( tr("edit writer") );
   m_editWriterPushButton->setAutoDefault(false);
   hboxR1->addWidget(m_editWriterPushButton);

   outputImageGroupBox->setLayout(hboxR1);
   
   vbox0->addWidget(outputImageGroupBox);

   // Row 2:

   QHBoxLayout* hboxR2 = new QHBoxLayout();

   // r2_col1 gsd:

   QGroupBox* gsdGroupBox = new QGroupBox( tr("gsd in meters") );
   gsdGroupBox->setAlignment(Qt::AlignHCenter);
   QHBoxLayout* hbox_r2_c1 = new QHBoxLayout();
   m_gsdLineEdit = new QLineEdit();
   hbox_r2_c1->addWidget(m_gsdLineEdit);
   gsdGroupBox->setLayout(hbox_r2_c1);
   hboxR2->addWidget(gsdGroupBox);
   
   // lines:
   QGroupBox* linesGroupBox = new QGroupBox(tr("lines"));
   linesGroupBox->setAlignment(Qt::AlignHCenter);
   QHBoxLayout* hbox_r2_c2 = new QHBoxLayout();
   m_linesLineEdit = new QLineEdit();
   hbox_r2_c2->addWidget(m_linesLineEdit);
   linesGroupBox->setLayout(hbox_r2_c2);
   hboxR2->addWidget(linesGroupBox);

   // samples:
   QGroupBox* samplesGroupBox = new QGroupBox(tr("samples"));
   samplesGroupBox->setAlignment(Qt::AlignHCenter);
   QHBoxLayout* hbox_r2_c3 = new QHBoxLayout();
   m_samplesLineEdit = new QLineEdit();
   hbox_r2_c3->addWidget(m_samplesLineEdit);
   samplesGroupBox->setLayout(hbox_r2_c3);
   hboxR2->addWidget(samplesGroupBox);

   // Scene rect button:
   m_sceneRectPushButton = new QPushButton( tr("use scene rect") );
   m_sceneRectPushButton->setAutoDefault(false);
   hboxR2->addWidget( m_sceneRectPushButton );

   vbox0->addLayout(hboxR2);

   // Row 3:
   QHBoxLayout* hboxR3 = new QHBoxLayout();
   
   // save spec:
   m_saveSpecFilePushButton = new QPushButton( tr("save spec file") );
   m_saveSpecFilePushButton->setAutoDefault(false);
   hboxR3->addWidget( m_saveSpecFilePushButton );

   // save image:
   m_saveImagePushButton = new QPushButton( tr("save image") );
   m_saveImagePushButton->setAutoDefault(false);
   hboxR3->addWidget( m_saveImagePushButton );

   // close:
   m_closePushButton = new QPushButton( tr("close") );
   m_closePushButton->setAutoDefault(false);
   hboxR3->addWidget( m_closePushButton );

   vbox0->addLayout(hboxR3);

   setLayout(vbox0);

   // Signals and slots connections:
   connect(m_closePushButton, SIGNAL(clicked()),
           this, SLOT( close() ) );
   connect(m_editWriterPushButton, SIGNAL(clicked()),
           this, SLOT(editWriterPushButtonClicked()));
   connect(m_gsdLineEdit, SIGNAL(returnPressed()),
           this, SLOT(gsdLineEditReturnPressed()));
   connect(m_linesLineEdit, SIGNAL(returnPressed()), this,
           SLOT(linesLineEditReturnPressed()));
   connect(m_outputFileLineEdit, SIGNAL(returnPressed()),
           this, SLOT(outputFileLineEditReturnPressed()));
   connect(m_outputFilePushButton, SIGNAL(clicked()),
           this, SLOT(outputFilePushButtonClicked()));
   connect(m_outputTypeComboBox, SIGNAL(activated(const QString&)),
           this, SLOT(outputTypeComboBoxActivated(const QString&)));
   connect(m_samplesLineEdit, SIGNAL(returnPressed()), this,
           SLOT(samplesLineEditReturnPressed()));
   connect(m_saveImagePushButton, SIGNAL(clicked()), this,
           SLOT(runIgenPushButtonClicked()));
   connect(m_saveSpecFilePushButton, SIGNAL(clicked()), this,
           SLOT(saveSpecFilePushButtonClicked()));
   connect(m_sceneRectPushButton, SIGNAL(clicked()),
           this, SLOT(sceneRectPushButtonClicked()));
   
} // ChipperDialog::buildDialog()

void ossimGui::ChipperDialog::setView()
{
   if ( m_widget )
   {
      ossimRefPtr<ossimConnectableObject> input = m_widget->connectableObject()->getInput(0);
      if ( input.valid() )
      {
         m_input = dynamic_cast<ossimImageSource*>( input.get() );
         if ( m_input.valid() )
         {
            ossimRefPtr<ossimImageGeometry> geom = m_input->getImageGeometry();
            if ( geom.valid() )
            {
               ossimRefPtr<ossimProjection> proj = geom->getProjection();
               if ( proj.valid() )
               {
                  m_windowView = proj;
                  
                  //---
                  // Duplicate the view since we will change the gsd for writing purposes.
                  //---
                  ossimRefPtr<ossimProjection> obj = (ossimProjection*) m_windowView->dup();
                  m_outputView = PTR_CAST(ossimMapProjection, obj.get());
               }
               
               if ( (m_outputGeoPolygon.size() == 0) && m_windowView.valid())
               {
                  // Must be first time through.
                  m_gsd = m_windowView->getMetersPerPixel();
                  setSceneBoundingRect();
               }
               else
               {
                  updateRoiRect();
               }
            }
         }
      }
   }
   
} // End: ossimGui::ChipperDialog::setView()

void ossimGui::ChipperDialog::setContainerView(ossimConnectableObject* container)
{
   if ( container && m_outputView.valid() )
   {
      // Set the resolution.
      m_outputView->setMetersPerPixel( m_gsd );

      // Snap the tie point to something that makes sense.
      if ( m_outputView->isGeographic() )
      {
         // If geographic, snap to an arc second.
         ossim_float64 arcSecond = 1.0/3600.0;
         m_outputView->snapTiePointTo(arcSecond, OSSIM_DEGREES);
      }
      else
      {
         // If not geographic, snap the tie point to an even meter.
         m_outputView->snapTiePointTo(1.0, OSSIM_METERS);
      }
      
      // Find all the views from input.
      ossimTypeNameVisitor visitor( ossimString("ossimViewInterface"),
                                    false, // firstofTypeFlag
                                    (ossimVisitor::VISIT_INPUTS|
                                     ossimVisitor::VISIT_CHILDREN) );
      container->accept( visitor );
      
      ossim_uint32 idx = 0;
      const ossim_uint32 SIZE = visitor.getObjects().size();
      for( idx = 0; idx < SIZE; ++idx )
      {
         ossimViewInterface* viewInterface = visitor.getObjectAs<ossimViewInterface>( idx );
         if (viewInterface)
         {
            viewInterface->setView( m_outputView.get() );
         }
      }
      
      // Lets initialize everyone else after we set all views just incase there are dependencies.
      for( idx = 0; idx < SIZE; ++idx )
      {
         ossimRefPtr<ossimConnectableObject> obj =
            visitor.getObjectAs<ossimConnectableObject>( idx );
         if ( obj.valid() )
         {
            ossimRefreshEvent evt( obj.get() );
            obj->fireEvent(evt);
            obj->propagateEventToOutputs(evt);
         }
      }
   }
   
} // End: ossimGui::ChipperDialog::setContainerView( ... )

void ossimGui::ChipperDialog::setSceneBoundingRect()
{
   if ( m_input.get() )
   {
      ossimIrect rect;
      m_input->getBoundingRect( rect, 0 );
      setWidgetRect( rect );
      getBounds( m_lines, m_samples );
      updateOutputGrect();
      updateRoiRect();
   }
}

void ossimGui::ChipperDialog::updateOutputGrect()
{
   if ( m_windowView.get() && m_widget )
   {
      //---
      // First make sure the gsd is set.
      // Later we'll want to make this in decimal degrees if the projection
      // is geographic.  This interface will need to be added to the igen dialog.
      //---
      m_outputView->setMetersPerPixel(m_gsd);
      
      m_outputGeoPolygon.clear();
      
      ossimIrect rect = m_annotator.getRoiRect();
      ossimDpt dpt;
      ossimGpt gpt;
      
      // Upper left tie point from window view to ground space.
      dpt = rect.ul();
      m_windowView->lineSampleToWorld(dpt, gpt);
      
      // Convert the ground point output view space.
      m_outputView->worldToLineSample(gpt, dpt);
      
      // Snap it to an even view point.
      dpt.x = ossim::round<ossim_float64>(dpt.x);
      dpt.y = ossim::round<ossim_float64>(dpt.y);
      
      // Convert to ground point.
      m_outputView->lineSampleToWorld(dpt, gpt);
      
      // Add upper left.
      m_outputGeoPolygon.addPoint(gpt);
      
      // Add upper right
      dpt.x += m_samples - 1;
      m_outputView->lineSampleToWorld(dpt, gpt);
      m_outputGeoPolygon.addPoint(gpt);
      
      // Add lower right
      dpt.y += m_lines - 1;
      m_outputView->lineSampleToWorld(dpt, gpt);
      m_outputGeoPolygon.addPoint(gpt);
      
      // lower left
      dpt.x -= m_samples -1;
      m_outputView->lineSampleToWorld(dpt, gpt);
      m_outputGeoPolygon.addPoint(gpt);
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimGui::ChipperDialog::updateOutputGrect DEBUG:"
            << "\nAnnotator rect:  " << rect
            << "\nlines: " << m_lines
            << "\nsamples: " << m_samples
            << "\nm_outputGeoPolygon\n" << m_outputGeoPolygon
            << endl;
      }
      // updateDialog();
      
   } // Matches: if ( m_windowView.get() && m_widget )
   
} // End: ChipperDialog::updateOutputGrect()

void ossimGui::ChipperDialog::updateRoiRect()
{
   if ( m_widget && m_windowView.get() && (m_outputGeoPolygon.size() == 4) )
   {
      ossimIrect rect;
      ossimDpt dpt;
      
      // upper left
      m_windowView->worldToLineSample(m_outputGeoPolygon[0], dpt);
      rect.set_ul(dpt);
      
      // lower right
      m_windowView->worldToLineSample(m_outputGeoPolygon[2], dpt);
      rect.set_lr(dpt);
      
      setWidgetRect(rect);

      updateDialog();
   }
}

void ossimGui::ChipperDialog::recalculateRect()
{
   if ( m_widget && m_windowView.get() )
   {
      ossimIrect rect = m_annotator.getRoiRect();
   
      if (rect.hasNans() == false )
      {
         ossimDpt view_gsd = m_windowView->getMetersPerPixel();
         
         double meters_in_line_dir = m_lines*m_gsd.y;
         double meters_in_samp_dir = m_samples*m_gsd.x;
         
         double view_lines = meters_in_line_dir / view_gsd.y;
         double view_samps = meters_in_samp_dir / view_gsd.x;
         
         ossimDpt center;
         rect.getCenter(center);
         
         ossimDrect drect;
         
         // upper left
         ossimDpt ul;
         ul.x = center.x - (view_samps/2.0);
         ul.y = center.y - (view_lines/2.0);
         drect.set_ul(ul);
         
         ossimDpt lr;
         // lower right
         lr.x = ul.x + view_samps-1;
         lr.y = ul.y + view_lines-1;
         drect.set_lr(lr);
         
         // Now make an ossimIrect for the widget.
         rect = drect;
         
         setWidgetRect(rect);
         
         // Update the stored geographic points.
         updateOutputGrect();
      }
   }
   
} // End: ChipperDialog::recalculateRect()

void ossimGui::ChipperDialog::updateDialog()
{
   if ( m_widget && m_windowView.get() )
   {
      //---
      // Since with the dialog box can trigger the callbacks set this
      // so that the receivers of callback events don't execute.
      //---
      m_callBackDisabled = true;

      // Set the gsd.  Currently just using x value.
      ossimString s = ossimString::toString(m_gsd.x, 4);
      QString qs = s.c_str();
      m_gsdLineEdit->setText(qs);
      
      // Set the lines.
      s = ossimString::toString(m_lines);
      qs = s.c_str();
      m_linesLineEdit->setText(qs);
      
      // Set the samples.
      s = ossimString::toString(m_samples);
      qs = s.c_str();
      m_samplesLineEdit->setText(qs);
      
      // Set the output filename.
      m_outputFileLineEdit->setText(m_outputFile.c_str());
      
      // Enable the valueChanged event methods...
      m_callBackDisabled = false;
   }
}

void ossimGui::ChipperDialog::outputFilePushButtonClicked()
{
   QFileDialog fd(this);
   fd.setFileMode(QFileDialog::AnyFile);
   fd.setViewMode(QFileDialog::List);
   // fd.setDirectory();

   if ( fd.exec() )
   {
      QStringList fileNames = fd.selectedFiles();
      if ( fileNames.size() )
      {
         QString file = fileNames.at(0);
         m_outputFile = file.toStdString();

         // This is from the dialog so update the line edit.
         m_outputFileLineEdit->setText(file);
      }
   }
}

void ossimGui::ChipperDialog::saveSpecFilePushButtonClicked()
{
   if ( m_widget && m_outputView.valid() && m_writer.valid() )
   {
      
      if (m_outputFile == ossimFilename::NIL)
      {
         QString caption("Notice:");
         QString text = "You must specify an output file!";
         
         // Give the user an already open
         QMessageBox::warning( this,
                               caption,
                               text,
                               QMessageBox::Ok,
                               QMessageBox::NoButton);
         return;
      }

      // Get the spec file name:
      QString file;
      QFileDialog fd(this);
      fd.setFileMode(QFileDialog::AnyFile);
      fd.setViewMode(QFileDialog::List);
      // fd.setDirectory();

      if ( fd.exec() )
      {
         QStringList fileNames = fd.selectedFiles();
         if ( fileNames.size() )
         {
            file = fileNames.at(0);
         }
      }
      
      if (m_outputFile.string() == file.toStdString())
      {
         QString caption("Notice:");
         QString text = "Your spec file cannot be the same as the output file!";
         
         // Give the user an already open
         QMessageBox::warning( this,
                               caption,
                               text,
                               QMessageBox::Ok,
                               QMessageBox::NoButton);
         return;
      }
      
      // Update the ground rectangle in case someone changed the view.
      updateOutputGrect();
      
      ossimIgenGenerator* igen = new ossimIgenGenerator();
      
      igen->setOutputPolygon(m_outputGeoPolygon);
      m_outputView->setMetersPerPixel(m_gsd);
      
      igen->setView(m_outputView.get());
      igen->setInput( m_input.get() );
      
      m_writer->setOutputName(m_outputFile);
      igen->setOutput(m_writer.get());
      
      igen->generateSpecList();
      ossimKeywordlist kwl;
      igen->getSpec(kwl, 0);
      
      ossimFilename spec_file = file.toStdString();
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_INFO)
            << "INFO ossimQtIgenController::saveSpecFile: Writing spec file = "
            << spec_file << std::endl;
      }
      
      kwl.write(spec_file);
      
      delete igen;
      igen = 0;
   }
   
} // End: ossimGui::ChipperDialog::saveSpecFilePushButtonClicked()

void ossimGui::ChipperDialog::runIgenPushButtonClicked()
{
   if ( errorStatus() == ossimGui::ChipperDialog::OK )
   {
      // Make sure the output file name has been set.
      if (m_outputFile != ossimFilename::NIL)
      {
         // Check to see if file exist and prompt user for overrite.
         bool continueRun = true;
         if (m_outputFile.exists())
         {
            QString caption("Question:");
            QString text = "Overwrite existing file:  ";
            text += m_outputFile.c_str();
            int answer = QMessageBox::question( this,
                                                caption,
                                                text,
                                                QMessageBox::Yes,
                                                QMessageBox::No );
            if (answer == QMessageBox::No)
            {
               continueRun = false;
            }
         }

         if ( continueRun )
         {
            // Check the chain to make sure the output file is not one of the input files.
            if ( !isInChain(m_outputFile) )
            {
               // Make a spec file
               ossimFilename spec_file = m_outputFile;
               spec_file.setExtension("spec");
               
               if (m_outputFile != spec_file)
               {
                  ossimRefPtr<const ossimConnectableObject> widgetInput = m_input.get();
                  if (widgetInput.get())
                  {
                     // Duplicate the widget's input
                     ossimRefPtr<ossimConnectableObject> writerInput =
                        duplicate(widgetInput.get());
                     if ( writerInput.valid() )
                     {
                        //  Set up the view(s) in the container and sync them by firing event.
                        setContainerView(writerInput.get());
                        ossimPropertyEvent propEvt(writerInput.get());
                        writerInput->fireEvent(propEvt);
                        writerInput->propagateEventToOutputs(propEvt);
                        
                        // Set the area of interest using a cutter.
                        ossimGeoPolyCutter* cutter = new ossimGeoPolyCutter;
                        cutter->setPolygon(m_outputGeoPolygon);
                        cutter->setView(m_outputView.get());
                        
                        // Put the cutter into the chain at the end.
                        ossimImageChain* chain =
                           dynamic_cast<ossimImageChain*>(writerInput.get());
                        if (chain)
                        {
                           chain->addFirst(cutter);
                        }
                        else
                        {
                           return;
                        }
                        
                        // Connect writer to the cutter.
                        m_writer->connectMyInputTo(0, writerInput.get());
                        
                        // Set the output file.
                        m_writer->setOutputName(m_outputFile);
                        
                        // Initialize.
                        m_writer->initialize();
                        
                        // Make a progress dialog.
                        ossimGui::ProgressDialog* pd = new ossimGui::ProgressDialog(this);
                        pd->progressWidget()->setObject( m_writer.get() );
                        
                        // pd->setMinimumDuration(250); Update 4 times a second.
                        
                        //---
                        // Connect the progress dialog's signal "canceled()" up to our slot
                        // "saveCanceled()" so that we can tell the writer to abort.
                        //---
                        // connect( pd, SIGNAL(canceled()), this, SLOT(abortClicked()) );
                        
                        // ossimProcessListener* pl = dynamic_cast<ossimProcessListener>(pd);
                        // if (pl)
                        // {
                        // Make the progress dialog a listener to the writer.
                        // m_writer->addListener(pl);
                        // }
                        
                        // Set up the progress dialog...
                        QString qs = "Processing file ";
                        qs += m_outputFile.c_str();
                        // pd->progressWidget()->setLabelText(qs);
                        pd->show();
                        
                        // Process the tile...
                        bool exceptionCaught = false;
                        try
                        {
                           m_writer->execute();
                        }
                        catch(std::exception& e)
                        {
                           pd->close();
                           QString caption = "Exception caught!\n";
                           QString text = e.what();
                           QMessageBox::information( this,
                                                     caption,
                                                     text,
                                                     QMessageBox::Ok );
                           exceptionCaught = true;
                        }
                        catch (...)
                        {
                           pd->close();
                           QString caption = "Unknown exception caught!\n";
                           QString text = "";
                           QMessageBox::information( this,
                                                     caption,
                                                     text,
                                                     QMessageBox::Ok );
                           exceptionCaught = true;
                        }
                        
                        // Close and disconnect for next run.
                        m_writer->close();
                        m_writer->disconnectAllInputs();
                        cutter = 0;
                        writerInput = 0;
                        
                        if (exceptionCaught)
                        {
                           removeFile(); 
                        }
                        // else if (pd->wasCanceled())
                        // {
                        //    pd->close();
                        //    removeFile();
                        // }
                        
                        // if (pl)
                        // {
                        // m_writer->removeListener(pl);
                        // }
                        
                        // Cleanup...
                        delete pd;
                        pd = 0;
                        if(m_outputFile.exists())
                        {
                           if (traceDebug())
                           {
                              ossimNotify(ossimNotifyLevel_DEBUG)
                                 << "Add to datamanager autoload..." << endl;
                           }
                           // ossimQtAddImageFileEvent event(m_outputFile);
                           // ossimQtApplicationUtility::sendEventToRoot(this,
                           //                                            &event); 
                        }
                     }
                  }
               }
               else
               {
                  QString caption("Notice:");
                  QString text = "Your output file ends with \".spec\"\n";
                  text += "Please select a new output file.";
                  
                  // Give the user a warning.
                  QMessageBox::warning( this,
                                        caption,
                                        text,
                                        QMessageBox::Ok,
                                        QMessageBox::NoButton);
                  m_outputFileLineEdit->setText("");
               }
            }
            else
            {
               QString caption("Notice:");
               QString text = "Your output file cannot be one of the input files!\n";
               text += "Please select a new output file.";
               
               // Give the user a warning.
               QMessageBox::warning( this,
                                     caption,
                                     text,
                                     QMessageBox::Ok,
                                     QMessageBox::NoButton);
               m_outputFileLineEdit->setText("");
            }
         }
         else
         {
            m_outputFileLineEdit->setText("");
         }
      }
   }
   
} // End: ChipperDialog::runIgenPushButtonClicked()

void ossimGui::ChipperDialog::removeFile() const
{
   // 0 == success, -1 failure
   int status = ossimFilename::remove( m_outputFile.c_str());
   
   QString caption = "Processing of file aborted!";
   QString text = "File:  ";
   text += m_outputFile.c_str();
   if (status == 0)
   {
      text += "\nFile removed successfully...";
   }

   QMessageBox msgBox;
   msgBox.setText( text );
   msgBox.exec();
   
   //QMessageBox::information( this,
   //                          caption,
   //                          text,
   //                           QMessageBox::Ok );
}

bool ossimGui::ChipperDialog::isInChain(const ossimFilename& outputFile) const
{
   bool result = false;

   if ( errorStatus() == ossimGui::ChipperDialog::OK )
   {
      // First get the keyword list for the chain.
      ossimKeywordlist kwl;
      m_input->saveStateOfAllInputs(kwl);
      
      if (traceDebug())
      {
         ossimNotify(ossimNotifyLevel_DEBUG)
            << "ossimGui::ChipperDialog::isInChain DEBUG:"
            << "\nInput keyword list\n" << kwl
            << "\noutputFile:  " << outputFile 
            << std::endl;
      }
      
      // Check for filenames.
      std::vector<ossimString> keys =
         kwl.findAllKeysThatContains(ossimKeywordNames::FILENAME_KW);
      ossim_uint32 index = 0;
      for (index = 0; index < keys.size(); ++index)
      {
         // Find the filename for key.
         ossimFilename f = kwl.find(keys[index]);
         
         if (traceDebug())
         {
            ossimNotify(ossimNotifyLevel_DEBUG)
               << "ossimGui::ChipperDialog::isInChain DEBUG:"
               << "keys[" << index << "]:  " << keys[index]
               << "\ninput file for key:  " << f << std::endl;
         }
      
         if (f == outputFile)
         {
            result = true;
         }

         if ( !result )
         {
            keys.clear();

            // Check for overviews.
            keys = kwl.findAllKeysThatContains(ossimKeywordNames::OVERVIEW_FILE_KW);
            for (index = 0; index < keys.size(); ++index)
            {
               // Find the overview file for key.
               f = kwl.find(keys[index]);
               
               if (traceDebug())
               {
                  ossimNotify(ossimNotifyLevel_DEBUG)
                     << "ossimGui::ChipperDialog::isInChain DEBUG:"
                     << "keys[" << index << "]:  " << keys[index]
                     << "\ninput overview file for key:  " << f << std::endl;
               }

               if (f == outputFile)
               {
                  result = true;
               }
            }
         }
      }
   }

   return result;
   
} // End: ossimGui::ChipperDialog::isInChain( ... )

ossimRefPtr<ossimConnectableObject> ossimGui::ChipperDialog::duplicate(
   const ossimConnectableObject* obj) const
{
   ossimRefPtr<ossimConnectableObject> connectable = 0;
   
   if(obj)
   {
      ossimKeywordlist kwl;
      obj->saveState(kwl);

      ossimRefPtr<ossimObject> tempObj =
         ossimObjectFactoryRegistry::instance()->createObject(kwl);
      
      if ( tempObj.valid() )
      {
         connectable = dynamic_cast<ossimConnectableObject*>( tempObj.get() );
         
         if ( connectable.valid() )
         {
            ossimConnectableContainerInterface* inter =
               dynamic_cast<ossimConnectableContainerInterface*>( connectable.get() );
            
            if(inter)
            {
               inter->makeUniqueIds();
            }
            else
            {
               connectable->setId( ossimIdManager::instance()->generateId() );
            }
            
            for(ossim_uint32 i = 0; i < obj->getNumberOfInputs(); ++i)
            {
               if( obj->getInput(i) )
               {
                  ossimRefPtr<ossimConnectableObject> newInput = duplicate( obj->getInput(i) );
                  if( newInput.valid() )
                  {
                     connectable->connectMyInputTo( newInput.get() );
                  }
               }
            }
         }
      }
   }
   
   return connectable;
   
} // End: ossimGui::ChipperDialog::duplicate

void ossimGui::ChipperDialog::gsdLineEditReturnPressed()
{
   if ( !m_callBackDisabled )
   {
      ossimString s = m_gsdLineEdit->text().toStdString();
      m_gsd.x = s.toDouble();
      m_gsd.y = m_gsd.x;
      recalculateRect();
   }
}

void ossimGui::ChipperDialog::linesLineEditReturnPressed()
{
   if ( !m_callBackDisabled )
   {
      ossimString s = m_linesLineEdit->text().toStdString();
      m_lines = s.toUInt32();
      recalculateRect();
   }
}

void ossimGui::ChipperDialog::samplesLineEditReturnPressed()
{
   if ( !m_callBackDisabled )
   {
      ossimString s = m_samplesLineEdit->text().toStdString();
      m_samples = s.toUInt32();
      recalculateRect();
   }
}

void ossimGui::ChipperDialog::outputFileLineEditReturnPressed()
{
   if ( !m_callBackDisabled )
   {
      m_outputFile = m_outputFileLineEdit->text().toStdString();
   } 
}

void ossimGui::ChipperDialog::sceneRectPushButtonClicked()
{
   setSceneBoundingRect();
}

void ossimGui::ChipperDialog::imageWidgetDestroyed()
{
}

void ossimGui::ChipperDialog::editWriterPushButtonClicked()
{
   if ( m_writer.valid() )
   {
      PropertyEditorDialog* propEditor = new PropertyEditorDialog( this );
      propEditor->setObject( m_writer.get() );
      propEditor->exec();

      delete propEditor;
      propEditor = 0;

      // In case user changed writer "image_type" say from tile_tiff to tiff_strip.
      updateOutputTypeFromWriter();
   }
}

void ossimGui::ChipperDialog::updateOutputTypeFromWriter()
{
   if ( m_writer.valid() && m_outputTypeComboBox )
   {
      ossimRefPtr<ossimProperty> imageTypeProp =
         m_writer->getProperty( ossimString(ossimKeywordNames::IMAGE_TYPE_KW) );
      if ( imageTypeProp.valid() )
      {
         if ( imageTypeProp->getName() == ossimKeywordNames::IMAGE_TYPE_KW )
         {
            QString imageType = imageTypeProp->valueToString().c_str();
            if ( imageType.size() )
            {
               if ( imageType != m_outputTypeComboBox->currentText() )
               {
                  // Update dialog:
                  for (int index = 0; index < m_outputTypeComboBox->count(); ++index)
                  {
                     if ( m_outputTypeComboBox->itemText(index) == imageType )
                     {
                        m_outputTypeComboBox->setCurrentIndex( index );
                        break;
                     }
                  }
               }
            }
         }
      }
   }
}

void ossimGui::ChipperDialog::outputTypeComboBoxActivated( const QString & type )
{
   createWriter( type );
}

void ossimGui::ChipperDialog::buildOutputTypeComboBox()
{
   if ( m_outputTypeComboBox )
   {
      m_outputTypeComboBox->clear();

      std::vector<ossimString> writerList;
      ossimImageWriterFactoryRegistry::instance()->getImageTypeList(writerList);
      
      std::vector<ossimString>::const_iterator i = writerList.begin();
      while (i != writerList.end())
      {
         QString qs = (*i).c_str();
         m_outputTypeComboBox->addItem(qs);
         ++i;
      }

      // Set the default output type.
      const QString DEFAULT_WRITER_TYPE = "tiff_tiled_band_separate";
      for (int index = 0; index < m_outputTypeComboBox->count(); ++index)
      {
         if (m_outputTypeComboBox->itemText(index) == DEFAULT_WRITER_TYPE)
         {
            m_outputTypeComboBox->setCurrentIndex( index );
            break;
         }
      }
      
      // Make a writer so the user can edit it.
      createWriter( getWriterString() );
   }
   
} // End: ossimGui::ChipperDialog::buildOutputTypeComboBox()

void ossimGui::ChipperDialog::createWriter(const QString& type)
{
   if ( m_outputTypeComboBox )
   {
      // Get the writer type.
      ossimString os = type.toStdString();
      
      // Make the writer.
      ossimRefPtr<ossimImageFileWriter> writer =
         ossimImageWriterFactoryRegistry::instance()->createWriter(os);
      if ( writer.valid() )
      {
         if ( m_writer.valid() )
         {         
            if (writer->getClassName() != m_writer->getClassName())
            {
               m_writer = writer;
            }
            else
            {
               //---
               // Same writer but output image type may be different:
               // Example tiff has four different output types.
               //---
               m_writer->setOutputImageType(writer->getOutputImageTypeString());
               
               writer = 0; // Zero out for below code:
            }
         }
         else
         {
            m_writer = writer;
         }

         if ( writer.valid() )
         {  
            //---
            // Set the writer to create overviews and a histogram as the default.
            // The user can override this with the  "edit writer" interface.
            //---
            m_writer->setWriteOverviewFlag(true);
            m_writer->setWriteHistogramFlag(true);
         }
      }
   }
   
} // ossimGui::ChipperDialog::createWriter( type )

QString ossimGui::ChipperDialog::getWriterString() const 
{
   QString result = "";
   if ( m_outputTypeComboBox )
   {
      result = m_outputTypeComboBox->currentText();
   }
   return result;
}

ossim_uint32 ossimGui::ChipperDialog::getLines() const
{
   ossim_uint32 result = 0;
   if ( m_widget && m_windowView.get() )
   {
      ossimIrect rect = m_annotator.getRoiRect();
      if (rect.isNan() == false )
      {
         double size = (rect.height() * m_windowView->getMetersPerPixel().y) / m_gsd.y;
         result = static_cast<ossim_uint32>(floor(size));
      }
   }
   return result;
}

ossim_uint32 ossimGui::ChipperDialog::getSamples() const
{
   ossim_uint32 result = 0;
   if ( m_widget && m_windowView.get() )
   {
      ossimIrect rect = m_annotator.getRoiRect();
      if (rect.isNan() == false )
      {
         double size = (rect.width() * m_windowView->getMetersPerPixel().x) / m_gsd.x;
         result = static_cast<ossim_uint32>(floor(size));
      }
   }
   return result;
}

void ossimGui::ChipperDialog::getBounds(
   ossim_uint32& lines, ossim_uint32& samples) const
{
   if ( m_widget && m_windowView.get() )
   {
      ossimIrect rect = m_annotator.getRoiRect();
      if (rect.isNan() == false )
      {
         double size = 0.0;
         if ( m_gsd.y )
         {
            size = (rect.height() * m_windowView->getMetersPerPixel().y) / m_gsd.y;
            lines = static_cast<ossim_uint32>(floor(size));
         }

         if ( m_gsd.x )
         {
            size = (rect.width() * m_windowView->getMetersPerPixel().x) / m_gsd.x;
            samples = static_cast<ossim_uint32>(floor(size));
         }
      }
   }
}

void ossimGui::ChipperDialog::setWidgetRect(const ossimIrect& rect)
{
   m_annotator.setRoiRect(rect);
}

// Slot:
void ossimGui::ChipperDialog::syncView()
{
   setView();
}

