#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/AdjustableParameterEditor.h>
#include <ossimGui/BandSelectorEditor.h>
#include <ossimGui/BrightnessContrastEditor.h>
#include <ossimGui/ChipperDialog.h>
#include <ossimGui/CopyChainVisitor.h>
#include <ossimGui/Event.h>
#include <ossimGui/ExportImageDialog.h>
#include <ossimGui/HistogramRemapperEditor.h>
#include <ossimGui/HsiRemapperEditor.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/ImageViewManipulator.h>
#include <ossimGui/PolygonRemapperDialog.h>
#include <ossimGui/PositionInformationDialog.h>
#include <ossimGui/SetViewVisitor.h>
#include <ossim/base/ossimAdjustableParameterInterface.h>
#include <ossim/base/ossimGeoidManager.h>
#include <ossim/base/ossimVisitor.h>
#include <ossim/elevation/ossimElevManager.h>
#include <ossim/imaging/ossimFilterResampler.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimGeoPolyCutter.h>
#include <ossim/projection/ossimImageViewAffineTransform.h>
#include <ossim/projection/ossimImageViewProjectionTransform.h>
#include <ossim/projection/ossimImageViewTransform.h>
#include <ossim/projection/ossimMapProjection.h>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QtGui/QComboBox>
#include <QtGui/QMdiArea>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QStatusBar>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <iostream>
#include <fstream>

ossimGui::ImageActions::Visitor::Visitor()
:ossimVisitor(ossimVisitor::VISIT_INPUTS | ossimVisitor::VISIT_CHILDREN)   
{
   reset();
}

void ossimGui::ImageActions::Visitor::reset()
{
   ossimVisitor::reset();
   m_imageAdjustments.clear();
   m_isProjected = false;
   m_isAffine    = false;
   m_imageHandlers.clear();
   m_bandSelectors.clear();
   m_histogramRemappers.clear();
   m_hsiRemappers.clear();
   m_imageRenderers.clear();
   m_viewInterfaces.clear();
   m_scalarRemappers.clear();
   m_hsiRemappers.clear();
   m_brightnessContrastSources.clear();
   m_containers.clear();
}

void ossimGui::ImageActions::Visitor::visit(ossimConnectableObject* obj)
{
   if(!hasVisited(obj))
   {
      ossimVisitor::visit(obj);
      if(obj->canCastTo("ossimViewInterface"))
      {
         m_viewInterfaces.push_back(obj);
      }
      
      
      if(obj->canCastTo("ossimBandSelector"))
      {
         m_bandSelectors.push_back(obj);
      }
      else if(obj->canCastTo("ossimImageHandler"))
      {
         ossimImageSource* is = dynamic_cast<ossimImageSource*> (obj);
         m_imageHandlers.push_back(obj);
         if(is)
         {
            ossimRefPtr<ossimImageGeometry> geom = is->getImageGeometry();
            if(geom.valid()&&dynamic_cast<ossimAdjustableParameterInterface*>(geom->getProjection()))
            {
               m_imageAdjustments.push_back(obj);
            }
         }
      }
      else if(obj->canCastTo("ossimScalarRemapper"))
      {
         m_scalarRemappers.push_back(obj);
      }
      else if(obj->canCastTo("ossimBrightnessContrastSource"))
      {
         m_brightnessContrastSources.push_back(obj);
      }
      else if(obj->canCastTo("ossimHistogramRemapper"))
      {
         m_histogramRemappers.push_back(obj);
      }
      else if(obj->canCastTo("ossimHsiRemapper"))
      {
         m_hsiRemappers.push_back(obj);
      }
      else if(obj->canCastTo("ossimImageRenderer"))
      {
         ossimImageRenderer* renderer = dynamic_cast<ossimImageRenderer*>(obj);
         if(renderer)
         {
            m_imageRenderers.push_back(obj);
            if(dynamic_cast<ossimImageViewProjectionTransform*> (renderer->getImageViewTransform()))
            {
               m_isProjected = true;
            }
            else if(dynamic_cast<ossimImageViewAffineTransform*> (renderer->getImageViewTransform()))
            {
               m_isAffine = true;
            }
         }
      }
      else if(obj->canCastTo("ossimConnectableContainerInterface"))
      {
         m_containers.push_back(obj);
      }
      
   }
}

ossimGui::ImageActions::ImageActions()
:m_widget(0)
{
   m_syncType = View::SYNC_TYPE_NONE;
   m_currentScenePoint.makeNan();
}

void ossimGui::ImageActions::exportImage()
{
   // need support to output the 8 bit chain layer.  
   // we will do that later
   //
   ExportImageDialog* dialog = new ExportImageDialog(m_widget);
   dialog->setObject(m_widget->connectableObject()->getInput(0));
   dialog->exec();
}

void ossimGui::ImageActions::saveAs()
{
   ChipperDialog* dialog = new ChipperDialog( m_widget );
   dialog->setAttribute(Qt::WA_DeleteOnClose);
   // connect(this, SIGNAL(syncView(View&)), dialog, SLOT(syncView(View&)));
   connect(this, SIGNAL(syncView(View&)), dialog, SLOT(syncView()));
   
   if ( dialog->errorStatus() == ossimGui::ChipperDialog::OK )
   {
      dialog->show();
      // dialog->exec();
   }
}

