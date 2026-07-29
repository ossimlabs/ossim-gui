#include "RegistrationSourceJobs.h"

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include "RegistrationSourceJobSupport.h"
#include "RegistrationTiePointWorkbench.h"

#include <ossim/base/ossimEvent.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossimGui/DataManagerWidget.h>
#include <ossim_autoreg/AutoRegistration.h>
#include <ossim_autoreg/RegistrationTextReport.h>

#include <QMetaObject>
#include <QThread>

#include <functional>
#include <sstream>

namespace
{
   class FixedRegistrationCallbackScope
   {
   public:
      explicit FixedRegistrationCallbackScope(
         ossimFixedRegistrationSource* source)
      :m_source(source)
      {
      }

      ~FixedRegistrationCallbackScope()
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
               const ossimFixedRegistrationSource::ProgressInfo&)>());
         m_source->setTiePointSnapshotCallback(
            std::function<void(
               const ossimFixedRegistrationSource::TiePointSnapshot&)>());
         m_source->setApplyResultCallback(
            std::function<bool(
               const ossimFixedRegistrationSource::RegistrationResult&)>());
         m_source = 0;
      }

   private:
      ossimFixedRegistrationSource* m_source;
   };
}

namespace ossimGui
{
   RegistrationSourceJob::RegistrationSourceJob(
      ossimFixedRegistrationSource* registrationSource,
      DataManagerWidget* dataManagerWidget,
      std::shared_ptr<std::atomic_bool> shutdownRequested,
      const ossimString& label,
      std::shared_ptr<RegistrationTiePointSnapshotMailbox>
         tiePointMailbox)
   :m_registrationSource(registrationSource),
    m_dataManagerWidget(dataManagerWidget),
    m_shutdownRequested(shutdownRequested),
    m_label(label),
    m_launchInputStatus(registrationSource
                           ? registrationSource->inputStatusSummary()
                           : std::string()),
    m_launchSettings(registrationSource
                        ? registrationSource->autoRegistrationSettingsSummary()
                        : std::string()),
    m_tiePointMailbox(std::move(tiePointMailbox)),
    m_success(false)
   {
      setId("ossimGui::RegistrationSourceJob");
      setName("Register: " + m_label);
   }

   bool RegistrationSourceJob::success() const
   {
      return m_success;
   }

   const ossimString& RegistrationSourceJob::resultSummary() const
   {
      return m_resultSummary;
   }

   const ossimString& RegistrationSourceJob::advisorySummary() const
   {
      return m_advisorySummary;
   }

   const std::string& RegistrationSourceJob::reportText() const
   {
      return m_reportText;
   }

   const ossimFilename& RegistrationSourceJob::reportPath() const
   {
      return m_reportPath;
   }

   const DataManagerWidgetEvent::HandlerListType&
   RegistrationSourceJob::sourceHandlersToReload() const
   {
      return m_sourceHandlersToReload;
   }

   bool RegistrationSourceJob::widgetShutdownRequested() const
   {
      return m_shutdownRequested && m_shutdownRequested->load();
   }

   void RegistrationSourceJob::start()
   {
      if(isCanceled())
      {
         m_success = false;
         m_resultSummary = "Registration canceled before it started.";
         setDescription(m_resultSummary);
         setName("Registration canceled: " + m_label);
         finished();
         return;
      }

      setState(ossimJob_RUNNING);
      run();
      finished();
   }

   std::string RegistrationSourceJob::displayProgressMessage(
      const ossimFixedRegistrationSource::ProgressInfo& progress) const
   {
      const std::string message = progress.message();
      std::string geometryMessage;
      if(message.find("preview pass") != std::string::npos)
      {
         geometryMessage = "preview geometry: " + message;
      }
      else if(message.find("accepted pass") != std::string::npos)
      {
         geometryMessage = "accepted geometry: " + message;
      }
      else if(message.find("restored best") != std::string::npos)
      {
         geometryMessage = "restored geometry: " + message;
      }
      else if(message.find("coarse seed accepted") != std::string::npos ||
              message.find("fallback coarse seed accepted") !=
                 std::string::npos)
      {
         geometryMessage = "coarse geometry: " + message;
      }

      if(!geometryMessage.empty())
      {
         m_lastGeometryProgress = geometryMessage;
         return geometryMessage;
      }

      if(!m_lastGeometryProgress.empty() &&
         message.find("generating ties") != std::string::npos)
      {
         return m_lastGeometryProgress + " | " + message;
      }
      return registration_job_detail::progressPhaseMessage(message);
   }

