#ifndef ossimGui_RegistrationTiePointWorkbench_HEADER
#define ossimGui_RegistrationTiePointWorkbench_HEADER

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include <ossim/registration/ossimBundleAdjustmentRegistrationSource.h>
#include <ossim/registration/ossimFixedRegistrationSource.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class QWidget;

namespace ossimGui
{
   class ImageScrollView;

   class RegistrationTiePointSnapshot
   {
   public:
      ossim_uint32 firstInputIndex() const { return m_firstInputIndex; }
      void setFirstInputIndex(ossim_uint32 value) { m_firstInputIndex = value; }
      ossim_uint32 secondInputIndex() const { return m_secondInputIndex; }
      void setSecondInputIndex(ossim_uint32 value)
      {
         m_secondInputIndex = value;
      }
      int passIndex() const { return m_passIndex; }
      void setPassIndex(int value) { m_passIndex = value; }
      int passCount() const { return m_passCount; }
      void setPassCount(int value) { m_passCount = value; }
      bool bundleEdge() const { return m_bundleEdge; }
      void setBundleEdge(bool value) { m_bundleEdge = value; }
      const std::string& message() const { return m_message; }
      void setMessage(const std::string& value) { m_message = value; }
      const std::vector<ossim_autoreg::TiePointObservation>& tiePoints() const
      {
         return m_tiePoints;
      }
      void setTiePoints(
         const std::vector<ossim_autoreg::TiePointObservation>& value)
      {
         m_tiePoints = value;
      }
      const std::vector<ossim_autoreg::TiePointResidual>&
      initialTiePointResiduals() const
      {
         return m_initialTiePointResiduals;
      }
      void setInitialTiePointResiduals(
         const std::vector<ossim_autoreg::TiePointResidual>& value)
      {
         m_initialTiePointResiduals = value;
      }
      const std::vector<ossim_autoreg::TiePointResidual>&
      tiePointResiduals() const
      {
         return m_tiePointResiduals;
      }
      void setTiePointResiduals(
         const std::vector<ossim_autoreg::TiePointResidual>& value)
      {
         m_tiePointResiduals = value;
      }

   private:
      ossim_uint32 m_firstInputIndex = 0;
      ossim_uint32 m_secondInputIndex = 0;
      int m_passIndex = -1;
      int m_passCount = 0;
      bool m_bundleEdge = false;
      std::string m_message;
      std::vector<ossim_autoreg::TiePointObservation> m_tiePoints;
      std::vector<ossim_autoreg::TiePointResidual>
         m_initialTiePointResiduals;
      std::vector<ossim_autoreg::TiePointResidual> m_tiePointResiduals;
   };

   class RegistrationTiePointSnapshotMailbox
   {
   public:
      void publish(const RegistrationTiePointSnapshot& snapshot);
      void publish(
         const ossimFixedRegistrationSource::TiePointSnapshot& snapshot);
      bool read(std::uint64_t& revision,
                std::vector<RegistrationTiePointSnapshot>& snapshots) const;
      void finish(bool success, const std::string& message);
      bool completion(bool& success, std::string& message) const;

   private:
      mutable std::mutex m_mutex;
      std::uint64_t m_revision = 0;
      std::vector<RegistrationTiePointSnapshot> m_snapshots;
      bool m_finished = false;
      bool m_success = false;
      std::string m_completionMessage;
   };

   QWidget* createRegistrationTiePointWorkbench(
      QWidget* parent,
      ossimFixedRegistrationSource* source,
      ImageScrollView* view,
      const std::shared_ptr<RegistrationTiePointSnapshotMailbox>& mailbox);

   QWidget* createRegistrationTiePointWorkbench(
      QWidget* parent,
      ossimBundleAdjustmentRegistrationSource* source,
      ImageScrollView* view,
      const std::shared_ptr<RegistrationTiePointSnapshotMailbox>& mailbox);
}

#endif

#endif
