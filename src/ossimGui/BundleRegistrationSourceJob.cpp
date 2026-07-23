#include "RegistrationSourceJobs.h"

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include "RegistrationSourceJobSupport.h"

#include <ossim/imaging/ossimImageHandler.h>
#include <ossimGui/DataManagerWidget.h>
#include <ossim_autoreg/RegistrationTextReport.h>

#include <QMetaObject>
#include <QThread>

#include <functional>
#include <sstream>

namespace
{
   class BundleRegistrationCallbackScope
   {
   public:
      explicit BundleRegistrationCallbackScope(
         ossimBundleAdjustmentRegistrationSource* source)
      :m_source(source)
      {
      }

      ~BundleRegistrationCallbackScope()
      {
         reset();
      }

      void reset()
      {
         if(!m_source)
         {
            return;
         }
         m_source->setCancelCallback(std::function<bool()>());
         m_source->setProgressCallback(
            std::function<void(
               const ossimBundleAdjustmentRegistrationSource::
                  ProgressInfo&)>());
         m_source = 0;
      }

   private:
      ossimBundleAdjustmentRegistrationSource* m_source;
   };
}

namespace ossimGui
{
   BundleRegistrationSourceJob::BundleRegistrationSourceJob(
      ossimBundleAdjustmentRegistrationSource* registrationSource,
      DataManagerWidget* dataManagerWidget,
      std::shared_ptr<std::atomic_bool> shutdownRequested,
      const ossimString& label)
   :m_registrationSource(registrationSource),
    m_dataManagerWidget(dataManagerWidget),
    m_shutdownRequested(shutdownRequested),
    m_label(label),
    m_success(false)
   {
      setId("ossimGui::BundleRegistrationSourceJob");
      if(m_registrationSource.valid() &&
         m_registrationSource->allInputsFloating())
      {
         setName("Bundle adjust all-floating: " + m_label);
      }
      else
      {
         setName("Bundle adjust anchored: " + m_label);
      }
   }

   bool BundleRegistrationSourceJob::success() const
   {
      return m_success;
   }

   const ossimString& BundleRegistrationSourceJob::resultSummary() const
   {
      return m_resultSummary;
   }

   const ossimString& BundleRegistrationSourceJob::advisorySummary() const
   {
      return m_advisorySummary;
   }

   const std::string& BundleRegistrationSourceJob::reportText() const
   {
      return m_reportText;
   }

   const ossimFilename& BundleRegistrationSourceJob::reportPath() const
   {
      return m_reportPath;
   }

   const DataManagerWidgetEvent::HandlerListType&
   BundleRegistrationSourceJob::sourceHandlersToReload() const
   {
      return m_sourceHandlersToReload;
   }

   bool BundleRegistrationSourceJob::widgetShutdownRequested() const
   {
      return m_shutdownRequested && m_shutdownRequested->load();
   }

   void BundleRegistrationSourceJob::start()
   {
      if(isCanceled())
      {
         m_success = false;
         m_resultSummary = "Bundle adjustment canceled before it started.";
         setDescription(m_resultSummary);
         setName("Bundle adjustment canceled: " + m_label);
         finished();
         return;
      }

      setState(ossimJob_RUNNING);
      run();
      finished();
   }

   void BundleRegistrationSourceJob::updateProgressName(
      const ossimBundleAdjustmentRegistrationSource::ProgressInfo& progress)
   {
      ossimString name = "Bundle adjust";
      const std::string message =
         registration_job_detail::progressPhaseMessage(progress.message());
      if(!message.empty())
      {
         name += " ";
         name += message.c_str();
      }
      name += ": ";
      name += m_label;
      setName(name);
      if(!message.empty())
      {
         setDescription(message.c_str());
      }
      setPercentComplete(progress.percentComplete());
   }

   ossimFilename BundleRegistrationSourceJob::defaultGeometryOutput(
      const ossimBundleAdjustmentRegistrationSource::InputWrapper& input)
      const
   {
      ossimFilename result;
      ossimImageHandler* handler = input.sourceHandler();
      if(handler)
      {
         result = handler->getFilename().expand();
      }
      if(result.empty())
      {
         result = ossimString("bundle_") +
                  ossimString::toString(input.inputIndex()) + ".geom";
      }
      else
      {
         result.setExtension("geom");
      }
      return result;
   }

   bool BundleRegistrationSourceJob::saveGeometriesOnGuiThread(
      const std::vector<ossimFilename>& outputGeometryFiles)
   {
      if(widgetShutdownRequested())
      {
         return false;
      }
      bool saved = false;
      auto saveGeometry = [this, &outputGeometryFiles, &saved]() {
         saved = !widgetShutdownRequested() &&
                 m_registrationSource.valid() &&
                 m_registrationSource->saveGeometries(outputGeometryFiles);
      };

      if(m_dataManagerWidget &&
         QThread::currentThread() != m_dataManagerWidget->thread())
      {
         const bool invoked =
            QMetaObject::invokeMethod(m_dataManagerWidget,
                                      saveGeometry,
                                      Qt::BlockingQueuedConnection);
         if(!invoked)
         {
            saved = false;
         }
      }
      else
      {
         saveGeometry();
      }

      return saved;
   }