   void RegistrationSourceJob::updateProgressName(
      const ossimFixedRegistrationSource::ProgressInfo& progress)
   {
      ossimString name = "Register";
      const std::string message = displayProgressMessage(progress);
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

   ossimFilename RegistrationSourceJob::defaultGeometryOutput(
      const ossimFixedRegistrationSource::InputWrapper& input) const
   {
      ossimFilename result;
      ossimImageHandler* handler = input.sourceHandler();
      if(handler)
      {
         result = handler->getFilename().expand();
      }
      if(result.empty())
      {
         result = ossimString("floating_") +
                  ossimString::toString(input.inputIndex()) + ".geom";
      }
      else
      {
         result.setExtension("geom");
      }
      return result;
   }

   bool RegistrationSourceJob::applyResultOnGuiThread(
      const ossimFixedRegistrationSource::RegistrationResult& result)
   {
      if(!m_registrationSource.valid() || !m_dataManagerWidget ||
         widgetShutdownRequested())
      {
         return false;
      }

      bool applied = false;
      const auto apply = [this, &result, &applied]() {
         if(!m_registrationSource.valid() || widgetShutdownRequested())
         {
            applied = false;
            return;
         }

         applied = m_registrationSource->applyRegistrationResultToInput(result);
         if(applied)
         {
            const ossimFixedRegistrationSource::InputWrapper* input =
               m_registrationSource->inputWrapper(result.inputIndex());
            ossimImageSource* source = input ? input->source() : 0;
            if(source)
            {
               ossimRefPtr<ossimRefreshEvent> refreshEvent =
                  new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
               ossimEventVisitor visitor(refreshEvent.get(),
                                         ossimVisitor::VISIT_ALL);
               source->accept(visitor);
            }
         }
      };

      if(QThread::currentThread() == m_dataManagerWidget->thread())
      {
         apply();
      }
      else
      {
         const bool invoked =
            QMetaObject::invokeMethod(m_dataManagerWidget,
                                      apply,
                                      Qt::BlockingQueuedConnection);
         if(!invoked)
         {
            applied = false;
         }
      }
      return applied;
   }

   bool RegistrationSourceJob::saveResultGeometryOnGuiThread(
      const ossimFixedRegistrationSource::RegistrationResult& result,
      const ossimFilename& outputFile)
   {
      if(!m_registrationSource.valid() || !m_dataManagerWidget ||
         widgetShutdownRequested())
      {
         return false;
      }

      bool saved = false;
      const auto saveGeometry = [this, &result, &outputFile, &saved]() {
         if(!m_registrationSource.valid() || widgetShutdownRequested())
         {
            saved = false;
            return;
         }

         const ossim_autoreg::RegistrationSession* session =
            m_registrationSource->registrationSession(
               result.floatingInputIndex());
         saved = session && session->saveMovingGeometry(outputFile);
      };

      if(QThread::currentThread() == m_dataManagerWidget->thread())
      {
         saveGeometry();
      }
      else
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
      return saved;
   }

   void RegistrationSourceJob::run()
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
            [this](
               const ossimFixedRegistrationSource::ProgressInfo& progress) {
               if(!widgetShutdownRequested())
               {
                  updateProgressName(progress);
               }
            });
         m_registrationSource->setTiePointSnapshotCallback(
            [this](
               const ossimFixedRegistrationSource::TiePointSnapshot&
                  snapshot) {
               if(m_tiePointMailbox && !widgetShutdownRequested())
                  m_tiePointMailbox->publish(snapshot);
            });
         m_registrationSource->setApplyResultsToInputs(false);
         m_registrationSource->setApplyResultCallback(
            [this](
               const ossimFixedRegistrationSource::RegistrationResult&
                  result) {
               return applyResultOnGuiThread(result);
            });
         FixedRegistrationCallbackScope callbackScope(
            m_registrationSource.get());
         m_success = m_registrationSource->executeRegistration();
         callbackScope.reset();
         if(m_tiePointMailbox)
         {
            m_tiePointMailbox->finish(
               m_success,
               m_registrationSource->registrationSummary());
         }
         if(widgetShutdownRequested())
         {
            m_success = false;
            m_resultSummary = "Registration canceled during shutdown.";
            setDescription(m_resultSummary);
            setName("Registration canceled: " + m_label);
            setPercentComplete(100.0);
            return;
         }