void ossimGui::ImageActions::exportKeywordlist()
{
   if(m_widget->connectableObject()->getInput(0))
   {
      QString fileName = QFileDialog::getSaveFileName(NULL, "Export Image");

      ossimFilename file = fileName.toAscii().data();
      CopyChainVisitor visitor;

      if(!file.empty())
      {
         std::ofstream ostr(file.c_str());
         m_widget->connectableObject()->getInput(0)->accept(visitor);

         visitor.kwl().writeToStream(ostr);
      }
   }

}

void ossimGui::ImageActions::editBandSelector()
{
   if(!m_visitor.m_bandSelectors.empty())
   {
      BandSelectorEditor* editor = new BandSelectorEditor(m_widget);
      editor->setObject(m_visitor.m_bandSelectors[0].get());
      editor->show();
   }
}

void ossimGui::ImageActions::editHsiAdjustments()
{
   if(!m_visitor.m_hsiRemappers.empty())
   {
      HsiRemapperEditor* editor = new HsiRemapperEditor(m_widget);
      editor->setObject(m_visitor.m_hsiRemappers[0].get());
      editor->show();
      //std::cout << "Editing HSI adjustments === " << m_visitor.m_hsiRemappers[0].get() << std::endl;
   }
}

void ossimGui::ImageActions::editHistogramRemapper()
{
   if(!m_visitor.m_histogramRemappers.empty())
   {
      HistogramRemapperEditor* editor = new HistogramRemapperEditor(m_widget);
      editor->setObject(m_visitor.m_histogramRemappers[0].get());
      editor->show();
   }
}

void ossimGui::ImageActions::editBrightnessContrast()
{
   if(!m_visitor.m_brightnessContrastSources.empty())
   {
      BrightnessContrastEditor* editor = new BrightnessContrastEditor(m_widget);
      editor->setObject(m_visitor.m_brightnessContrastSources[0].get());
      editor->show();
   }
}

