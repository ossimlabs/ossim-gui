#ifndef ossimGui_RegistrationTiePointWorkbench_HEADER
#define ossimGui_RegistrationTiePointWorkbench_HEADER

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

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

   class RegistrationTiePointSnapshotMailbox
   {
   public:
      void publish(
         const ossimFixedRegistrationSource::TiePointSnapshot& snapshot);
      bool read(std::uint64_t& revision,
                std::vector<
                   ossimFixedRegistrationSource::TiePointSnapshot>& snapshots)
         const;
      void finish(bool success, const std::string& message);
      bool completion(bool& success, std::string& message) const;

   private:
      mutable std::mutex m_mutex;
      std::uint64_t m_revision = 0;
      std::vector<ossimFixedRegistrationSource::TiePointSnapshot> m_snapshots;
      bool m_finished = false;
      bool m_success = false;
      std::string m_completionMessage;
   };

   QWidget* createRegistrationTiePointWorkbench(
      QWidget* parent,
      ossimFixedRegistrationSource* source,
      ImageScrollView* view,
      const std::shared_ptr<RegistrationTiePointSnapshotMailbox>& mailbox);
}

#endif

#endif