         const std::vector<
            ossimFixedRegistrationSource::RegistrationResult>& results =
               m_registrationSource->registrationResults();
         m_resultSummary = m_registrationSource->registrationSummary().c_str();
         m_advisorySummary =
            m_registrationSource->registrationAdvisorySummary().c_str();
         ossim_uint32 propagatedGeometries = 0;
         std::vector<ossimFilename> writtenGeometryFiles;
         std::vector<ossimFilename> failedGeometryFiles;
         ossim_uint32 idx = 0;
         for(idx = 0; idx < results.size(); ++idx)
         {
            if(results[idx].success() && !isCanceled())
            {
               const ossimFixedRegistrationSource::InputWrapper* input =
                  m_registrationSource->inputWrapper(results[idx].inputIndex());
               if(input && m_registrationSource->optimizeEnabled())
               {
                  const ossimFilename outputFile =
                     defaultGeometryOutput(*input);
                  if(results[idx].appliedToInput() ||
                     applyResultOnGuiThread(results[idx]))
                  {
                     ++propagatedGeometries;
                  }
                  if(saveResultGeometryOnGuiThread(results[idx], outputFile))
                  {
                     writtenGeometryFiles.push_back(outputFile);
                     ossimImageHandler* sourceHandler = input->sourceHandler();
                     if(sourceHandler)
                     {
                        m_sourceHandlersToReload.push_back(sourceHandler);
                     }
                  }
                  else
                  {
                     failedGeometryFiles.push_back(outputFile);
                  }
               }
            }
         }

         if(isCanceled())
         {
            m_resultSummary = "Registration canceled.";
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
         if(propagatedGeometries > 0)
         {
            m_resultSummary += ", updated ";
            m_resultSummary += ossimString::toString(propagatedGeometries);
            m_resultSummary += " live geometry adjustment(s)";
         }
         if(!failedGeometryFiles.empty())
         {
            m_resultSummary += ", failed to write ";
            m_resultSummary += ossimString::toString(
               static_cast<ossim_uint32>(failedGeometryFiles.size()));
            m_resultSummary += " geometry file(s)";
            for(idx = 0; idx < failedGeometryFiles.size(); ++idx)
            {
               m_resultSummary += (idx == 0) ? ": " : ", ";
               m_resultSummary += failedGeometryFiles[idx];
            }
            m_success = false;
         }
         m_reportPath = registration_job_detail::qualityReportPath(
            m_label,
            "fixed-registration");
         std::vector<ossim_autoreg::FixedAutoRegistrationResult>
            sharedResults;
         sharedResults.reserve(results.size());
         for(const ossimFixedRegistrationSource::RegistrationResult& result :
             results)
         {
            sharedResults.push_back(result.sharedResult());
         }
         std::ostringstream report;
         report << "registration_type: fixed\n"
                << "label: " << m_label << "\n"
                << "summary: " << m_resultSummary << "\n"
                << "launch_input_status: " << m_launchInputStatus << "\n"
                << "launch_settings: " << m_launchSettings << "\n"
                << "written_geometry_count: "
                << writtenGeometryFiles.size() << "\n"
                << "failed_geometry_count: "
                << failedGeometryFiles.size() << "\n";
         for(std::size_t idx = 0; idx < writtenGeometryFiles.size(); ++idx)
         {
            report << "written_geometry[" << idx << "]: "
                   << writtenGeometryFiles[idx] << "\n";
         }
         for(std::size_t idx = 0; idx < failedGeometryFiles.size(); ++idx)
         {
            report << "failed_geometry[" << idx << "]: "
                   << failedGeometryFiles[idx] << "\n";
         }
         report << "\n";
         report << ossim_autoreg::fixedRegistrationTextReport(
            m_registrationSource->autoRegistrationOptions(),
            sharedResults);
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
            setName("Registration canceled: " + m_label);
         }
         else
         {
            setName((m_success ? "Registered: " : "Registration failed: ") +
                    m_label);
         }
      }
      else
      {
         m_success = false;
         m_resultSummary = "Registration source is no longer available.";
         setDescription(m_resultSummary);
         setName("Registration failed: " + m_label);
      }
      setPercentComplete(100.0);
   }
}

#endif