void ossimGui::ImageActions::showPolygonRemapper()
{
   if ( m_widget )
   {
      ossimRefPtr<ConnectableImageObject> inputConnection = m_widget->connectableObject();
      if ( inputConnection.valid() )
      {
         ossimTypeNameVisitor visitor(ossimString("ossimGeoPolyCutter"),
                                      true,
                                      ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
         
         inputConnection->accept(visitor);
         
         
         ossimRefPtr<ossimGeoPolyCutter> polyCutter = 0; 
         
         if ( visitor.getObjects().size() )
         {
            polyCutter = visitor.getObjectAs<ossimGeoPolyCutter> ( 0 );
         }
         else
         {
            ossimRefPtr<ossimImageGeometry> geom = m_widget->getGeometry();
            if ( geom.valid() )
            {
               polyCutter = new ossimGeoPolyCutter;

               polyCutter->setView( geom.get() );
               
               // Set to null inside:
               polyCutter->setCutType( ossimGeoPolyCutter::OSSIM_POLY_NULL_INSIDE );
               
               //ossimRefPtr<ossimImageChain> ic = dynamic_cast<ossimImageChain*> (inputConnection.get());
               
               ossimTypeNameVisitor icVisitor(ossimString("ossimImageChain"),
                                              true,
                                              ossimVisitor::VISIT_CHILDREN|ossimVisitor::VISIT_INPUTS);
               
               inputConnection->accept(icVisitor);
               if (icVisitor.getObjects().size() )
               {
                  ossimRefPtr<ossimImageChain> imageChain = icVisitor.getObjectAs<ossimImageChain> ( 0 );
                  
                  if (imageChain.valid() )
                  {
                     imageChain->addFirst(polyCutter.get() );
                  }
               }
            }
               
         }

         if ( polyCutter.valid() )
         {
            PolygonRemapperDialog* editor = new PolygonRemapperDialog( m_widget );
            editor->setWidget( m_widget );
            editor->setPolyCutter( polyCutter.get() );
            
            connect( m_widget, SIGNAL( track(const ossimDpt& ) ),
                     editor, SLOT(track(const ossimDpt&)));
            connect( m_widget, SIGNAL( mousePress(QMouseEvent*, const ossimDpt&) ),
                     editor, SLOT(mousePress(QMouseEvent*, const ossimDpt&)));
            
            editor->show();
         }
         
      } // if ( inputConnection.valid() )
      
   } // if ( m_widget.valid() )
   
} // End: ossimGui::ImageActions::showPolygonRemapper()

void ossimGui::ImageActions::showPositionInformation()
{
   PositionInformationDialog* editor = new PositionInformationDialog( m_widget );
   editor->setWidget( m_widget );
   connect( m_widget, SIGNAL( track(const ossimDpt& ) ), editor, SLOT(track(const ossimDpt&)));
   editor->show();
}

void ossimGui::ImageActions::editGeometryAdjustments()
{
   if(!m_visitor.m_imageAdjustments.empty())
   {
      AdjustableParameterEditor* editor = new AdjustableParameterEditor(m_widget);
      editor->setObject(m_visitor.m_imageAdjustments[0].get());
      editor->show();
   }
}

void ossimGui::ImageActions::editView()
{
   if(!m_visitor.m_imageRenderers.empty())
   {
      //std::cout << "Editing view === " << m_visitor.m_imageRenderers[0].get() << std::endl;
   }
}

void ossimGui::ImageActions::fitToWindow()
{
   ossimDrect currentViewSize = m_widget->connectableObject()->getBounds();
   m_widget->manipulator()->fit();
   m_currentScenePoint = m_widget->viewportBoundsInSceneSpace().midPoint();
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::fullRes()
{
   m_widget->manipulator()->fullRes();
   m_currentScenePoint = m_widget->viewportBoundsInSceneSpace().midPoint();
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::zoomIn(double factor)
{
   m_widget->manipulator()->zoomIn(factor);
   m_currentScenePoint = m_widget->viewportBoundsInSceneSpace().midPoint();
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::zoomOut(double factor)
{
   m_widget->manipulator()->zoomOut(factor);
   m_currentScenePoint = m_widget->viewportBoundsInSceneSpace().midPoint();
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::syncingOptionsChanged(const QString& value)
{
   if(value == "<Select Syncing>" || value == "None")
   {
      m_syncType = View::SYNC_TYPE_NONE;
   }
   else if(value == "Cursor")
   {
      m_syncType = View::SYNC_TYPE_CURSOR;
   }
   else if(value == "Position")
   {
      m_syncType = View::SYNC_TYPE_POSITION;
   }
   else if(value == "Full")
   {
      m_syncType = View::SYNC_TYPE_ALL;
      m_syncType = m_syncType&(~View::SYNC_TYPE_CURSOR);
   }
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::layerOptionsChanged(int idx)
{
   m_widget->setMultiLayerAlgorithm(idx);
}

void ossimGui::ImageActions::track(const ossimDpt& scenePoint)
{
   m_currentScenePoint = scenePoint;
   setupAndExecuteSyncing();
}

void ossimGui::ImageActions::interpolationTypeChanged(const QString& value)
{
   if(!m_visitor.m_imageRenderers.empty())
   {
      ossimVisitor::ListRef::iterator rendererIterator = m_visitor.m_imageRenderers.begin();
      while(rendererIterator != m_visitor.m_imageRenderers.end())
      {
         ossimPropertyInterface* propertyInterface = dynamic_cast<ossimPropertyInterface*> (rendererIterator->get());
         if(propertyInterface)
         {
            propertyInterface->setProperty("filter_type", value.toAscii().data());
         }
         ++rendererIterator;
      }
      rendererIterator = m_visitor.m_imageRenderers.begin();
      ossimEventVisitor visitor(new ossimRefreshEvent());
      while(rendererIterator != m_visitor.m_imageRenderers.end())
      {
         (*rendererIterator)->accept(visitor);
         ++rendererIterator;
      }
   }
   m_resamplerType = value;
   if(m_syncType&View::SYNC_TYPE_RESAMPLER)
   {
      setupAndExecuteSyncing();
   }
}

void ossimGui::ImageActions::addActions(QMainWindow* mainWindow)
{
   if(!m_widget) return;
   // collect all the types we want direct pointers to
   //
   m_visitor.reset();
   m_widget->connectableObject()->accept(m_visitor);
   
   
   if(!mainWindow) return;
   QMenuBar* menuBar = mainWindow?mainWindow->menuBar():0;

   QMenu* imageMenu = menuBar->findChild<QMenu*>( "imageMenu");
   QAction* action = 0;
   if(!imageMenu)
   {
      imageMenu = new QMenu("Image", menuBar);
      imageMenu->setObjectName("imageMenu");
      QMenu* windowMenu = menuBar->findChild<QMenu*>("windowMenu");
      if(windowMenu)
      {
         action = menuBar->insertMenu(windowMenu->menuAction(), imageMenu);
      }
      else 
      {
      }
   }
   
   action = imageMenu->findChild<QAction*>("exportImageAction");
   if(!action)
   {
      action = imageMenu->addAction("Export");
      action->setObjectName("exportImageAction");
      action->setEnabled(true);
      
   }
   connect(action, SIGNAL(triggered(bool)),this, SLOT(exportImage()));

   action = imageMenu->findChild<QAction*>("saveAsAction");
   if(!action)
   {
      action = imageMenu->addAction("Save as");
      action->setObjectName("saveAsAction");
      action->setEnabled(true);
      
   }
   connect(action, SIGNAL(triggered(bool)),this, SLOT(saveAs()));
  
   action = imageMenu->findChild<QAction*>("exportKeywordListAction");
   if(!action)
   {
      action = imageMenu->addAction("Export Keywordlist");
      action->setObjectName("exportKeywordListAction");
      action->setEnabled(true);
   }
   connect(action, SIGNAL(triggered(bool)),this, SLOT(exportKeywordlist()));

   imageMenu->addSeparator();

   action = imageMenu->findChild<QAction*>("imageSelectBandsAction");
   if(!action)
   {
      action = imageMenu->addAction("Band Selection");
      action->setObjectName("imageSelectBandsAction");
   }
   if(!m_visitor.m_bandSelectors.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editBandSelector()));
      action->setEnabled(true);
   }
   else
   {
      action->setEnabled(false);
   }
 
   action = imageMenu->findChild<QAction*>("imageBrightnessContrastAction");
   if(!action)
   {
      action = imageMenu->addAction("Brightness Contrast");
      action->setObjectName("imageBrightnessContrastAction");
   }
   if(!m_visitor.m_brightnessContrastSources.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editBrightnessContrast()));
      action->setEnabled(true);
   }
   else 
   {
      action->setEnabled(false);
   }

   action = imageMenu->findChild<QAction*>("imageGeometryAdjustmentAction");
   if(!action)
   {
      action = imageMenu->addAction("Geometry Adjustment");
      action->setObjectName("imageGeometryAdjustmentAction");
   }
   if(!m_visitor.m_imageAdjustments.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editGeometryAdjustments()));
      action->setEnabled(true);
   }
   else 
   {
      action->setEnabled(false);
   }
   
   action = imageMenu->findChild<QAction*>("imageHistogramRemapperAction");
   if(!action)
   {
      action = imageMenu->addAction("Histogram Remapper");
      action->setObjectName("imageHistogramRemapperAction");
   }
   if(!m_visitor.m_histogramRemappers.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editHistogramRemapper()));
      action->setEnabled(true);
   }
   else 
   {
      action->setEnabled(false);
   }

   action = imageMenu->findChild<QAction*>("polygonRemapperAction");
   if(!action)
   {
      action = imageMenu->addAction("Polygon Remapper");
      action->setObjectName("polygonRemapperAction");
      action->setEnabled( true );
   }
   connect(action, SIGNAL(triggered(bool)),this, SLOT( showPolygonRemapper() ));
   
   action = imageMenu->findChild<QAction*>("imageHsiAdjustmentsAction");
   if(!action)
   {
      action = imageMenu->addAction("HSI Adjustments");
      action->setObjectName("imageHsiAdjustmentsAction");
   }
   if(!m_visitor.m_hsiRemappers.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editHsiAdjustments()));
      action->setEnabled(true);
   }
   else 
   {
      action->setEnabled(false);
   }
   
   imageMenu->addSeparator();

   action = imageMenu->findChild<QAction*>("positionInformationAction");
   if(!action)
   {
      action = imageMenu->addAction("Position Information");
      action->setObjectName("positionInformationAction");
      action->setEnabled( true );
   }
   connect(action, SIGNAL(triggered(bool)),this, SLOT( showPositionInformation() ));
   
   action = imageMenu->findChild<QAction*>("imageViewAction");
   if(!action)
   {
      action = imageMenu->addAction("View");
      action->setObjectName("imageViewAction");
   }
   if(!m_visitor.m_imageRenderers.empty())
   {
      connect(action, SIGNAL(triggered(bool)),this, SLOT(editView()));
      action->setEnabled(true);
   }
   else 
   {
      action->setEnabled(false);
   }
   
   /****************************** setup tool bar ******************************/
   QToolBar* toolbar = mainWindow?mainWindow->findChild<QToolBar*>( "imageToolBar"):0;
   
   if(!toolbar&&mainWindow)
   {
      toolbar = new QToolBar();
      toolbar->setObjectName("imageToolBar");
      mainWindow->addToolBar(toolbar);
   }
   QComboBox* interpolationType = toolbar->findChild<QComboBox*>("interpolationType");
   QToolButton* fitToWindow = toolbar->findChild<QToolButton*>("fitToWindowButton");
   QToolButton* fullResButton = toolbar->findChild<QToolButton*>("fullResButton");
   QToolButton* zoomInButton = toolbar->findChild<QToolButton*>("zoomInButton");
   QToolButton* zoomOutButton = toolbar->findChild<QToolButton*>("zoomOutButton");
   QComboBox* syncingOptions = toolbar->findChild<QComboBox*>("syncingOptions");
   if(!m_visitor.m_imageRenderers.empty())
   {
      ossimImageRenderer* renderer = dynamic_cast<ossimImageRenderer*>(m_visitor.m_imageRenderers[0].get());
      // already have a toolbar there
      //QLabel* interpolationTypeLabel = 0;
      if(!interpolationType)
      {
         interpolationType = new QComboBox();
         std::vector<ossimString> filterTypes;
         ossimFilterResampler filterResampler;
         filterResampler.getFilterTypes(filterTypes);
         interpolationType->setObjectName("interpolationType");
         ossim_uint32 idx = 0;
         ossimString currentFilterValue = renderer->getPropertyValueAsString("filter_type").c_str();
         // ossim_uint32 savedIdx = 0;
         interpolationType->addItem("<Select Interpolation>");
         for(idx =0; idx < filterTypes.size();++idx)
         {
            interpolationType->addItem(filterTypes[idx].c_str());
         }
        // interpolationTypeLabel = new QLabel("Interpolation Type:");
         //interpolationTypeLabel->setObjectName("interpolationTypeLabel");
        // toolbar->addWidget(interpolationTypeLabel);
         toolbar->addWidget(interpolationType);
      }
      else 
      {
         interpolationType->setEnabled(true);

      }
      {
         std::vector<ossimString> filterTypes;
         ossimFilterResampler filterResampler;
         filterResampler.getFilterTypes(filterTypes);
         ossim_uint32 idx = 0;
         ossimString currentFilterValue = renderer->getPropertyValueAsString("filter_type").c_str();
         ossim_int32 savedIdx = -1;
         for(idx =0; idx < filterTypes.size();++idx)
         {
            if(currentFilterValue==filterTypes[idx]) savedIdx = idx;
         }
         interpolationType->setCurrentIndex(savedIdx+1);
         m_resamplerType = interpolationType->currentText();
      }
      connect(interpolationType, SIGNAL(activated (const QString &)), this, SLOT(interpolationTypeChanged(const QString &)));
   }
   else 
   {
      if(interpolationType) interpolationType->setEnabled(false);
   }

   if(!m_visitor.m_viewInterfaces.empty())
   {
      if(!fitToWindow)
      {
         //  fitToWindow = new QPushButton("Fit");
         fitToWindow = new QToolButton();
         fitToWindow->setText("Fit");
         fitToWindow->setObjectName("fitToWindowButton");
         fitToWindow->setIcon(QIcon(":/themes/default/mActionZoomFullExtent.png"));
         toolbar->addWidget(fitToWindow);
      }
      else 
      {
         fitToWindow->setEnabled(true);
      }
      if(!fullResButton)
      {
         fullResButton = new QToolButton();
         fullResButton->setIcon(QIcon(":/themes/default/mActionZoomActual.png"));
         fullResButton->setText("Full Res");
         fullResButton->setObjectName("fullResButton");
         toolbar->addWidget(fullResButton);
      }
      else 
      {
         fullResButton->setEnabled(true);
      }
      if(!zoomInButton)
      {
         zoomInButton = new QToolButton();
         zoomInButton->setIcon(QIcon(":/themes/default/mActionZoomIn.png"));
         zoomInButton->setText("+");
         zoomInButton->setObjectName("zoomInButton");
         toolbar->addWidget(zoomInButton);
      }
      else 
      {
         zoomInButton->setEnabled(true);
      }
      if(!zoomOutButton)
      {
         zoomOutButton = new QToolButton();
         zoomOutButton->setIcon(QIcon(":/themes/default/mActionZoomOut.png"));
         zoomOutButton->setText("-");
         zoomOutButton->setObjectName("zoomOutButton");
         toolbar->addWidget(zoomOutButton);
      }
      else 
      {
         zoomOutButton->setEnabled(true);
      }
      //QLabel* syncingOptionsLabel = 0;
      if(!syncingOptions)
      {
         syncingOptions = new QComboBox();
         syncingOptions->setObjectName("syncingOptions");
         syncingOptions->addItem("<Select Syncing>");
         syncingOptions->addItem("None");
         syncingOptions->addItem("Full");
         syncingOptions->addItem("Position");
         syncingOptions->addItem("Cursor");
         //syncingOptionsLabel = new QLabel("syncing:");
         //syncingOptionsLabel->setObjectName("syncingOptionsLabel");
         //toolbar->addWidget(syncingOptionsLabel);
         toolbar->addWidget(syncingOptions);
      }
      else 
      {
         syncingOptions->setEnabled(true);
         syncingOptionsChanged(syncingOptions->currentText());
      }
      connect(fitToWindow, SIGNAL(clicked(bool)),this, SLOT(fitToWindow()));
      connect(fullResButton, SIGNAL(clicked(bool)),this, SLOT(fullRes()));
      connect(zoomInButton, SIGNAL(clicked(bool)),this, SLOT(zoomIn()));
      connect(zoomOutButton, SIGNAL(clicked(bool)),this, SLOT(zoomOut()));
      connect(syncingOptions, SIGNAL(activated (const QString &)), this, SLOT(syncingOptionsChanged(const QString &)));
   }
   else 
   {
      if(fitToWindow)
      {
         fitToWindow->setEnabled(false);
      }
      if(fullResButton)
      {
         fullResButton->setEnabled(false);
      }
      if(zoomInButton)
      {
         zoomInButton->setEnabled(false);
      }
      if(zoomOutButton)
      {
         zoomOutButton->setEnabled(false);
      }
   }

   QComboBox* layerOptions = toolbar->findChild<QComboBox*>("layerOptions");
   // QLabel* layerOptionsLabel = 0;
   if(!layerOptions)
   {
      layerOptions = new QComboBox();
      layerOptions->setObjectName("layerOptions");
      layerOptions->addItem("<Select Swipe Mode>");
      layerOptions->addItem("Horizontal Swipe");
      layerOptions->addItem("Vertical Swipe");
      layerOptions->addItem("Box Swipe");
      layerOptions->addItem("Circle Swipe");
      //LayerOptions->addItem("Animation");
      //layerOptionsLabel = new QLabel("Layer Options:");
      //layerOptionsLabel->setObjectName("layerOptionsLabel");
      // toolbar->addWidget(layerOptionsLabel);
      toolbar->addWidget(layerOptions);
   }
   else 
   {
      layerOptions->setEnabled(true);
   }
   layerOptions->setCurrentIndex(m_widget->multiLayerAlgorithmType());

   connect(layerOptions, SIGNAL(activated (int)), this, SLOT(layerOptionsChanged(int)));
   
