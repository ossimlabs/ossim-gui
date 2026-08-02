#ifndef ossimGuiRegistrationSourceJobs_HEADER
#define ossimGuiRegistrationSourceJobs_HEADER

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include <ossim/base/ossimFilename.h>
#include <ossim/parallel/ossimJob.h>
#include <ossim/registration/ossimBundleAdjustmentRegistrationSource.h>
#include <ossim/registration/ossimFixedRegistrationSource.h>
#include <ossimGui/Event.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace ossimGui
{
   class DataManagerWidget;
   class RegistrationTiePointSnapshotMailbox;

   class RegistrationSourceJob : public ossimJob
   {
   public:
      RegistrationSourceJob(
         ossimFixedRegistrationSource* registrationSource,
         DataManagerWidget* dataManagerWidget,
         std::shared_ptr<std::atomic_bool> shutdownRequested,
         const ossimString& label,
         std::shared_ptr<RegistrationTiePointSnapshotMailbox>
            tiePointMailbox =
               std::shared_ptr<RegistrationTiePointSnapshotMailbox>());

      bool success() const;
      const ossimString& resultSummary() const;
      const ossimString& advisorySummary() const;
      const std::string& reportText() const;
      const ossimFilename& reportPath() const;
      const DataManagerWidgetEvent::HandlerListType&
         sourceHandlersToReload() const;
      bool widgetShutdownRequested() const;

      void start() override;

   protected:
      std::string displayProgressMessage(
         const ossimFixedRegistrationSource::ProgressInfo& progress) const;
      void updateProgressName(
         const ossimFixedRegistrationSource::ProgressInfo& progress);
      ossimFilename defaultGeometryOutput(
         const ossimFixedRegistrationSource::InputWrapper& input) const;
      bool applyResultOnGuiThread(
         const ossimFixedRegistrationSource::RegistrationResult& result);
      bool saveResultGeometryOnGuiThread(
         const ossimFixedRegistrationSource::RegistrationResult& result,
         const ossimFilename& outputFile);
      void run() override;

      ossimRefPtr<ossimFixedRegistrationSource> m_registrationSource;
      DataManagerWidget* m_dataManagerWidget;
      std::shared_ptr<std::atomic_bool> m_shutdownRequested;
      ossimString m_label;
      std::string m_launchInputStatus;
      std::string m_launchSettings;
      mutable std::string m_lastGeometryProgress;
      ossimString m_resultSummary;
      ossimString m_advisorySummary;
      std::string m_reportText;
      ossimFilename m_reportPath;
      DataManagerWidgetEvent::HandlerListType m_sourceHandlersToReload;
      std::shared_ptr<RegistrationTiePointSnapshotMailbox>
         m_tiePointMailbox;
      bool m_success;
   };

   class BundleRegistrationSourceJob : public ossimJob
   {
   public:
      BundleRegistrationSourceJob(
         ossimBundleAdjustmentRegistrationSource* registrationSource,
         DataManagerWidget* dataManagerWidget,
         std::shared_ptr<std::atomic_bool> shutdownRequested,
         const ossimString& label);

      bool success() const;
      const ossimString& resultSummary() const;
      const ossimString& advisorySummary() const;
      const std::string& reportText() const;
      const ossimFilename& reportPath() const;
      const DataManagerWidgetEvent::HandlerListType&
         sourceHandlersToReload() const;
      bool widgetShutdownRequested() const;

      void start() override;

   protected:
      void updateProgressName(
         const ossimBundleAdjustmentRegistrationSource::ProgressInfo&
            progress);
      ossimFilename defaultGeometryOutput(
         const ossimBundleAdjustmentRegistrationSource::InputWrapper& input)
         const;
      bool applySnapshotOnGuiThread(
         const ossimBundleAdjustmentRegistrationSource::AdjustmentSnapshot&
            snapshot);
      bool saveGeometriesOnGuiThread(
         const std::vector<ossimFilename>& outputGeometryFiles);
      void run() override;

      ossimRefPtr<ossimBundleAdjustmentRegistrationSource>
         m_registrationSource;
      DataManagerWidget* m_dataManagerWidget;
      std::shared_ptr<std::atomic_bool> m_shutdownRequested;
      ossimString m_label;
      ossimString m_resultSummary;
      ossimString m_advisorySummary;
      std::string m_reportText;
      ossimFilename m_reportPath;
      DataManagerWidgetEvent::HandlerListType m_sourceHandlersToReload;
      bool m_success;
   };
}

#endif

#endif
