#ifndef ossimGuiRegistrationSourceJobSupport_HEADER
#define ossimGuiRegistrationSourceJobSupport_HEADER

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimString.h>

#include <QString>

#include <string>

namespace ossimGui
{
   namespace registration_job_detail
   {
      std::string progressPhaseMessage(const std::string& message);
      ossimFilename qualityReportPath(const ossimString& label,
                                      const QString& prefix);
      bool writeQualityReport(const ossimFilename& path,
                              const std::string& text);
      void appendQualityReportStatus(ossimString& summary,
                                     const ossimFilename& path,
                                     bool wroteReport);
   }
}

#endif

#endif