//   connect(m_widget, SIGNAL( mouseMove(QMouseEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(mouseMove(QMouseEvent*,  const ossimDrect& , const ossimDpt& )));
//   connect(m_widget, SIGNAL( mouseDoubleClick(QMouseEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(mouseDoubleClick(QMouseEvent*,   const ossimDrect& , const ossimDpt& )));
//   connect(m_widget, SIGNAL( mousePress(QMouseEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(mousePress(QMouseEvent*,   const ossimDrect& , const ossimDpt& )));
//   connect(m_widget, SIGNAL( mouseRelease(QMouseEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(mouseRelease(QMouseEvent*,   const ossimDrect& , const ossimDpt& )));
//   connect(m_widget, SIGNAL( wheel(QWheelEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(wheel(QWheelEvent*,   const ossimDrect& , const ossimDpt& )));

   connect(m_widget, SIGNAL( track(const ossimDpt& )),
            this, SLOT(track(const ossimDpt&)));
}

void ossimGui::ImageActions::removeActions(QMainWindow* mainWindow)
{
   disconnect(m_widget, 0, this, 0);

   /********************** REMOVE MENU ****************************/
   QMenuBar* menuBar  = mainWindow->menuBar();
   
   QMenu* imageMenu = menuBar->findChild<QMenu*>("imageMenu");
   if(imageMenu)
   {
      QAction* action = imageMenu->findChild<QAction*>("imageSelectBandsAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("imageHsiAdjustmentsAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("imageHistogramRemapperAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("imageGeometryAdjustmentAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("imageViewAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("imageBrightnessContrastAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("exportImageAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("saveAsAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
      action = imageMenu->findChild<QAction*>("exportKeywordListAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }

      //

      action = imageMenu->findChild<QAction*>("polygonRemapperAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }

      //

      action = imageMenu->findChild<QAction*>("positionInformationAction");
      if(action)
      {
         disconnect(action, 0, this, 0);
      }
   }
   
   /********************** REMOVE TOOLBAR ****************************/
   QToolBar* imageToolBar = mainWindow->findChild<QToolBar*>("imageToolBar");
   
   if(imageToolBar)
   {
      QWidget* widget = imageToolBar->findChild<QWidget*>("interpolationType");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("fitToWindowButton");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("fullResButton");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("zoomInButton");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("zoomOutButton");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("syncingOptions");
      if(widget)
      {
         widget->disconnect(this);
      }
      widget = imageToolBar->findChild<QWidget*>("layerOptions");
      if(widget)
      {
         widget->disconnect(this);
      }
      
      //imageToolBar->hide();
   }

}

void ossimGui::ImageActions::setupAndExecuteSyncing()
{
   ossimRefPtr<ossimImageGeometry> geom = getView();
   if(geom.valid()&&!m_currentScenePoint.hasNans()&&(m_syncType!=View::SYNC_TYPE_NONE))
   {
      int syncType = m_syncType;
      
      View view(syncType, m_currentScenePoint, geom.get());
      view.setResamplerType(m_resamplerType.toAscii().data());
      if(view.geomType() != View::GEOM_TYPE_MAP_PROJECTION)
      {
         view.setSyncType(View::SYNC_TYPE_GEOM, false); // we will for now only synch full scale if its of type map projection
      }
      emit syncView(view);
   }
}

ossimImageGeometry* ossimGui::ImageActions::getView()
{
   return m_widget->getGeometry();
}


ossimGui::ImageMdiSubWindow::ImageMdiSubWindow( QWidget * parent, Qt::WindowFlags flags)
:MdiSubWindowBase(parent, flags)
{
   m_connectableObject = new ConnectableDisplayObject(this);
   m_actions = new ImageActions();
   setAttribute(Qt::WA_DeleteOnClose);
   setGeometry(0,0,512,512);
   setMinimumSize(QSize(64,64));
   m_imageScrollView = new ImageScrollView();
   m_imageScrollView->setConnectableObject(static_cast<ConnectableImageObject*>(m_connectableObject.get()));
   setWidget(m_imageScrollView);
   m_actions->setWidget(m_imageScrollView);
   connect(this, SIGNAL(windowStateChanged ( Qt::WindowStates , Qt::WindowStates  )),this, SLOT(stateChanged(Qt::WindowStates , Qt::WindowStates)));
   m_containerListener = new ContainerListener(this);
//   connect(m_imageScrollWidget, SIGNAL( mouseMove(QMouseEvent*,   const ossimDrect& , const ossimDpt& )), 
//           this, SLOT(mouseMove(QMouseEvent*,  const ossimDrect& , const ossimDpt& )));
}

ossimGui::ImageMdiSubWindow::~ImageMdiSubWindow()
{
   removeListeners();
   if(m_containerListener) delete m_containerListener;
   m_containerListener = 0;
   if(m_connectableObject.valid())
   {
      static_cast<ConnectableDisplayObject*>(m_connectableObject.get())->setDisplay(0);
      m_imageScrollView->setConnectableObject(0);
   }
   if(m_actions)
   {
      delete m_actions;
      m_actions = 0;
   }
}

void ossimGui::ImageMdiSubWindow::setConnectableObject(ConnectableObject* connectable)
{
   if(m_connectableObject.valid())
   {
      m_imageScrollView->setConnectableObject(0);
      m_connectableObject->disconnect();
      static_cast<ConnectableDisplayObject*>(m_connectableObject.get())->setDisplay(0);
   }
   MdiSubWindowBase::setConnectableObject(connectable);
   if(m_connectableObject.valid())
   {
      m_imageScrollView->setConnectableObject(static_cast<ConnectableImageObject*>(m_connectableObject.get()));
   }
}

void	ossimGui::ImageMdiSubWindow::closeEvent(QCloseEvent* /* event */) 
{
   removeItems();
}

void ossimGui::ImageMdiSubWindow::stateChanged( Qt::WindowStates oldState, Qt::WindowStates newState)
{
   if((oldState == Qt::WindowNoState) && (newState & Qt::WindowActive))
   {
      // must be coming from an inactive state.  Analyze the chain we are 
      addItems();
      connect(m_actions, SIGNAL(syncView(View&)), this, SLOT(syncView(View&)));
   }
   else if(newState == Qt::WindowNoState)
   {
      // we must be coming from an active state so remove our dynamic items
      removeItems();
      disconnect(m_actions, 0, this, 0);
      m_actions->visitor().reset();
   }
}

void ossimGui::ImageMdiSubWindow::syncView(View& viewInfo)
{
   if(mdiArea())
   {
      QList<QMdiSubWindow *> wList = mdiArea()->subWindowList();
      QList<QMdiSubWindow *>::iterator currentWindow= wList.begin();
      while(currentWindow != wList.end())
      {
         ossimGui::MdiSubWindowBase* w = dynamic_cast<ossimGui::MdiSubWindowBase*>(*currentWindow);
         if( w && (w!=this) )
         {
            w->sync(viewInfo);
         }
         else if( w == this )
         {
            // if we are the ones syncing then lets do some informational readouts
            // such as RGB values,  GEO coordinates, ... etc
            //
            ossimRefPtr<ossimImageGeometry> geom = viewInfo.geometry();
            if(geom.valid())
            {
               std::ostringstream out;
               
               ossimDrect viewSpaceRect = m_imageScrollView->viewportBoundsInSceneSpace();
               ossimDpt viewPoint = viewInfo.lookPosition();
               ossimDpt eastingNorthing;
               ossimDpt minProjectionPlanePt, maxProjectionPlanePt;
               minProjectionPlanePt.makeNan();
               maxProjectionPlanePt.makeNan();
               ossimRefPtr<ossimMapProjection> mapProj = dynamic_cast<ossimMapProjection*> (geom->getProjection());
               ossimGpt lookGpt = viewInfo.lookPositionAsGpt();
               // ossim_int32 pcsCode = -1;
               if(mapProj.valid())
               {
                  // pcsCode = mapProj->getPcsCode();
                  if(!mapProj->isGeographic())
                  {
                     mapProj->lineSampleToEastingNorthing(
                        viewSpaceRect.ll(), minProjectionPlanePt);
                     mapProj->lineSampleToEastingNorthing(
                        viewSpaceRect.ur(), maxProjectionPlanePt);
                  }
                  else
                  {
                     ossimGpt tempGpt;
                     geom->localToWorld(viewSpaceRect.ll(), tempGpt);
                     minProjectionPlanePt = tempGpt;
                     geom->localToWorld(viewSpaceRect.ur(), tempGpt);
                     maxProjectionPlanePt = tempGpt;
                  }
               }
               else 
               {
                  ossimGpt corners[4];
                  geom->localToWorld(viewSpaceRect.ul(), corners[0]);
                  geom->localToWorld(viewSpaceRect.ur(), corners[1]);
                  geom->localToWorld(viewSpaceRect.lr(), corners[2]);
                  geom->localToWorld(viewSpaceRect.ll(), corners[3]);
                  minProjectionPlanePt.x = (ossim::min(ossim::min(ossim::min(corners[0].lond(),corners[1].lond()), corners[2].lond()), corners[3].lond()));
                  maxProjectionPlanePt.x = (ossim::max(ossim::max(ossim::max(corners[0].lond(), corners[1].lond()), corners[2].lond()), corners[3].lond()));
                  minProjectionPlanePt.y = (ossim::min(ossim::min(ossim::min(corners[0].latd(), corners[1].latd()), corners[2].latd()), corners[3].latd()));
                  maxProjectionPlanePt.y = (ossim::max(ossim::max(ossim::max(corners[0].latd(), corners[1].latd()), corners[2].latd()), corners[3].latd()));
               }
               if(mainWindow())
               {
                  QStatusBar* statusBar = mainWindow()->statusBar();
                  std::ostringstream location;
                  ossim_float64 h = ossimElevManager::instance()->getHeightAboveMSL(lookGpt);
                  ossim_float64 alat = fabs(lookGpt.latd());
                  ossim_float64 alon = fabs(lookGpt.lond());
                  location << ossimString::toString(alat) <<((lookGpt.latd()>=0)?" N":" S")<<  ", " << ossimString::toString(alon)<< ((lookGpt.lond()>=0)?" E":" W") << std::endl;
                  bool fixed = true;
                  location << " MSL: " << ossimString::toString(h, 2, fixed);
                  location << "   x: " << ossimString::toString(viewPoint.x);
                  location << " y: " << ossimString::toString(viewPoint.y);
                  if(statusBar)
                  {
                     statusBar->showMessage(QString(location.str().c_str()));
                  }
                  
               }
#if 0
               ossim_uint32 fixedWidth = 15;
               out<< std::setiosflags( ios::left ) << std::setw(fixedWidth) << "location:" << lookGpt << std::endl;
               if(!minProjectionPlanePt.hasNans()&& !maxProjectionPlanePt.hasNans())
               {
                  out.setf(std::ios::fixed);
                  out << std::setiosflags( ios::left ) << "BBOX="  << minProjectionPlanePt.x << "," << minProjectionPlanePt.y << "," << maxProjectionPlanePt.x << "," << maxProjectionPlanePt.y << std::endl;
                  if(pcsCode > 0)
                  {
                     out << "EPSG:" << pcsCode << "\n";
                  }
               }
#endif
            }
         }
         ++currentWindow;
      }
   }
}

void ossimGui::ImageMdiSubWindow::setJobQueue(ossimJobQueue* q)
{
   m_imageScrollView->setJobQueue(q);
}

ossimGui::ImageScrollView* ossimGui::ImageMdiSubWindow::scrollWidget()
{
   return m_imageScrollView;
}

void ossimGui::ImageMdiSubWindow::sync(View& view)
{
   ossimGpt oldCenter;
   
   oldCenter.makeNan();
   ossimDrect rect = m_imageScrollView->viewportBoundsInSceneSpace();
   
   ossimRefPtr<ossimImageGeometry> widgetGeometry = m_imageScrollView->getGeometry();
   if(widgetGeometry.valid())
   {
       
      widgetGeometry->localToWorld(rect.midPoint(), oldCenter);
   }
   // first let's sync track cursor and position
   //
   ossimRefPtr<ossimImageGeometry> trackGeom = view.geometry();
   ossimRefPtr<ossimImageGeometry> geom      = view.geometry();
   int syncType = view.syncType();
   
   SetViewVisitor visitor(geom.get());
   connectableObject()->accept(visitor);
   
   // if I have no view interfaces to set them I can't sync
   // so fall back to the current projection so we can still cursor track
   // and position track
   //
   if(syncType&View::SYNC_TYPE_GEOM) 
   {
      if(!visitor.getObject()||m_imageScrollView->manipulator()->isAffine())
      {
         syncType = syncType&(~View::SYNC_TYPE_GEOM);
      }
   }
   
   if(!(syncType&View::SYNC_TYPE_GEOM))
   {
      trackGeom = widgetGeometry.get();
   }
   
   if(!(syncType&View::SYNC_TYPE_GEOM))
   {
      visitor.setGeometry(0);
   }
   if(view.syncType()&View::SYNC_TYPE_RESAMPLER)
   {
      visitor.setResamplerType(view.resamplerType());
   }
   if(m_imageScrollView)
   {
      if(m_imageScrollView->manipulator()&&!m_imageScrollView->manipulator()->isAffine())
      {
         if((view.geomType() != View::GEOM_TYPE_UNKNOWN)&&(view.geometry()))
         {
           m_imageScrollView->manipulator()->setObject(view.geometry()->dup());
         }
      }
   }
   // now set the view
   visitor.setView();

   // now set the scroll positions for the view
   //
   bool hasProjection = trackGeom.valid()?trackGeom->getProjection()!=0:false;
   if(trackGeom.valid()&&hasProjection)
   {
      ossimGpt referencePosition = view.lookPositionAsGpt();
   
      
      if(!view.lookPositionValid())
      {
         referencePosition = oldCenter;
      }
      if(!referencePosition.isLatNan()&&!referencePosition.isLonNan())
      {
         ossimDpt viewPoint;
         trackGeom->worldToLocal(referencePosition, viewPoint);
         if(syncType&View::SYNC_TYPE_CURSOR)
         {
            m_imageScrollView->setTrackPoint(viewPoint);
         }
         else
         {
            ossimDpt nanPt;
            nanPt.makeNan();
            m_imageScrollView->setTrackPoint(nanPt);
         }
         if(syncType&View::SYNC_TYPE_POSITION||
            syncType&View::SYNC_TYPE_GEOM)
         {
            m_imageScrollView->centerOn(viewPoint.x, viewPoint.y);
         }
      }
      else 
      {
         
      }
   }
}

void ossimGui::ImageMdiSubWindow::addItems()
{
   QMainWindow* mainWin = mainWindow();
   if(!mainWin) return;
   removeListeners();  
   m_actions->addActions(mainWin);
   addListeners();
}

ossimImageGeometry* ossimGui::ImageMdiSubWindow::getView()
{
   return m_imageScrollView->getGeometry();
}

void ossimGui::ImageMdiSubWindow::addOrSetupToolbar()
{
   QMainWindow* mainWin = mainWindow();
   if(!mainWin) return;
   
   /****************************** End setup tool bar ******************************/
}

void ossimGui::ImageMdiSubWindow::removeItems()
{
   QMainWindow* mainWin = mainWindow();
   if(!mainWin) return;
   removeListeners();
   m_actions->removeActions(mainWin);
   
}

void ossimGui::ImageMdiSubWindow::addListeners()
{
   if(m_actions)
   {
      ossim_uint32 idx = 0;
      ImageActions::Visitor& visitor = m_actions->visitor();
      for(idx = 0; idx < visitor.m_containers.size();++idx)
      {
         ossimListenerManager* manager = dynamic_cast<ossimListenerManager*> (visitor.m_containers[idx].get());
         manager->addListener(m_containerListener);
      }
   }
}

void ossimGui::ImageMdiSubWindow::removeListeners()
{
   if(m_actions)
   {
      ossim_uint32 idx = 0;
      ImageActions::Visitor& visitor = m_actions->visitor();
      for(idx = 0; idx < visitor.m_containers.size();++idx)
      {
         ossimListenerManager* manager = dynamic_cast<ossimListenerManager*> (visitor.m_containers[idx].get());
         manager->removeListener(m_containerListener);
      }
   }
}

void ossimGui::ImageMdiSubWindow::ContainerListener::containerEvent(ossimContainerEvent& /* event */)
{
   QApplication::postEvent(m_window, new QEvent((QEvent::Type)ossimGui::WINDOW_REFRESH_ACTIONS_EVENT_ID));
}

bool ossimGui::ImageMdiSubWindow::event(QEvent* evt)
{
   switch(evt->type())
   {
      //---
      // Getting warning:
      // warning: case value ‘1004’ not in enumerated type ‘QEvent::Type’
      //---
      case ossimGui::WINDOW_REFRESH_ACTIONS_EVENT_ID:
      {
         if(windowState()&Qt::WindowActive)
         {
            removeItems();
            addItems();
         }
         return true;
         break;
      }
      default:
      {
         break;
      }
   }
   return MdiSubWindowBase::event(evt);
}


