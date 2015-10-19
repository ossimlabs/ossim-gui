#include <ossimGui/ExportImageDialog.h>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <ossimGui/CopyChainVisitor.h>
#include <ossimGui/ImageWriterJob.h>
#include <ossimGui/Event.h>
#include <ossimGui/Util.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/base/ossimScalarTypeLut.h>
namespace ossimGui
{
   ExportImageDialog::ExportImageDialog(QWidget* parent)
   :QDialog(parent)
   {
      setupUi(this);
      setAttribute(Qt::WA_DeleteOnClose);
      connect(m_exportAbortButton, SIGNAL(clicked(bool)), this, SLOT(exportAbortClicked(bool)));
      connect(m_closeButton, SIGNAL(clicked(bool)), this, SLOT(closeClicked(bool)));
      connect(m_fileTypes, SIGNAL(activated(int)), this, SLOT(fileTypeActivated(int)));
      connect(m_fileButton, SIGNAL(clicked(bool)), this, SLOT(openFileSaveDialog()));
      m_fileButton->setEnabled(false);
      m_exportingFlag   = false;
   }
   void ExportImageDialog::setObject(ossimObject* obj)
   {
      m_connectable = dynamic_cast<ossimConnectableObject*> (obj);
      
      populateFileTypes();
      populateGeneralInformation();
   }
   void ExportImageDialog::populateFileTypes()
   {
      m_fileTypes->clear();
      QStringList writerList;
      writerList.push_back("<select writer type>");
      Util::imageWriterTypes(writerList);
      m_fileTypes->addItems(writerList);
   }
   void ExportImageDialog::populateGeneralInformation()
   {
      ossimImageSource* imgSource = dynamic_cast<ossimImageSource*> (m_connectable.get());
      if(imgSource)
      {
         ossim_float64 k = 1024;
         ossim_float64 meg = k*k;
         ossim_float64 gig = meg*1024;
         QString w;
         QString h;
         QString bands;
         ossimIrect rect = imgSource->getBoundingRect();
         w.setNum(rect.width());
         h.setNum(rect.height());
         bands.setNum(imgSource->getNumberOfOutputBands());
         m_width->setText(w);
         m_height->setText(h);
         m_bands->setText(bands);
         m_scalarType->setText(ossimScalarTypeLut::instance()->getEntryString(imgSource->getOutputScalarType()).c_str());
         
         ossim_uint64 fileSize = (static_cast<ossim_uint64>(rect.width())*
                                  static_cast<ossim_uint64>(rect.height())*
                                  static_cast<ossim_uint64>(imgSource->getNumberOfOutputBands())*
                                  static_cast<ossim_uint64>(ossim::scalarSizeInBytes(imgSource->getOutputScalarType())));
         QString fileSizeString;
         if(fileSize > gig)
         {
            double value = fileSize/gig;
            fileSizeString.setNum(value, 'f', 2);
            fileSizeString += " GB";
         }
         else if(fileSize > meg)
         {
            double value = fileSize/meg;
            fileSizeString.setNum(value, 'f', 2);
            fileSizeString += " MB";
         }
         else if(fileSize > k)
         {
            double value = fileSize/k;
            fileSizeString.setNum(value, 'f', 2);
            fileSizeString += " KB";
         }
         else
         {
            fileSizeString.setNum(fileSize);
            fileSizeString += " b";
         }
         m_size->setText(fileSizeString);
         
      }
      else 
      {
         m_width->setText("N/A");
         m_height->setText("N/A");
         m_scalarType->setText("N/A");
         m_bands->setText("N/A");
         m_size->setText("N/A");
      }

   }
   void ExportImageDialog::populatePropertyView()
   {
      m_propertyView->setObject(m_writer.get());
   }
   void ExportImageDialog::fileTypeActivated(int idx)
   {
      if(idx == 0)
      {
         m_writer = 0;
         populatePropertyView();
         m_fileButton->setEnabled(false);
      }
      else 
      {
         ossimFilename oldFilename = m_writer.valid()?m_writer->getFilename():ossimFilename();
         ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(m_fileTypes->itemText(idx).toAscii().data()));
         m_writer = dynamic_cast<ossimImageFileWriter*> (obj.get());
         if(m_writer.valid())
         {
            m_fileButton->setEnabled(true);
            ossimString ext = m_writer->getExtension();
            if(!oldFilename.empty())
            {
               oldFilename.setExtension(ext);
            }
            m_writer->setFilename(oldFilename);
         }
         populatePropertyView();
      }
   }

   void ExportImageDialog::exportAbortClicked(bool)
   {
      if(m_exportingFlag&&m_writer.valid())
      {
         // now abort
         //
         m_writer->abort();
         m_exportingFlag = false;
         m_exportAbortButton->setText("Export");
      }
      else if(m_writer.valid())
      {
         if(m_writer->getFilename().empty())
         {
            QMessageBox::warning(this, tr("Export Image Error"),
                                 tr("No filename specified"),
                                 QMessageBox::Ok, 
                                 QMessageBox::Ok);         
         }
         else 
         {
            if(m_exportInBackground->isChecked())
            {
               QMainWindow* mainWindow = Util::findParentOfType<QMainWindow*>(this);
               CopyChainVisitor visitor;
               m_writer->connectMyInputTo(0, m_connectable.get());
               m_writer->accept(visitor);
               ossimRefPtr<ossimJob> job = new ImageWriterJob(visitor.kwl());
               job->setName("Output " + m_writer->getFilename());
               job->ready();
               
               DataManagerWidgetJobEvent* evt = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_ADD);
               evt->setJobList(job.get());
               QApplication::postEvent(mainWindow, evt);
            }
            else 
            {
               m_progressBar->setObject(m_writer.get());
               m_writer->connectMyInputTo(0, m_connectable.get());
               m_exportAbortButton->setText("Abort");
               m_writer->execute();
               m_progressBar->setObject(0);
               m_exportingFlag = false;
               m_exportAbortButton->setText("Export");
           }
         }
      }
   }
   void ExportImageDialog::closeClicked(bool)
   {
      if(m_writer.valid())
      {
         if(!m_writer->isExecuting())
         {
            close();
         }
         else
         {
            QMessageBox::warning(this, "Warning", "Please abort the current job before closing the export window.");
         }
      }
      else 
      {
         close();
      }

   }
   
   void ExportImageDialog::openFileSaveDialog()
   {
      if(m_writer.valid())
      {
         ossimFilename file(m_writer->getFilename());
         QString fileName = QFileDialog::getSaveFileName(this, "Export Image", (file.isDir()?file.c_str():file.path().c_str()));
      
         file = fileName.toAscii().data();
         m_writer->setFilename(file);
         populatePropertyView();
      }
   }
}