   void BundleRegistrationSourceJob::run()
   {
      setPercentComplete(0.0);
      m_sourceHandlersToReload.clear();
      m_advisorySummary.clear();
      if(m_registrationSource.valid())
      {
         m_registrationSource->setCancelCallback([this]() {
            return this->isCanceled() || widgetShutdownRequested();
         });
         m_registrationSource->setProgressCallback(
            [this](const ossimBundleAdjustmentRegistrationSource::
                      ProgressInfo& progress) {
               if(!widgetShutdownRequested())
               {
                  updateProgressName(progress);
               }
            });
         BundleRegistrationCallbackScope callbackScope(
            m_registrationSource.get());
         m_success = m_registrationSource->executeRegistration();
         callbackScope.reset();
         if(widgetShutdownRequested())
         {
            m_success = false;
            m_resultSummary = "Bundle adjustment canceled during shutdown.";
            setDescription(m_resultSummary);
            setName("Bundle adjustment canceled: " + m_label);
            setPercentComplete(100.0);
            return;
         }

         const ossimBundleAdjustmentRegistrationSource::RegistrationResult&
            result = m_registrationSource->registrationResult();
         ossim_uint32 idx = 0;
         m_resultSummary = m_registrationSource->registrationSummary().c_str();
         m_advisorySummary =
            m_registrationSource->registrationAdvisorySummary().c_str();

         std::vector<ossimFilename> writtenGeometryFiles;
         std::vector<ossimFilename> outputGeometryFiles;
         if(m_success && !isCanceled() &&
            m_registrationSource->optimizeEnabled())
         {
            const std::vector<ossim_uint32>& inputIndexes =
               m_registrationSource->bundleInputIndexes();
            for(idx = 0; idx < inputIndexes.size(); ++idx)
            {
               const ossimBundleAdjustmentRegistrationSource::InputWrapper*
                  input = m_registrationSource->inputWrapper(inputIndexes[idx]);
               if(input)
               {
                  outputGeometryFiles.push_back(defaultGeometryOutput(*input));
               }
            }

            if(outputGeometryFiles.size() == inputIndexes.size() &&
               saveGeometriesOnGuiThread(outputGeometryFiles))
            {
               writtenGeometryFiles = outputGeometryFiles;
               for(idx = 0; idx < inputIndexes.size(); ++idx)
               {
                  const ossimBundleAdjustmentRegistrationSource::InputWrapper*
                     input = m_registrationSource->inputWrapper(
                        inputIndexes[idx]);
                  if(input)
                  {
                     ossimImageHandler* sourceHandler = input->sourceHandler();
                     if(sourceHandler)
                     {
                        m_sourceHandlersToReload.push_back(sourceHandler);
                     }
                  }
               }
            }
            else
            {
               m_success = false;
               m_resultSummary +=
                  ", failed to write bundle geometry file(s)";
            }
         }

         if(isCanceled())
         {
            m_resultSummary = "Bundle adjustment canceled.";
            m_success = false;
         }
         if(!writtenGeometryFiles.empty())
         {
            m_resultSummary += ", wrote ";
            m_resultSummary += ossimString::toString(
               static_cast<ossim_uint32>(writtenGeometryFiles.size()));
            m_resultSummary += " geometry file(s)";
            for(idx = 0; idx < writtenGeometryFiles.size(); ++idx)
            {
               m_resultSummary += (idx == 0) ? ": " : ", ";
               m_resultSummary += writtenGeometryFiles[idx];
            }
         }
         m_reportPath = registration_job_detail::qualityReportPath(
            m_label,
            "bundle-registration");
         ossim_autoreg::BundleRegistrationTextReportContext reportContext;
         reportContext.request = m_registrationSource->registrationRequest();
         reportContext.result = result.sharedResult();
         for(const ossim_autoreg::BundleAutoRegistrationPairResult& pair :
             reportContext.result.pairResults())
         {
            reportContext.pairResidualSummaries.push_back(
               pair.residualSummary());
         }
         reportContext.outputGeometries = writtenGeometryFiles;
         reportContext.geometriesWritten = !writtenGeometryFiles.empty();
         reportContext.boundPolicyMessage = result.boundPressureAdvisory();
         if(!m_success)
         {
            reportContext.rejectionReason = result.message().empty()
               ? std::string("registration failed")
               : result.message();
         }
         std::ostringstream report;
         report << "registration_type: bundle\n"
                << "label: " << m_label << "\n"
                << "summary: " << m_resultSummary << "\n\n";
         report << ossim_autoreg::bundleRegistrationTextReport(reportContext);
         m_reportText = report.str();
         const bool wroteReport = registration_job_detail::writeQualityReport(
            m_reportPath,
            m_reportText);
         registration_job_detail::appendQualityReportStatus(
            m_resultSummary,
            m_reportPath,
            wroteReport);

         setDescription(m_resultSummary);
         if(isCanceled())
         {
            setName("Bundle adjustment canceled: " + m_label);
         }
         else
         {
            setName((m_success ? "Bundle adjusted: " :
                                 "Bundle adjustment failed: ") +
                    m_label);
         }
      }
      else
      {
         m_success = false;
         m_resultSummary =
            "Bundle adjustment source is no longer available.";
         setDescription(m_resultSummary);
         setName("Bundle adjustment failed: " + m_label);
      }
      setPercentComplete(100.0);
   }
}

#endif
