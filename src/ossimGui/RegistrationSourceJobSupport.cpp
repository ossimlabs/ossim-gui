#include "RegistrationSourceJobSupport.h"

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include <QDateTime>
#include <QDir>

#include <fstream>

namespace
{
   bool containsText(const std::string& text, const std::string& pattern)
   {
      return text.find(pattern) != std::string::npos;
   }

   QString safeReportLabel(const ossimString& label)
   {
      QString result = QString::fromStdString(label.string()).trimmed();
      if(result.isEmpty())
      {
         result = "registration";
      }
      for(int idx = 0; idx < result.size(); ++idx)
      {
         const QChar ch = result.at(idx);
         if(!ch.isLetterOrNumber() && ch != '_' && ch != '-')
         {
            result[idx] = '_';
         }
      }
      return result.left(80);
   }
}

namespace ossimGui
{
   namespace registration_job_detail
   {
      std::string progressPhaseMessage(const std::string& message)
      {
         if(message.empty())
         {
            return message;
         }

         std::string phase;
         if(containsText(message, "adaptive trial") ||
            containsText(message, "candidate bank") ||
            containsText(message, "source candidate bank"))
         {
            phase = "candidate bank";
         }
         else if(containsText(message, "post-bank") ||
                 containsText(message, "guided probe") ||
                 containsText(message, "adaptive refinement"))
         {
            phase = "post-bank refinement";
         }
         else if(containsText(message, "final-quality") ||
                 containsText(message, "final quality") ||
                 containsText(message, "full-quality refinement"))
         {
            phase = "final quality";
         }
         else if(containsText(message, "coarse") ||
                 containsText(message, "Coarse"))
         {
            phase = "coarse";
         }
         else if(containsText(message, "generating ties") ||
                 containsText(message, "generating pair ties"))
         {
            phase = "tie generation";
         }
         else if(containsText(message, "optimizing"))
         {
            phase = "optimization";
         }
         else if(containsText(message, "filtering residuals"))
         {
            phase = "residual filtering";
         }

         if(phase.empty() || containsText(message, phase + ":"))
         {
            return message;
         }
         return phase + ": " + message;
      }

      ossimFilename qualityReportPath(const ossimString& label,
                                      const QString& prefix)
      {
         QDir reportDir(QDir::temp().filePath(
            "ossim-geocell-registration-reports"));
         if(!reportDir.exists())
         {
            reportDir.mkpath(".");
         }
         const QString timestamp =
            QDateTime::currentDateTimeUtc().toString("yyyyMMddTHHmmsszzzZ");
         const QString filename =
            prefix + "-" + safeReportLabel(label) + "-" + timestamp + ".txt";
         return ossimFilename(reportDir.filePath(filename).toStdString());
      }

      bool writeQualityReport(const ossimFilename& path,
                              const std::string& text)
      {
         if(path.empty())
         {
            return false;
         }
         std::ofstream out(path.c_str());
         if(!out)
         {
            return false;
         }
         out << text;
         return static_cast<bool>(out);
      }

      void appendQualityReportStatus(ossimString& summary,
                                     const ossimFilename& path,
                                     bool wroteReport)
      {
         summary += wroteReport ? ", quality report: " :
                                  ", failed to write quality report: ";
         summary += path;
      }
   }
}

#endif
