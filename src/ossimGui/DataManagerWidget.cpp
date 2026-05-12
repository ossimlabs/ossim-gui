#include <ossimGui/DataManagerWidget.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageCombiner.h>
#include <ossim/base/ossimEvent.h>
#include <ossim/base/ossimBooleanProperty.h>
#include <ossim/base/ossimNumericProperty.h>
#include <ossim/base/ossimContainerProperty.h>
#include <ossim/base/ossimRefreshEvent.h>
#include <ossim/base/ossimStringProperty.h>
#include <ossim/base/ossimUrl.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageHandlerRegistry.h>
#include <ossim/imaging/ossimImageWriterFactoryRegistry.h>
#include <ossim/imaging/ossimMemoryImageSource.h>
#include <ossim/projection/ossimEquDistCylProjection.h>
#include <ossim/projection/ossimUtmProjection.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageSourceFactoryRegistry.h>
#include <ossim/imaging/ossimOverviewBuilderFactoryRegistry.h>
#include <QMainWindow>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QItemDelegate>
#include <QHeaderView>
#include <QInputDialog>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QToolBar>
#include <QItemDelegate>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QMessageBox>
#include <QUrl>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QDir>
#include <ossimGui/Common.h>
#include <ossimGui/Event.h>
#include <ossimGui/OpenImageDialog.h>
#include <ossimGui/OpenImageUrlJob.h>
#include <ossimGui/ConnectableDisplayObject.h>
#include <ossimGui/MdiSubWindowBase.h>
#include <ossimGui/ImageMdiSubWindow.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>
#include <ossimGui/IvtGeomTransform.h>
#include <ossimGui/ImageScrollView.h>
#include <ossimGui/Util.h>
#include <ossimGui/CopyChainVisitor.h>
#include <ossimGui/ImageWriterJob.h>
#include <ossimGui/ExportImageDialog.h>
#include <ossimGui/MultiImageDialog.h>
#include <ossimGui/AutoMeasurementDialog.h>
#include <ossimGui/RegistrationOverlay.h>
#include <ossimGui/RegPoint.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <sstream>

#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
#include <ossim/registration/ossimBundleAdjustmentRegistrationSource.h>
#include <ossim/registration/ossimFixedRegistrationSource.h>
#include <ossim/registration/ossimRegistrationSourceFactory.h>
#include <ossim_autoreg/AutoRegistration.h>
#include <ossim_autoreg/TiePointGenerator.h>
#endif

#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
namespace
{
   void ensureRegistrationSourceFactoryRegistered()
   {
      ossimObjectFactoryRegistry::instance()->registerFactory(
         ossimRegistrationSourceFactory::instance());
   }

   QString tiePointGeneratorSummary()
   {
      std::vector<std::string> typeNames =
         ossim_autoreg::TiePointGeneratorFactory::instance()->typeNames();
      QStringList names;
      for(const std::string& typeName : typeNames)
      {
         names << QString::fromStdString(typeName);
      }
      return names.join(", ");
   }

   bool registrationTieGeneratorAvailable(const std::string& method)
   {
      return static_cast<bool>(
         ossim_autoreg::TiePointGeneratorFactory::instance()->create(method));
   }

   std::string preferredRegistrationMatchMethod()
   {
      const char* orderedMethods[] = {
         "mixed-phase-orb",
         "opencv-sift",
         "opencv-orb",
         "opencv-phase-correlation",
         "phase-correlation",
         "hybrid-phase-ncc",
         "spatial-ncc"
      };
      for(const char* method : orderedMethods)
      {
         if(registrationTieGeneratorAvailable(method))
            return method;
      }
      return "hybrid-phase-ncc";
   }

   std::string preferredFixedAutoMatchMethod()
   {
      return preferredRegistrationMatchMethod();
   }

   std::string preferredBundleMatchMethod()
   {
      const std::string method =
         ossim_autoreg::defaultRegistrationMatchMethod();
      if(registrationTieGeneratorAvailable(method))
         return method;
      return preferredRegistrationMatchMethod();
   }

   void applyBundleDefaultsToSource(
      ossimBundleAdjustmentRegistrationSource* bundle,
      bool anchorEnabled)
   {
      if(!bundle)
         return;

      bundle->applyBundleRegistrationDefaults(anchorEnabled);
   }

   std::string bundleRegistrationDenseAlternateSummary(
      const ossimBundleAdjustmentRegistrationSource::RegistrationResult&
         result)
   {
      std::size_t evaluatedCount = 0;
      std::size_t usedCount = 0;
      std::string lastReason;
      for(std::size_t idx = 0; idx < result.pairResults().size(); ++idx)
      {
         const ossimBundleAdjustmentRegistrationSource::PairResult& pair =
            result.pairResults()[idx];
         if(pair.denseAlternateEvaluated())
         {
            ++evaluatedCount;
            if(pair.denseAlternateUsed())
            {
               ++usedCount;
            }
            lastReason = pair.denseAlternateAcceptance().reason();
         }
      }

      if(!evaluatedCount)
      {
         return std::string();
      }

      std::ostringstream out;
      out << "dense alternates " << usedCount << "/" << evaluatedCount
          << " used";
      if(!lastReason.empty())
      {
         out << ", last reason " << lastReason;
      }
      return out.str();
   }

   ossimString bundleRegistrationDiagnosticsSummary(
      const ossimBundleAdjustmentRegistrationSource::RegistrationResult&
         result)
   {
      std::ostringstream out;
      const ossim_autoreg::BundleAdjustmentResult& optimization =
         result.optimization();
      const ossim_autoreg::BundleConnectivityDiagnostics& connectivity =
         result.connectivity();

      out << "connectivity "
          << connectivity.getEdges().size() << " edge(s)/"
          << connectivity.getComponents().size() << " component(s)";
      if(!connectivity.getConnected())
      {
         out << ", disconnected";
      }

      if(optimization.ran())
      {
         const std::string solverBackend =
            optimization.solverBackendName().empty()
               ? std::string("unknown")
               : optimization.solverBackendName();
         out << ", solver " << solverBackend
             << ", active params " << optimization.activeParameterCount()
             << ", image blocks " << optimization.activeImageBlockCount()
             << ", normal blocks "
             << optimization.normalEquationBlockPairCount()
             << ", residuals " << optimization.validResidualCount();
      }

      const std::string denseAlternateSummary =
         bundleRegistrationDenseAlternateSummary(result);
      if(!denseAlternateSummary.empty())
      {
         out << ", " << denseAlternateSummary;
      }

      return out.str().c_str();
   }

   QString registrationReportSafeLabel(const ossimString& label)
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

   ossimFilename registrationQualityReportPath(const ossimString& label,
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
         prefix + "-" + registrationReportSafeLabel(label) + "-" +
         timestamp + ".txt";
      return ossimFilename(reportDir.filePath(filename).toStdString());
   }

   bool writeRegistrationQualityReport(const ossimFilename& path,
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

   void appendRegistrationQualityReportStatus(ossimString& summary,
                                              const ossimFilename& path,
                                              bool wroteReport)
   {
      summary += wroteReport ? ", quality report: " :
                               ", failed to write quality report: ";
      summary += path;
   }

   void appendTiePointSpreadQualityReport(
      std::ostringstream& out,
      const std::string& prefix,
      const ossim_autoreg::TiePointSpreadQuality& quality)
   {
      out << prefix << ".valid: "
          << (quality.valid() ? "true" : "false") << "\n";
      if(!quality.valid())
      {
         return;
      }
      out << prefix << ".weak: "
          << (quality.weak() ? "true" : "false") << "\n";
      out << prefix << ".area_ratio: " << quality.areaRatio() << "\n";
      out << prefix << ".fixed_aspect_ratio: "
          << quality.fixedAspectRatio() << "\n";
      out << prefix << ".moving_aspect_ratio: "
          << quality.movingAspectRatio() << "\n";
      out << prefix << ".reason: " << quality.reason() << "\n";
   }

   void appendTiePointTranslationConsistencyReport(
      std::ostringstream& out,
      const std::string& prefix,
      const ossim_autoreg::TiePointTranslationConsistency& consistency)
   {
      out << prefix << ".valid: "
          << (consistency.valid() ? "true" : "false") << "\n";
      if(!consistency.valid())
      {
         return;
      }
      out << prefix << ".count: " << consistency.count() << "\n";
      out << prefix << ".median_residual_pixels: "
          << consistency.medianResidualPixels() << "\n";
      out << prefix << ".rms_residual_pixels: "
          << consistency.rmsResidualPixels() << "\n";
      out << prefix << ".max_residual_pixels: "
          << consistency.maxResidualPixels() << "\n";
   }

   void appendTiePointScoreQualityReport(
      std::ostringstream& out,
      const std::string& prefix,
      const ossim_autoreg::TiePointScoreQuality& quality)
   {
      out << prefix << ".valid: "
          << (quality.valid() ? "true" : "false") << "\n";
      if(!quality.valid())
      {
         return;
      }
      out << prefix << ".count: " << quality.count() << "\n";
      out << prefix << ".min_score: " << quality.minScore() << "\n";
      out << prefix << ".mean_score: " << quality.meanScore() << "\n";
      out << prefix << ".median_score: " << quality.medianScore() << "\n";
      out << prefix << ".max_score: " << quality.maxScore() << "\n";
   }

   void appendDenseAlternateReport(
      std::ostringstream& out,
      const std::string& prefix,
      const ossimBundleAdjustmentRegistrationSource::PairResult& pair)
   {
      out << prefix << ".dense_alternate_evaluated: "
          << (pair.denseAlternateEvaluated() ? "true" : "false") << "\n";
      if(!pair.denseAlternateEvaluated())
      {
         return;
      }

      const ossim_autoreg::TiePointAlternateAcceptance& acceptance =
         pair.denseAlternateAcceptance();
      out << prefix << ".dense_alternate_used: "
          << (pair.denseAlternateUsed() ? "true" : "false") << "\n";
      out << prefix << ".dense_alternate_accepted: "
          << (acceptance.accepted() ? "true" : "false") << "\n";
      out << prefix << ".dense_alternate_reason: "
          << acceptance.reason() << "\n";
      appendTiePointSpreadQualityReport(
         out,
         prefix + ".dense_base_spread",
         acceptance.baseSpread());
      appendTiePointSpreadQualityReport(
         out,
         prefix + ".dense_candidate_spread",
         acceptance.candidateSpread());
      appendTiePointTranslationConsistencyReport(
         out,
         prefix + ".dense_base_translation",
         acceptance.baseConsistency());
      appendTiePointTranslationConsistencyReport(
         out,
         prefix + ".dense_candidate_translation",
         acceptance.candidateConsistency());
      appendTiePointScoreQualityReport(
         out,
         prefix + ".dense_base_score",
         acceptance.baseScoreQuality());
      appendTiePointScoreQualityReport(
         out,
         prefix + ".dense_candidate_score",
         acceptance.candidateScoreQuality());
   }

   std::string fixedRegistrationQualityReportText(
      const ossimString& label,
      const ossimString& summary,
      bool success,
      const std::vector<ossimFixedRegistrationSource::RegistrationResult>&
         results,
      const std::vector<ossimFilename>& writtenGeometryFiles,
      const std::vector<ossimFilename>& failedGeometryFiles)
   {
      std::ostringstream out;
      out << "registration_type: fixed\n";
      out << "label: " << label << "\n";
      out << "success: " << (success ? "true" : "false") << "\n";
      out << "summary: " << summary << "\n";
      out << "result_count: " << results.size() << "\n";
      out << "written_geometry_count: " << writtenGeometryFiles.size()
          << "\n";
      for(std::size_t idx = 0; idx < writtenGeometryFiles.size(); ++idx)
      {
         out << "written_geometry[" << idx << "]: "
             << writtenGeometryFiles[idx] << "\n";
      }
      out << "failed_geometry_count: " << failedGeometryFiles.size() << "\n";
      for(std::size_t idx = 0; idx < failedGeometryFiles.size(); ++idx)
      {
         out << "failed_geometry[" << idx << "]: "
             << failedGeometryFiles[idx] << "\n";
      }
      for(std::size_t idx = 0; idx < results.size(); ++idx)
      {
         const ossimFixedRegistrationSource::RegistrationResult& result =
            results[idx];
         const ossim_autoreg::OptimizationResult& optimization =
            result.optimization();
         out << "result[" << idx << "].input_index: "
             << result.inputIndex() << "\n";
         out << "result[" << idx << "].floating_input_index: "
             << result.floatingInputIndex() << "\n";
         out << "result[" << idx << "].success: "
             << (result.success() ? "true" : "false") << "\n";
         out << "result[" << idx << "].session_open: "
             << (result.sessionOpen() ? "true" : "false") << "\n";
         out << "result[" << idx << "].ran: "
             << (result.ran() ? "true" : "false") << "\n";
         out << "result[" << idx << "].tie_points: "
             << result.tiePoints().size() << "\n";
         out << "result[" << idx << "].message: "
             << result.message() << "\n";
         if(std::isfinite(result.effectiveTargetRmsePixels()))
         {
            out << "result[" << idx
                << "].effective_target_rmse_pixels: "
                << result.effectiveTargetRmsePixels() << "\n";
         }
         if(std::isfinite(result.effectiveSearchSpan()))
         {
            out << "result[" << idx
                << "].effective_search_span: "
                << result.effectiveSearchSpan() << "\n";
         }
         out << "result[" << idx << "].fixed_scene_policy: "
             << result.fixedScenePolicy() << "\n";
         out << "result[" << idx << "].search_span_policy: "
             << result.searchSpanPolicy() << "\n";
         out << "result[" << idx << "].optimization.ran: "
             << (optimization.ran() ? "true" : "false") << "\n";
         if(optimization.ran())
         {
            out << "result[" << idx
                << "].optimization.converged: "
                << (optimization.converged() ? "true" : "false") << "\n";
            out << "result[" << idx
                << "].optimization.iterations: "
                << optimization.iterations() << "\n";
            out << "result[" << idx
                << "].optimization.final_rmse_pixels: "
                << optimization.finalRmsPixels() << "\n";
            out << "result[" << idx
                << "].optimization.active_parameters: "
                << optimization.activeParameterCount() << "\n";
         }
      }
      return out.str();
   }

   std::string bundleRegistrationQualityReportText(
      const ossimString& label,
      const ossimString& summary,
      bool success,
      const ossimBundleAdjustmentRegistrationSource::RegistrationResult&
         result,
      const std::vector<ossimFilename>& writtenGeometryFiles)
   {
      std::ostringstream out;
      const ossim_autoreg::BundleAdjustmentResult& optimization =
         result.optimization();
      const ossim_autoreg::BundleConnectivityDiagnostics& connectivity =
         result.connectivity();

      out << "registration_type: bundle\n";
      out << "label: " << label << "\n";
      out << "success: " << (success ? "true" : "false") << "\n";
      out << "summary: " << summary << "\n";
      out << "session_open: "
          << (result.sessionOpen() ? "true" : "false") << "\n";
      out << "ran: " << (result.ran() ? "true" : "false") << "\n";
      out << "message: " << result.message() << "\n";
      out << "model_freedom_advisory: "
          << (result.modelFreedomAdvisory().empty()
                 ? std::string("ok")
                 : result.modelFreedomAdvisory()) << "\n";
      out << "edge_prune_policy: "
          << (result.edgePrunePolicyMessage().empty()
                 ? std::string("not_attempted")
                 : result.edgePrunePolicyMessage()) << "\n";
      out << "pair_count: " << result.pairResults().size() << "\n";
      for(std::size_t idx = 0; idx < result.pairResults().size(); ++idx)
      {
         const ossimBundleAdjustmentRegistrationSource::PairResult& pair =
            result.pairResults()[idx];
         out << "pair[" << idx << "].first_input_index: "
             << pair.firstInputIndex() << "\n";
         out << "pair[" << idx << "].second_input_index: "
             << pair.secondInputIndex() << "\n";
         out << "pair[" << idx << "].tie_points: "
             << pair.tiePoints().size() << "\n";
         out << "pair[" << idx << "].added_tie_points: "
             << pair.addedTiePointCount() << "\n";
         appendDenseAlternateReport(
            out,
            std::string("pair[") +
               ossimString::toString(static_cast<ossim_uint32>(idx)).string()
               + "]",
            pair);
      }
      out << "connectivity.image_count: "
          << connectivity.getImageCount() << "\n";
      out << "connectivity.anchor_enabled: "
          << (connectivity.getAnchorEnabled() ? "true" : "false") << "\n";
      out << "connectivity.connected: "
          << (connectivity.getConnected() ? "true" : "false") << "\n";
      out << "connectivity.edge_count: "
          << connectivity.getEdges().size() << "\n";
      for(std::size_t idx = 0; idx < connectivity.getEdges().size(); ++idx)
      {
         const ossim_autoreg::BundleConnectivityEdge& edge =
            connectivity.getEdges()[idx];
         out << "connectivity.edge[" << idx << "]: image["
             << edge.getFirstImageIndex() << "] -> image["
             << edge.getSecondImageIndex() << "], tie_points="
             << edge.getTiePointCount() << "\n";
      }
      out << "connectivity.component_count: "
          << connectivity.getComponents().size() << "\n";
      if(optimization.ran())
      {
         out << "optimization.ran: true\n";
         out << "optimization.converged: "
             << (optimization.converged() ? "true" : "false") << "\n";
         out << "optimization.iterations: "
             << optimization.iterations() << "\n";
         out << "optimization.final_rmse_pixels: "
             << optimization.finalRmsPixels() << "\n";
         out << "optimization.valid_residuals: "
             << optimization.validResidualCount() << "\n";
         out << "optimization.active_parameters: "
             << optimization.activeParameterCount() << "\n";
         out << "optimization.active_image_blocks: "
             << optimization.activeImageBlockCount() << "\n";
         out << "optimization.normal_equation_block_pairs: "
             << optimization.normalEquationBlockPairCount() << "\n";
         out << "optimization.normal_equation_block_pair_capacity: "
             << optimization.normalEquationBlockPairCapacity() << "\n";
         out << "optimization.normal_equation_block_pair_density: "
             << optimization.normalEquationBlockPairDensity() << "\n";
         out << "optimization.solver_backend: "
             << optimization.solverBackendName() << "\n";
      }
      else
      {
         out << "optimization.ran: false\n";
      }
      out << "written_geometry_count: " << writtenGeometryFiles.size()
          << "\n";
      for(std::size_t idx = 0; idx < writtenGeometryFiles.size(); ++idx)
      {
         out << "written_geometry[" << idx << "]: "
             << writtenGeometryFiles[idx] << "\n";
      }
      return out.str();
   }

   enum RegistrationSetupApproach
   {
      REGISTRATION_SETUP_FIXED_AUTO = 0,
      REGISTRATION_SETUP_FIXED_MANUAL = 1,
      REGISTRATION_SETUP_BUNDLE_ALL_FLOATING = 2,
      REGISTRATION_SETUP_BUNDLE_ANCHORED = 3
   };

   struct RegistrationSetupOptions
   {
      RegistrationSetupApproach approach;
      std::string matchMethod;
      std::string resamplerType;
      int chipSize;
      int searchRadius;
      int gridSpacing;
      double minScore;
      double viewGsd;
      std::size_t maxTiePoints;

      RegistrationSetupOptions()
      : approach(REGISTRATION_SETUP_FIXED_AUTO),
        matchMethod(preferredRegistrationMatchMethod()),
        resamplerType("cubic"),
        chipSize(31),
        searchRadius(64),
        gridSpacing(128),
        minScore(0.6),
        viewGsd(0.0),
        maxTiePoints(300)
      {
      }
   };

   RegistrationSetupOptions registrationSetupDefaults(
      RegistrationSetupApproach approach,
      const std::string& matchMethod)
   {
      RegistrationSetupOptions result;
      result.approach = approach;
      const bool bundle =
         approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING ||
         approach == REGISTRATION_SETUP_BUNDLE_ANCHORED;
      result.matchMethod = matchMethod.empty() ?
         (bundle ? preferredBundleMatchMethod() :
          (approach == REGISTRATION_SETUP_FIXED_AUTO ?
              std::string() :
              preferredRegistrationMatchMethod())) :
         matchMethod;
      result.resamplerType = "cubic";
      result.chipSize = 31;
      result.searchRadius = 36;
      result.gridSpacing = 96;
      result.minScore = 0.6;
      result.viewGsd = 0.0;
      result.maxTiePoints = 300;

      if(bundle)
      {
         ossim_autoreg::AutoRegistrationOptions defaults;
         defaults.generator().setMatchMethod(result.matchMethod);
         ossim_autoreg::applyBundleRegistrationDefaults(
            defaults,
            approach == REGISTRATION_SETUP_BUNDLE_ANCHORED);

         result.resamplerType = defaults.generator().resamplerType();
         result.chipSize = defaults.generator().chipSize();
         result.searchRadius = defaults.generator().searchRadius();
         result.gridSpacing = defaults.generator().gridSpacing();
         result.minScore = defaults.generator().minScore();
         result.viewGsd = defaults.generator().viewGsd();
         result.maxTiePoints = defaults.generator().maxTiePoints();
         return result;
      }

      if(result.matchMethod == "mixed-phase-orb" ||
         result.matchMethod == "mixed-hybrid-orb")
      {
         result.searchRadius = 96;
         result.gridSpacing = 128;
         result.maxTiePoints = bundle ? 200 : 300;
      }
      else if(result.matchMethod == "phase-correlation" ||
              result.matchMethod == "opencv-phase-correlation")
      {
         result.searchRadius = 36;
         result.gridSpacing = 96;
         result.maxTiePoints = bundle ? 200 : 300;
      }
      else if(result.matchMethod == "hybrid-phase-ncc")
      {
         result.searchRadius = 64;
         result.gridSpacing = 128;
         result.maxTiePoints = bundle ? 200 : 300;
      }
      else if(result.matchMethod == "spatial-ncc")
      {
         result.searchRadius = 96;
         result.gridSpacing = 192;
         result.minScore = 0.65;
         result.maxTiePoints = bundle ? 150 : 250;
      }
      else if(result.matchMethod == "opencv-sift")
      {
         result.searchRadius = 128;
         result.gridSpacing = 128;
         result.minScore = 0.6;
         result.maxTiePoints = bundle ? 250 : 500;
      }
      else if(result.matchMethod == "opencv-orb")
      {
         result.searchRadius = 128;
         result.gridSpacing = 128;
         result.minScore = 0.55;
         result.maxTiePoints = bundle ? 300 : 750;
      }
      else if(result.matchMethod == "opencv-akaze")
      {
         result.searchRadius = 128;
         result.gridSpacing = 128;
         result.minScore = 0.55;
         result.maxTiePoints = bundle ? 250 : 500;
      }
      else if(result.matchMethod == "opencv-brisk")
      {
         result.searchRadius = 128;
         result.gridSpacing = 128;
         result.minScore = 0.55;
         result.maxTiePoints = bundle ? 250 : 500;
      }
      else if(result.matchMethod == "opencv-gftt-lk")
      {
         result.searchRadius = 96;
         result.gridSpacing = 96;
         result.minScore = 0.55;
         result.maxTiePoints = bundle ? 250 : 500;
      }

      if(approach == REGISTRATION_SETUP_FIXED_MANUAL)
      {
         result.searchRadius = std::max(result.searchRadius, 128);
         result.maxTiePoints = 0;
      }
      return result;
   }

   class RegistrationSetupDialog : public QDialog
   {
   public:
      RegistrationSetupDialog(QWidget* parent = 0)
      : QDialog(parent),
        m_approach(0),
        m_matchMethod(0),
        m_resampler(0),
        m_chipSize(0),
        m_searchRadius(0),
        m_gridSpacing(0),
        m_minScore(0),
        m_viewGsd(0),
        m_maxTiePoints(0)
      {
         setWindowTitle("Registration Setup");

         m_approach = new QComboBox(this);
         m_approach->addItem(
            "Fixed to Floating Auto (Recommended)",
            REGISTRATION_SETUP_FIXED_AUTO);
         m_approach->addItem(
            "Bundle Anchored",
            REGISTRATION_SETUP_BUNDLE_ANCHORED);
         m_approach->addItem(
            "Bundle All-Floating",
            REGISTRATION_SETUP_BUNDLE_ALL_FLOATING);
         m_approach->addItem(
            "Fixed Manual",
            REGISTRATION_SETUP_FIXED_MANUAL);

         m_matchMethod = new QComboBox(this);
         addMatchMethod("Adaptive Auto (Recommended)", "");
         addAvailableMatchMethod("Adaptive Mixed Phase/ORB", "mixed-phase-orb");
         addAvailableMatchMethod("Adaptive Mixed Hybrid/ORB", "mixed-hybrid-orb");
         addAvailableMatchMethod("OpenCV SIFT", "opencv-sift");
         addAvailableMatchMethod("OpenCV ORB", "opencv-orb");
         addAvailableMatchMethod("OpenCV AKAZE", "opencv-akaze");
         addAvailableMatchMethod("OpenCV BRISK", "opencv-brisk");
         addAvailableMatchMethod("OpenCV GFTT/LK", "opencv-gftt-lk");
         addAvailableMatchMethod("OpenCV Phase", "opencv-phase-correlation");
         addMatchMethod("Phase Correlation", "phase-correlation");
         addMatchMethod("Hybrid Phase/NCC", "hybrid-phase-ncc");
         addMatchMethod("Spatial NCC", "spatial-ncc");

         std::vector<std::string> typeNames =
            ossim_autoreg::TiePointGeneratorFactory::instance()->typeNames();
         for(const std::string& typeName : typeNames)
         {
            if(m_matchMethod->findData(
                  QString::fromStdString(typeName)) < 0)
            {
               addMatchMethod(QString::fromStdString(typeName),
                              typeName.c_str());
            }
         }
         const int preferredIndex = m_matchMethod->findData(QString());
         if(preferredIndex >= 0)
            m_matchMethod->setCurrentIndex(preferredIndex);

         m_resampler = new QComboBox(this);
         m_resampler->addItem("cubic", "cubic");
         m_resampler->addItem("bilinear", "bilinear");
         m_resampler->addItem("nearest", "nearest_neighbor");
         m_resampler->addItem("sinc", "sinc");

         m_chipSize = new QSpinBox(this);
         m_chipSize->setRange(5, 255);
         m_chipSize->setSingleStep(2);
         m_chipSize->setValue(31);

         m_searchRadius = new QSpinBox(this);
         m_searchRadius->setRange(1, 4096);
         m_searchRadius->setValue(64);

         m_gridSpacing = new QSpinBox(this);
         m_gridSpacing->setRange(16, 8192);
         m_gridSpacing->setValue(128);

         m_minScore = new QDoubleSpinBox(this);
         m_minScore->setRange(0.0, 1.0);
         m_minScore->setDecimals(3);
         m_minScore->setSingleStep(0.05);
         m_minScore->setValue(0.6);

         m_viewGsd = new QDoubleSpinBox(this);
         m_viewGsd->setRange(-100.0, 1000000.0);
         m_viewGsd->setDecimals(3);
         m_viewGsd->setSingleStep(0.25);
         m_viewGsd->setValue(0.0);

         m_maxTiePoints = new QSpinBox(this);
         m_maxTiePoints->setRange(0, 100000);
         m_maxTiePoints->setValue(300);

         connect(m_approach,
                 static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                 [this](int) { applySelectedDefaults(); });
         connect(m_matchMethod,
                 static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                 [this](int) { applySelectedDefaults(); });

         QFormLayout* form = new QFormLayout();
         form->addRow("Approach", m_approach);
         form->addRow("Matcher", m_matchMethod);
         form->addRow("Resampler", m_resampler);
         form->addRow("Chip size", m_chipSize);
         form->addRow("Search radius", m_searchRadius);
         form->addRow("Grid spacing", m_gridSpacing);
         form->addRow("Minimum score", m_minScore);
         form->addRow("View GSD", m_viewGsd);
         form->addRow("Max ties", m_maxTiePoints);

         QGroupBox* optionsBox = new QGroupBox("Options", this);
         optionsBox->setLayout(form);

         QDialogButtonBox* buttons =
            new QDialogButtonBox(QDialogButtonBox::Ok |
                                 QDialogButtonBox::Cancel,
                                 Qt::Horizontal,
                                 this);
         connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
         connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

         QVBoxLayout* layout = new QVBoxLayout();
         layout->addWidget(optionsBox);
         layout->addWidget(buttons);
         setLayout(layout);
         applySelectedDefaults();
      }

      RegistrationSetupOptions options() const
      {
         RegistrationSetupOptions result;
         result.approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         result.matchMethod =
            m_matchMethod->itemData(m_matchMethod->currentIndex()).
               toString().toStdString();
         result.resamplerType =
            m_resampler->itemData(m_resampler->currentIndex()).
               toString().toStdString();
         result.chipSize = m_chipSize->value();
         if((result.chipSize % 2) == 0)
            ++result.chipSize;
         result.searchRadius = m_searchRadius->value();
         result.gridSpacing = m_gridSpacing->value();
         result.minScore = m_minScore->value();
         result.viewGsd = m_viewGsd->value();
         result.maxTiePoints =
            static_cast<std::size_t>(m_maxTiePoints->value());
         return result;
      }

   private:
      void applySelectedDefaults()
      {
         const RegistrationSetupApproach approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         const std::string matchMethod =
            m_matchMethod->itemData(m_matchMethod->currentIndex()).
               toString().toStdString();
         const RegistrationSetupOptions defaults =
            registrationSetupDefaults(approach, matchMethod);

         const int resamplerIndex =
            m_resampler->findData(QString::fromStdString(
               defaults.resamplerType));
         if(resamplerIndex >= 0)
            m_resampler->setCurrentIndex(resamplerIndex);
         m_chipSize->setValue(defaults.chipSize);
         m_searchRadius->setValue(defaults.searchRadius);
         m_gridSpacing->setValue(defaults.gridSpacing);
         m_minScore->setValue(defaults.minScore);
         m_viewGsd->setValue(defaults.viewGsd);
         m_maxTiePoints->setValue(
            static_cast<int>(defaults.maxTiePoints));
      }

      void addMatchMethod(const QString& label, const QString& method)
      {
         m_matchMethod->addItem(label, method);
      }

      void addAvailableMatchMethod(const QString& label, const QString& method)
      {
         if(registrationTieGeneratorAvailable(method.toStdString()))
            addMatchMethod(label, method);
      }

      QComboBox* m_approach;
      QComboBox* m_matchMethod;
      QComboBox* m_resampler;
      QSpinBox* m_chipSize;
      QSpinBox* m_searchRadius;
      QSpinBox* m_gridSpacing;
      QDoubleSpinBox* m_minScore;
      QDoubleSpinBox* m_viewGsd;
      QSpinBox* m_maxTiePoints;
   };

   void applyRegistrationSetupTieOptions(
      ossim_autoreg::TiePointGenerationOptions& tiePointOptions,
      const RegistrationSetupOptions& setupOptions)
   {
      tiePointOptions.matchMethod() = setupOptions.matchMethod;
      tiePointOptions.resamplerType() = setupOptions.resamplerType;
      tiePointOptions.chipSize() = setupOptions.chipSize;
      tiePointOptions.searchRadius() = setupOptions.searchRadius;
      tiePointOptions.gridSpacing() = setupOptions.gridSpacing;
      tiePointOptions.minScore() = setupOptions.minScore;
      tiePointOptions.viewGsd() = setupOptions.viewGsd;
      tiePointOptions.maxTiePoints() = setupOptions.maxTiePoints;
   }

   QString registeredNodeName(const std::string& matchMethod)
   {
      return QString("Registered: %1")
         .arg(QString::fromStdString(
            matchMethod.empty() ? std::string("hybrid-phase-ncc") :
                                  matchMethod));
   }
}
#endif

void ossimGui::DataManagerWidget::RefreshVisitor::visit(ossimObject* obj)
{
   if(!hasVisited(obj))
   {
      ossimVisitor::visit(obj);
      ossimRefPtr<DataManager::Node> node = dynamic_cast<DataManager::Node*> (obj);
      if(node.valid())
      {
         DataManager::NodeListType nodes(1);
         nodes[0] = node.get();
         m_widget->populateTreeWithNodes(nodes);
      }
   }
}

namespace ossimGui
{
   class OSSIMGUI_DLL ImageStagerJob : public ProcessInterfaceJob
   {
   public:
      enum StagerFlags
      {
         STAGE_NONE = 0,
         STAGE_OVERVIEWS = 1,
         STAGE_FULL_HISTOGRAMS = 2,
         STAGE_FAST_HISTOGRAMS = 4,
         STAGE_ALL = (STAGE_OVERVIEWS|STAGE_FULL_HISTOGRAMS|STAGE_FAST_HISTOGRAMS)
      };
      ImageStagerJob(ossimImageHandler* isource, StagerFlags f = STAGE_NONE)
      :ProcessInterfaceJob(),
      m_source(isource),
      m_flags(f)
      {
      }
      
      void setOverviewType(const ossimString& type){m_overviewType = type;}
      void setStagerFlag(int flag, bool on=true)
      {
         if(on)
         {
            m_flags = static_cast<StagerFlags>((m_flags|flag)&STAGE_ALL);
         }
         else
         {
            m_flags = static_cast<StagerFlags>((m_flags&(~flag))&STAGE_ALL);
         }
      }
      int flags()const{return m_flags;}
      
      
      ossimImageHandler* handler(){return m_source.get();}
      const ossimImageHandler* handler()const{return m_source.get();}
      
   protected:
      virtual void run()
      {
         if(m_flags&STAGE_OVERVIEWS)
         {
            ossimRefPtr<ossimOverviewBuilderBase> overviewBuilder = ossimOverviewBuilderFactoryRegistry::instance()->createBuilder(m_overviewType);
            if(overviewBuilder.valid())
            {
               if(!m_source->createDefaultHistogramFilename().exists())
               {
                  overviewBuilder->setHistogramMode(OSSIM_HISTO_MODE_NORMAL);
               }
               ossimString baseName = ("entry " + ossimString::toString(m_source->getCurrentEntry()) + ": "+  m_source->getFilename());
               setName(("Building Overviews: " + baseName).c_str());
               ossimRefPtr<ossimImageHandler> dupHandler = (ossimImageHandler*) m_source->dup();
               if(dupHandler.valid())
               {
                  overviewBuilder->setInputSource(dupHandler.get());
                  m_obj =  overviewBuilder.get();
                  ProcessInterfaceJob::run();
               }
               dupHandler = 0;
               m_obj = 0;
            }
            overviewBuilder = 0;
         }
      }
      void stageOverviews();
      void stageHistograms();
      ossimRefPtr<ossimImageHandler> m_source;
      ossimString m_overviewType;
      StagerFlags m_flags;
   };
   class ImageStagerJobCallback : public ossimJobCallback
   {
   public:
      ImageStagerJobCallback(DataManagerWidget* w, DataManagerNodeItem* i=0)
      :m_dataManagerWidget(w),
      m_item(i)
      {
      }
      virtual void finished(std::shared_ptr<ossimJob> job)
      {
         std::shared_ptr<ImageStagerJob> stagerJob = std::dynamic_pointer_cast<ImageStagerJob> (job);
         
         if(stagerJob)
         {
            if(m_item)
            {
               DataManagerWidgetEvent* evt = new DataManagerWidgetEvent(DataManagerWidgetEvent::COMMAND_RESET);
               evt->setItemList(m_item);
               
               QApplication::postEvent(m_dataManagerWidget, evt);
            }
         }
         stagerJob.reset();
      }
      
   protected:
      DataManagerWidget* m_dataManagerWidget;  
      DataManagerNodeItem* m_item;
      
   };

#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   class RegistrationSourceJob : public ossimJob
   {
   public:
      RegistrationSourceJob(ossimFixedRegistrationSource* registrationSource,
                            const ossimString& label)
      :m_registrationSource(registrationSource),
       m_label(label),
       m_success(false)
      {
         setId("ossimGui::RegistrationSourceJob");
         setName("Register: " + m_label);
      }

      bool success()const{return m_success;}
      const ossimString& resultSummary()const{return m_resultSummary;}
      const DataManagerWidgetEvent::HandlerListType& sourceHandlersToReload()const
      {
         return m_sourceHandlersToReload;
      }

      virtual void start()
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

   protected:
      void updateProgressName(
         const ossimFixedRegistrationSource::ProgressInfo& progress)
      {
         ossimString name = "Register";
         if(!progress.message().empty())
         {
            name += " ";
            name += progress.message().c_str();
         }
         name += ": ";
         name += m_label;
         setName(name);
         if(!progress.message().empty())
         {
            setDescription(progress.message().c_str());
         }
         setPercentComplete(progress.percentComplete());
      }

      ossimFilename defaultGeometryOutput(
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
                     ossimString::toString(input.inputIndex()) +
                     ".geom";
         }
         else
         {
            result.setExtension("geom");
         }
         return result;
      }

      virtual void run()
      {
         setPercentComplete(0.0);
         m_sourceHandlersToReload.clear();
         if(m_registrationSource.valid())
         {
            m_registrationSource->setCancelCallback([this]() {
               return this->isCanceled();
            });
            m_registrationSource->setProgressCallback(
               [this](
                  const ossimFixedRegistrationSource::ProgressInfo& progress) {
                  updateProgressName(progress);
               });
            m_success = m_registrationSource->executeRegistration();
            m_registrationSource->setCancelCallback(
               std::function<bool()>());
            m_registrationSource->setProgressCallback(
               std::function<void(
                  const ossimFixedRegistrationSource::ProgressInfo&)>());

            const std::vector<ossimFixedRegistrationSource::RegistrationResult>& results =
               m_registrationSource->registrationResults();
            ossim_uint32 tiePointCount = 0;
            ossim_uint32 successfulPairs = 0;
            ossim_uint32 propagatedGeometries = 0;
            std::vector<ossimFilename> writtenGeometryFiles;
            std::vector<ossimFilename> failedGeometryFiles;
            ossim_uint32 idx = 0;
            for(idx = 0; idx < results.size(); ++idx)
            {
               tiePointCount += static_cast<ossim_uint32>(results[idx].tiePoints().size());
               if(results[idx].success() && !isCanceled())
               {
                  ++successfulPairs;
                  const ossimFixedRegistrationSource::InputWrapper* input =
                     m_registrationSource->inputWrapper(results[idx].inputIndex());
                  if(input && m_registrationSource->optimizeEnabled())
                  {
                     const ossimFilename outputFile = defaultGeometryOutput(*input);
                     const ossim_autoreg::RegistrationSession* session =
                        m_registrationSource->registrationSession(
                           results[idx].floatingInputIndex());
                     if(session &&
                        input->applyAdjustableParametersToSource(
                           session->movingAdjustableParameters(),
                           m_registrationSource->adjustmentDescription(),
                           results[idx].adjustmentIndex()))
                     {
                        ++propagatedGeometries;
                     }
                     if(session && session->saveMovingGeometry(outputFile))
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

            if(results.size() == 1 &&
               !results[0].success() &&
               results[0].tiePoints().empty() &&
               !results[0].message().empty())
            {
               m_resultSummary = results[0].message().c_str();
            }
            else
            {
               m_resultSummary =
                  ossimString::toString(successfulPairs) + "/" +
                  ossimString::toString(static_cast<ossim_uint32>(results.size())) +
                  " pair(s), " + ossimString::toString(tiePointCount) +
                  " tie point(s)";
               for(idx = 0; idx < results.size(); ++idx)
               {
                  if(!results[idx].success() && !results[idx].message().empty())
                  {
                     m_resultSummary += "; input ";
                     m_resultSummary += ossimString::toString(results[idx].inputIndex());
                     m_resultSummary += ": ";
                     m_resultSummary += results[idx].message().c_str();
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
            if(!m_success && m_registrationSource.valid())
            {
               const std::string inputStatus =
                  m_registrationSource->inputStatusSummary();
               if(!inputStatus.empty())
               {
                  m_resultSummary += " - ";
                  m_resultSummary += inputStatus.c_str();
               }
               const std::string autoStatus =
                  m_registrationSource->autoRegistrationSettingsSummary();
               if(!autoStatus.empty())
               {
                  m_resultSummary += " - ";
                  m_resultSummary += autoStatus.c_str();
               }
            }
            const ossimFilename reportPath =
               registrationQualityReportPath(m_label, "fixed-registration");
            const bool wroteReport = writeRegistrationQualityReport(
               reportPath,
               fixedRegistrationQualityReportText(
                  m_label,
                  m_resultSummary,
                  m_success,
                  results,
                  writtenGeometryFiles,
                  failedGeometryFiles));
            appendRegistrationQualityReportStatus(
               m_resultSummary,
               reportPath,
               wroteReport);
            setDescription(m_resultSummary);
            if(isCanceled())
            {
               setName("Registration canceled: " + m_label);
            }
            else
            {
               setName((m_success ? "Registered: " : "Registration failed: ") + m_label);
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

      ossimRefPtr<ossimFixedRegistrationSource> m_registrationSource;
      ossimString m_label;
      ossimString m_resultSummary;
      DataManagerWidgetEvent::HandlerListType m_sourceHandlersToReload;
      bool m_success;
   };

   class BundleRegistrationSourceJob : public ossimJob
   {
   public:
      BundleRegistrationSourceJob(
         ossimBundleAdjustmentRegistrationSource* registrationSource,
         const ossimString& label)
      :m_registrationSource(registrationSource),
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

      bool success()const{return m_success;}
      const ossimString& resultSummary()const{return m_resultSummary;}
      const DataManagerWidgetEvent::HandlerListType& sourceHandlersToReload()const
      {
         return m_sourceHandlersToReload;
      }

      virtual void start()
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

   protected:
      void updateProgressName(
         const ossimBundleAdjustmentRegistrationSource::ProgressInfo& progress)
      {
         ossimString name = "Bundle adjust";
         if(!progress.message().empty())
         {
            name += " ";
            name += progress.message().c_str();
         }
         name += ": ";
         name += m_label;
         setName(name);
         if(!progress.message().empty())
         {
            setDescription(progress.message().c_str());
         }
         setPercentComplete(progress.percentComplete());
      }

      ossimFilename defaultGeometryOutput(
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
                     ossimString::toString(input.inputIndex()) +
                     ".geom";
         }
         else
         {
            result.setExtension("geom");
         }
         return result;
      }

      virtual void run()
      {
         setPercentComplete(0.0);
         m_sourceHandlersToReload.clear();
         if(m_registrationSource.valid())
         {
            m_registrationSource->setCancelCallback([this]() {
               return this->isCanceled();
            });
            m_registrationSource->setProgressCallback(
               [this](const ossimBundleAdjustmentRegistrationSource::
                         ProgressInfo& progress) {
                  updateProgressName(progress);
               });
            m_success = m_registrationSource->executeRegistration();
            m_registrationSource->setCancelCallback(
               std::function<bool()>());
            m_registrationSource->setProgressCallback(
               std::function<void(
                  const ossimBundleAdjustmentRegistrationSource::
                     ProgressInfo&)>());

            const ossimBundleAdjustmentRegistrationSource::RegistrationResult&
               result = m_registrationSource->registrationResult();
            ossim_uint32 tiePointCount = 0;
            ossim_uint32 pairCount =
               static_cast<ossim_uint32>(result.pairResults().size());
            ossim_uint32 idx = 0;
            for(idx = 0; idx < result.pairResults().size(); ++idx)
            {
               tiePointCount += static_cast<ossim_uint32>(
                  result.pairResults()[idx].tiePoints().size());
            }

            if(m_registrationSource->allInputsFloating())
            {
               m_resultSummary = "all-floating, ";
            }
            else
            {
               m_resultSummary = "anchored input ";
               m_resultSummary += ossimString::toString(
                  m_registrationSource->anchorInputIndex()) + ", ";
            }
            m_resultSummary +=
               ossimString::toString(pairCount) + " pair(s), " +
               ossimString::toString(tiePointCount) + " tie point(s)";

            if(result.optimization().ran())
            {
               m_resultSummary += ", RMSE ";
               m_resultSummary += ossimString::toString(
                  result.optimization().finalRmsPixels());
            }
            const ossimString diagnosticsSummary =
               bundleRegistrationDiagnosticsSummary(result);
            if(!diagnosticsSummary.empty())
            {
               m_resultSummary += ", ";
               m_resultSummary += diagnosticsSummary;
            }

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
                     input =
                        m_registrationSource->inputWrapper(inputIndexes[idx]);
                  if(input)
                  {
                     outputGeometryFiles.push_back(
                        defaultGeometryOutput(*input));
                  }
               }

               if(outputGeometryFiles.size() == inputIndexes.size() &&
                  m_registrationSource->saveGeometries(outputGeometryFiles))
               {
                  writtenGeometryFiles = outputGeometryFiles;
                  for(idx = 0; idx < inputIndexes.size(); ++idx)
                  {
                     const ossimBundleAdjustmentRegistrationSource::
                        InputWrapper* input =
                           m_registrationSource->inputWrapper(
                              inputIndexes[idx]);
                     if(input)
                     {
                        ossimImageHandler* sourceHandler =
                           input->sourceHandler();
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
            if(!result.message().empty())
            {
               m_resultSummary += " - ";
               m_resultSummary += result.message().c_str();
            }

            const ossimFilename reportPath =
               registrationQualityReportPath(m_label, "bundle-registration");
            const bool wroteReport = writeRegistrationQualityReport(
               reportPath,
               bundleRegistrationQualityReportText(
                  m_label,
                  m_resultSummary,
                  m_success,
                  result,
                  writtenGeometryFiles));
            appendRegistrationQualityReportStatus(
               m_resultSummary,
               reportPath,
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

      ossimRefPtr<ossimBundleAdjustmentRegistrationSource>
         m_registrationSource;
      ossimString m_label;
      ossimString m_resultSummary;
      DataManagerWidgetEvent::HandlerListType m_sourceHandlersToReload;
      bool m_success;
   };

   class RegistrationSourceJobCallback : public ossimJobCallback
   {
   public:
      RegistrationSourceJobCallback(DataManagerWidget* widget,
                                    DataManagerNodeItem* item)
      :m_dataManagerWidget(widget),
       m_item(item)
      {
      }

      virtual void finished(std::shared_ptr<ossimJob> job)
      {
         if(m_dataManagerWidget&&m_item)
         {
            DataManagerWidgetEvent* evt =
               new DataManagerWidgetEvent(DataManagerWidgetEvent::COMMAND_REFRESH);
            evt->setItemList(m_item);
            std::shared_ptr<RegistrationSourceJob> registrationJob =
               std::dynamic_pointer_cast<RegistrationSourceJob>(job);
            if(registrationJob)
            {
               evt->setHandlerList(registrationJob->sourceHandlersToReload());
               if(!registrationJob->success())
               {
                  evt->setWarningMessage(
                     "Registration failed",
                     registrationJob->resultSummary().string());
               }
            }
            std::shared_ptr<BundleRegistrationSourceJob> bundleJob =
               std::dynamic_pointer_cast<BundleRegistrationSourceJob>(job);
            if(bundleJob)
            {
               evt->setHandlerList(bundleJob->sourceHandlersToReload());
               if(!bundleJob->success())
               {
                  evt->setWarningMessage(
                     "Bundle adjustment failed",
                     bundleJob->resultSummary().string());
               }
            }
            QCoreApplication::postEvent(m_dataManagerWidget, evt);
         }
         ossimJobCallback::finished(job);
      }

   protected:
      DataManagerWidget* m_dataManagerWidget;
      DataManagerNodeItem* m_item;
   };
#endif
}

class TestCycleVisitor :public ossimVisitor
{
public:
   
   TestCycleVisitor(int visitorType =VISIT_INPUTS, 
                    ossimConnectableObject* objToFind=0):ossimVisitor(visitorType),m_objectToFind(objToFind){}
   virtual ossimRefPtr<ossimVisitor> dup()const{return new TestCycleVisitor(*this);}
   virtual void reset()
   {
      m_objectFound=false;
   }
   virtual void visit(ossimConnectableObject* obj)
   {
      if(!hasVisited(obj))
      {
         ossimVisitor::visit(obj);
         
         ossim_int32 idx = obj->findInputIndex(m_objectToFind.get());
         if(idx >= 0)
         {
            m_objectFound = true;
         }
      }
   }
   void setObjectToFind(ossimConnectableObject* objToFind){m_objectToFind = objToFind;}
   bool found()const{return m_objectFound;}
   
protected:
   ossimRefPtr<ossimConnectableObject> m_objectToFind;
   bool                                m_objectFound;
};

namespace ossimGui
{
   class ImageOpenJobCallback : public ossimJobCallback
   {
   public:
      ImageOpenJobCallback(DataManagerWidget* widget, ossimRefPtr<DataManager> manager):m_dataManagerWidget(widget),m_dataManager(manager){}
      virtual void finished(std::shared_ptr<ossimJob> job)
      {
         if(m_dataManager.valid())
         {
            std::shared_ptr<ossimGui::OpenImageUrlJob> imageOpenJob = std::dynamic_pointer_cast<ossimGui::OpenImageUrlJob> (job);
            if(imageOpenJob)
            {
               ossimGui::HandlerList& handlerList =imageOpenJob->handlerList();
               ossim_uint32 idx = 0;
               ossim_uint32 nImages = handlerList.size();
               DataManager::NodeListType nodeList;;
               for(idx = 0; idx < nImages; ++idx)
               {
                  ossimRefPtr<DataManager::Node> node = m_dataManager->addSource(handlerList[idx].get());
                  if(node.valid())
                  {
                     ossimRefPtr<DataManager::Node> chain =  m_dataManager->createDefaultImageChain(node.get());
                     if(chain.valid())
                     {
                        nodeList.push_back(chain.get());
                     }
                  }
               }
               if(!nodeList.empty())
               {
                  DataManagerEvent* e = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
                  e->setNodeList(nodeList);
                  QCoreApplication::postEvent(m_dataManagerWidget->mainWindow(), e);
               }
            }
         }
      }
      
   protected:
      DataManagerWidget*       m_dataManagerWidget;
      ossimRefPtr<DataManager> m_dataManager;
   };
}

ossimGui::DataManagerWidget* ossimGui::DataManagerItem::dataManagerWidget()
{
   return dynamic_cast<ossimGui::DataManagerWidget*>(QTreeWidgetItem::treeWidget());
}


ossimGui::DataManager* ossimGui::DataManagerItem::dataManager()
{
   DataManagerWidget* w = dataManagerWidget();
   if(w) return w->dataManager();
   
   return 0;
}

void  ossimGui::DataManagerItem::clearChildren()
{
   QList<QTreeWidgetItem*> children = takeChildren();
   QList<QTreeWidgetItem*>::iterator iter= children.begin();
   while(iter != children.end())
   {
      delete *iter;
      ++iter;
   }
}

ossimGui::DataManagerInputConnectionItem::DataManagerInputConnectionItem(DataManager::Node* node)
:DataManagerItem()
{
   setObject(node);
}

ossimGui::DataManagerInputConnectionItem::~DataManagerInputConnectionItem()
{
   m_node = 0;
}

void ossimGui::DataManagerInputConnectionItem::setObject(ossimObject* node)
{
   m_node = dynamic_cast<DataManager::Node*>(node);
   DataManagerItem::setObject(m_node.get());
   if(m_node.valid())
   {
      setText(0, m_node->name());
   }
   else
   {
      setText(0, "<empty>");
   }
}  
   
ossimGui::DataManagerInputConnectionFolder::DataManagerInputConnectionFolder(DataManager::Node* node)
:DataManagerFolder(),m_node(node)
{
   setText(0, "Inputs");
   setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
}

ossimGui::DataManagerInputConnectionFolder::~DataManagerInputConnectionFolder()
{
   m_node = 0;
}

void ossimGui::DataManagerInputConnectionFolder::getInputs(DataManager::NodeListType& result)
{
   if(m_node.valid())
   {
      ossimConnectableObject* connectable = m_node->getObjectAs<ossimConnectableObject>();
      if(connectable)
      {
         ossimConnectableObject::ConnectableObjectList& inputList = connectable->getInputList();
         if(!inputList.empty())
         {
            ossimRefPtr<DataManager> manager = dataManager();
            if(manager.valid())
            {
               ossim_uint32 idx = 0;
               for(idx = 0; idx < inputList.size();++idx)
               {
                  ossimRefPtr<DataManager::Node> node;
                  if(inputList[idx].get())
                  {
                     node = manager->findNode(inputList[idx].get());
                  }
                  else 
                  {
                     node = 0;
                  }

                  result.push_back(node.get());
               }
            }
         }
      }
   }
}

void ossimGui::DataManagerInputConnectionFolder::setObject(ossimObject* node)
{
   m_node = dynamic_cast<DataManager::Node*>(node);
}   

void ossimGui::DataManagerInputConnectionFolder::populateChildren()
{
   DataManager::NodeListType inputList;
   getInputs(inputList);
   ossim_int32 nChildren = childCount();
   if((inputList.size() == (ossim_uint32)childCount())&&!inputList.empty())
   {
      ossim_int32 itemIdx = 0;
      for(itemIdx = 0; itemIdx < nChildren; ++itemIdx)
      {
         DataManagerInputConnectionItem* item = dynamic_cast<DataManagerInputConnectionItem*>(child(itemIdx));
         if(item)
         {
            item->setObject(inputList[itemIdx].get());
         }
      }
   }
   else
   {
      QList<QTreeWidgetItem *> children = takeChildren();
      QList<QTreeWidgetItem *>::iterator iter = children.begin();
      while(iter!=children.end())
      {
         DataManagerInputConnectionItem* nodeItem = dynamic_cast<DataManagerInputConnectionItem*> (*iter);
         if(nodeItem) nodeItem->setObject(0); // clear the node outs
         delete (*iter);
         ++iter;
      }
      children.clear();
      if(!inputList.empty())
      {
         DataManager::NodeListType::iterator inputListIter = inputList.begin();
         while(inputListIter != inputList.end())
         {
            DataManagerInputConnectionItem* item = new DataManagerInputConnectionItem((*inputListIter).get());
            addChild(item);
            ++inputListIter;
         }
      }
   }
   
}

ossimConnectableObject* ossimGui::DataManagerInputConnectionFolder::connectableObject()
{
   return m_node.valid()?m_node->getObjectAs<ossimConnectableObject>():0;
}
const ossimConnectableObject* ossimGui::DataManagerInputConnectionFolder::connectableObject()const
{
   return m_node.valid()?m_node->getObjectAs<ossimConnectableObject>():0;
}

void ossimGui::DataManagerInputConnectionFolder::connect(QList<DataManagerItem*>& nodeItems, DataManagerInputConnectionItem* targetItem)
{   
   if(nodeItems.empty()) return;
   ossimRefPtr<ossimConnectableObject> connectable = connectableObject();
   if(connectable.valid())
   {
      // we need to check for cycles and also the validity. 
      // For now lets just make sure we can do the connections
      //
      DataManagerNodeItem* parentNodeItem = parentItemAs<DataManagerNodeItem>();
      if(parentNodeItem) parentNodeItem->setNodeListenerEnabled(false);
      QList<DataManagerItem*>::iterator iter = nodeItems.begin();
     
      // if we are dropping on the folder then just append the new items
      if(!targetItem||!targetItem->object())
      {
        bool objectsFound = false;
         TestCycleVisitor visitorA; // be used to test if this object appears in any inputs
         TestCycleVisitor visitorB(ossimVisitor::VISIT_NONE); // will be used to test immediate connections
       // find first available slot
         ossimRefPtr<ossimConnectableObject> thisNode = objectAsNode()?objectAsNode()->getObjectAs<ossimConnectableObject>():0;
         while(iter != nodeItems.end())
         {
            ossimRefPtr<DataManager::Node> n = (*iter)->objectAsNode();
            if(n.valid())
            {               
               ossimRefPtr<ossimConnectableObject> connectableNodeToAdd = n->getObjectAs<ossimConnectableObject>();
               visitorA.setObjectToFind(thisNode.get());
               visitorA.reset();
               connectable->accept(visitorA);
               if(!visitorA.found())
               {
                  visitorB.reset();
                  visitorB.setObjectToFind(connectableNodeToAdd.get());
                  thisNode->accept(visitorB);
                  if(!visitorB.found())
                  {
                     if(connectable!=connectableNodeToAdd.get())
                     {
                        connectable->connectMyInputTo(connectableNodeToAdd.get());
                     }
                     else
                     {
                        objectsFound = true;
                     }
               
                  }
                  else 
                  {
                     objectsFound = true;
                  }
               }
               else 
               {
                  objectsFound = true;
               }

            }
            
            ++iter;
         }
         if(objectsFound) 
         {
            QMessageBox::warning(treeWidget(), "Warning", "Some inputs were already found and were not added.");
         }
      }
      else if(targetItem)
      {
         int childIdx = indexOfChild(targetItem);
         // if the target item is a child of this folder
         if(childIdx >= 0)
         {
            ossimConnectableObject::ConnectableObjectList inputList = connectable->getInputList();
            if(connectable->getInputListIsFixedFlag()&&targetItem->objectAsNode()&&(*iter)->objectAsNode())
            {
               ossimRefPtr<ossimConnectableObject> itemToInsert = (*iter)->objectAsNode()->getObjectAs<ossimConnectableObject>();
               ossimRefPtr<ossimConnectableObject> target       = targetItem->objectAsNode()->getObjectAs<ossimConnectableObject>();
               
               ossimConnectableObject::ConnectableObjectList::iterator inputListIter  = std::find(inputList.begin(), inputList.end(), itemToInsert.get());
               ossimConnectableObject::ConnectableObjectList::iterator targetListIter = std::find(inputList.begin(), inputList.end(), target.get());
               // replacing a connection
               TestCycleVisitor visitor;
               if(nodeItems.size() == 1)
               {
                  if((inputListIter!=inputList.end())&&
                     (targetListIter!=inputList.end()))
                  {
                     std::iter_swap(inputListIter, targetListIter);
                  }
                  else if(targetListIter!=inputList.end())
                  {
                     visitor.setObjectToFind(itemToInsert.get());
                     visitor.reset();
                     connectable->accept(visitor);
                     if(!visitor.found())
                     {
                        visitor.setObjectToFind(connectable.get());
                        visitor.reset();
                        itemToInsert->accept(visitor);
                        if(!visitor.found())
                        {
                           if(itemToInsert.get() != connectable.get())
                           {
                              *targetListIter = itemToInsert.get();
                           }
                           else
                           {
                              QMessageBox::warning(treeWidget(), "Warning", "Cannot connect back to yourself.");
                           }
                        }
                        else 
                        {
                           QMessageBox::warning(treeWidget(), "Warning", "Input was already found and was not added.");
                        }

                     }
                     else 
                     {
                        QMessageBox::warning(treeWidget(), "Warning", "Input was already found and was not added.");
                     }

                  }
               }
               else 
               {
                  // error for now
               }
            }
            else if(targetItem->objectAsNode())
            {
               TestCycleVisitor visitor;
               bool cycleFlag = false;
               if((ossim_uint32)childIdx < inputList.size())
               {
                  ossimRefPtr<ossimConnectableObject> target       = targetItem->objectAsNode()->getObjectAs<ossimConnectableObject>();
                  while(iter != nodeItems.end())
                  {
                     if((*iter)->objectAsNode())
                     {
                        ossimRefPtr<ossimConnectableObject> itemToInsert = (*iter)->objectAsNode()->getObjectAs<ossimConnectableObject>();
                        
                        ossimConnectableObject::ConnectableObjectList::iterator inputListIter  = std::find(inputList.begin(), inputList.end(), itemToInsert.get());
                        ossimConnectableObject::ConnectableObjectList::iterator targetListIter = std::find(inputList.begin(), inputList.end(), target.get());
                        if((*iter)->parent() != this)
                        {
                           if(targetListIter!=inputList.end())
                           {
                              if(inputListIter == inputList.end())
                              {
                                 visitor.setObjectToFind(itemToInsert.get());
                                 visitor.reset();
                                 connectable->accept(visitor);
                                 if(!visitor.found())
                                 {
                                    visitor.setObjectToFind(connectable.get());
                                    visitor.reset();
                                    itemToInsert->accept(visitor);
                                    if(!visitor.found())
                                    {
                                       if(connectable != itemToInsert.get())
                                       {
                                          inputList.insert(targetListIter, itemToInsert.get());
                                       }
                                       else 
                                       {
                                          cycleFlag = true;
                                       }

                                    }
                                    else 
                                    {
                                       cycleFlag = true;
                                    }
                                 }
                                 else 
                                 {
                                    cycleFlag = true;
                                 }
                              }
                           }
                        }
                        else 
                        {
                           // need to do a move layer
                           if((inputListIter!=inputList.end())&&
                              (targetListIter!=inputList.end()))
                           {
                              // are from same parent and moving layers
                              //
                              inputList.erase(inputListIter);
                              targetListIter = std::find(inputList.begin(), inputList.end(), target.get());
                              if(targetListIter != inputList.end())
                              {
                                 inputList.insert(targetListIter, itemToInsert.get());
                              }
                           }
                        }
                     }
                     ++iter;
                  }
               }
               if(cycleFlag)
               {
                  QMessageBox::warning(treeWidget(), "Warning", "Some inputs were already found and were not added.");
               }
            }

            connectable->connectInputList(inputList);
         }
      }
      populateChildren();
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimEventVisitor visitor(refreshEvent.get());
      connectable->accept(visitor);
      if(parentNodeItem) parentNodeItem->setNodeListenerEnabled(true);
   }
}

void ossimGui::DataManagerInputConnectionFolder::disconnectSelected()
{
   
   DataManagerNodeItem* parentNodeItem = parentItemAs<DataManagerNodeItem>();
   if(parentNodeItem) parentNodeItem->setNodeListenerEnabled(false);
   ossim_int32 childIdx = 0;
   ossim_int32 nChildren = childCount();
   ossimRefPtr<ossimConnectableObject> connectable = connectableObject();
   bool modified = false;
   for(childIdx=0;childIdx<nChildren;++childIdx)
   {
      if(child(childIdx)->isSelected())
      {
         DataManagerInputConnectionItem* item = dynamic_cast<DataManagerInputConnectionItem*> (child(childIdx));
         if(item&&item->object())
         {
            ossimRefPtr<ossimConnectableObject> obj = item->objectAsNode()->getObjectAs<ossimConnectableObject>();
            if(obj.valid())
            {
               connectable->disconnect(obj.get());
               modified = true;
            }
         }
      }
   }
   if(modified)
   {
      populateChildren();
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimEventVisitor visitor(refreshEvent.get());
      connectable->accept(visitor);
   }
   if(parentNodeItem) parentNodeItem->setNodeListenerEnabled(true);
}

ossimGui::DataManagerDisplayItem::DataManagerDisplayItem(DataManager::Node* node)
:DataManagerNodeItem(node)
{
   setAutoDelete(true);
}

ossimGui::DataManagerDisplayItem::~DataManagerDisplayItem()
{
   if(m_node.valid())
   {
      ConnectableDisplayObject* displayObject = m_node->getObjectAs<ConnectableDisplayObject>();
      if(displayObject&&displayObject->display())
      {
         displayObject->close();
      }
   }
}

void ossimGui::DataManagerDisplayItem::setObject(ossimObject* n)
{
   if(m_node.valid())
   {
      ConnectableDisplayObject* displayObject = m_node->getObjectAs<ConnectableDisplayObject>();
      if(displayObject&&displayObject->display())
      {
         displayObject->close();
      }
   }
   DataManagerNodeItem::setObject(n);
}


ossimGui::DataManagerDisplayFolder::DataManagerDisplayFolder()
:DataManagerFolder()
{
   setText(0, "Displays");
}

ossimGui::DataManagerRegistrationFolder::DataManagerRegistrationFolder()
:DataManagerFolder()
{
   setText(0, "Registration");
}

ossimGui::DataManagerRegistrationItem::DataManagerRegistrationItem(
   DataManager::Node* node)
:DataManagerNodeItem(node)
{
   m_autoDelete = false;
}

ossimGui::DataManagerRegistrationItem::~DataManagerRegistrationItem()
{
}

void ossimGui::DataManagerRegistrationItem::dropItems(
   QList<DataManagerItem*>& chainItemList)
{
   if(m_inputConnectionFolder)
   {
      m_inputConnectionFolder->connect(chainItemList);
      setExpanded(true);
      m_inputConnectionFolder->setExpanded(true);
   }
}

void ossimGui::DataManagerRegistrationItem::execute()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   if(!objectAsNode())
   {
      return;
   }

   ossimFixedRegistrationSource* registration =
      objectAsNode()->getObjectAs<ossimFixedRegistrationSource>();
   ossimBundleAdjustmentRegistrationSource* bundleRegistration =
      objectAsNode()->getObjectAs<ossimBundleAdjustmentRegistrationSource>();
   if(!registration && !bundleRegistration)
   {
      QMessageBox::warning(treeWidget(),
                           "Registration",
                           "Selected registration object is not supported.");
      return;
   }

   ossimConnectableObject* connectable =
      objectAsNode()->getObjectAs<ossimConnectableObject>();
   if(connectable && connectable->getInputList().empty())
   {
      QMessageBox::warning(treeWidget(),
                           "Registration",
                           "No inputs are connected. Drop image chains onto "
                           "the registration item or its Inputs folder before "
                           "running registration.");
      return;
   }

   if(dataManagerWidget())
   {
      std::shared_ptr<ossimJobQueue> q = dataManagerWidget()->jobQueue();
      if(q)
      {
         if(registration)
         {
            std::shared_ptr<RegistrationSourceJob> job =
               std::make_shared<RegistrationSourceJob>(
                  registration,
                  ossimString(text(0).toStdString()));
            job->setCallback(
               std::make_shared<RegistrationSourceJobCallback>(
                  dataManagerWidget(),
                  this));
            job->ready();
            q->add(job);
         }
         else if(bundleRegistration)
         {
            std::shared_ptr<BundleRegistrationSourceJob> job =
               std::make_shared<BundleRegistrationSourceJob>(
                  bundleRegistration,
                  ossimString(text(0).toStdString()));
            job->setCallback(
               std::make_shared<RegistrationSourceJobCallback>(
                  dataManagerWidget(),
                  this));
            job->ready();
            q->add(job);
         }
      }
      else
      {
         QMessageBox::warning(treeWidget(),
                              "Registration",
                              "No job queue is available for registration.");
      }
   }
#else
   QMessageBox::warning(treeWidget(),
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

ossimGui::DataManagerImageWriterFolder::DataManagerImageWriterFolder()
:DataManagerFolder()
{
   setText(0, "Writers");
}



ossimGui::DataManagerImageWriterItem::DataManagerImageWriterItem(DataManager::Node* node)
:DataManagerNodeItem(node)
{
   m_autoDelete = false;
}

ossimGui::DataManagerImageWriterItem::~DataManagerImageWriterItem()
{
}

void ossimGui::DataManagerImageWriterItem::execute()
{
   if(objectAsNode())
   {
      CopyChainVisitor visitor;
      
      objectAsNode()->getObject()->accept(visitor);
      ossimPropertyInterface* propInterface = objectAsNode()->getObjectAs<ossimPropertyInterface>();
      if(dataManagerWidget())
      {
         std::shared_ptr<ossimJobQueue> q = dataManagerWidget()->jobQueue();
         if(q)
         {
            ossimString defaultName = propInterface?propInterface->getPropertyValueAsString("filename"):ossimString(text(0).toStdString());
            std::shared_ptr<ossimJob> job = std::make_shared<ImageWriterJob>(visitor.kwl());
            job->setName("Output " + defaultName);
            visitor.reset();
            job->ready();
            q->add(job);
         }
      }
   }   
}

ossimGui::DataManagerImageFileWriterItem::DataManagerImageFileWriterItem(DataManager::Node* node)
:DataManagerImageWriterItem(node)
{
}

ossimGui::DataManagerImageFileWriterItem::~DataManagerImageFileWriterItem()
{
}


ossimGui::DataManagerImageFilterItem::DataManagerImageFilterItem()
{
   m_properties=0;
}

ossimGui::DataManagerImageFilterItem::~DataManagerImageFilterItem()
{
   m_object = 0;
}

void ossimGui::DataManagerImageFilterItem::setObject(ossimObject* obj)
{
   DataManagerItem::setObject(obj);
   if(m_object.valid())
   {
      setText(0, m_object->getClassName().c_str());
   }
}

ossimGui::DataManagerImageFilterFolder* ossimGui::DataManagerImageFilterItem::folder()
{
   return findParentItemAs<DataManagerImageFilterFolder>();
}

void ossimGui::DataManagerImageFilterItem::populateChildren()
{
   if(m_properties)
   {
      delete m_properties;
   }
   if(m_object.valid())
   {
      m_properties = new DataManagerPropertyFolder();
      m_properties->setObject(m_object.get());
      addChild(m_properties);
      m_properties->populateChildren();
   }
}

ossimGui::DataManagerImageFilterFolder::DataManagerImageFilterFolder()
{
   setText(0, "Filters");
}

void ossimGui::DataManagerImageFilterFolder::populateChildren()
{
   clearChildren();
   if(m_object.valid())
   {
      ossimImageChain* chain = dynamic_cast<ossimImageChain*> (m_object.get());
      if(chain)
      {
         ossimConnectableObject::ConnectableObjectList& chainList = chain->getChainList();
         ossim_uint32 nChildren =chainList.size();
         ossim_uint32 idx = 0;
         for(idx = 0; idx < nChildren;++idx)
         {
            DataManagerImageFilterItem* item = new DataManagerImageFilterItem();
            item->setObject(chainList[idx].get());
            insertChild(0,item);
            item->populateChildren();
        }
      }
   }
}

void ossimGui::DataManagerImageFilterFolder::setObject(ossimObject* obj)
{
   DataManagerFolder::setObject(obj);
   clearChildren();
}

#if 0
void ossimGui::DataManagerImageFilterFolder::printChain()
{
   std::cout << "_______________________________________" << std::endl;
   ossimKeywordlist kwl;
   if(object())
   {
      object()->saveState(kwl);
      std::cout << kwl << std::endl;
   }
   std::cout << "_______________________________________" << std::endl;
}
#endif

void ossimGui::DataManagerImageFilterFolder::removeFilters()
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   
   if(chain)
   {
      chain->deleteList();
      populateChildren();
     // printChain();
   }
}

void ossimGui::DataManagerImageFilterFolder::removeFilter(ossimObject* obj)
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   if(chain)
   {
      chain->removeChild(dynamic_cast<ossimConnectableObject*> (obj));
      populateChildren();
      //printChain();
   }
}



void ossimGui::DataManagerImageFilterFolder::addFilterToFront(ossimObject* obj)
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   
   if(chain)
   {
      ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*> (obj);
      if(connectable)
      {
         chain->addLast(connectable);
         populateChildren();
         //printChain();
      }
   }
}

void ossimGui::DataManagerImageFilterFolder::addFilterToEnd(ossimObject* obj)
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   
   if(chain)
   {
      ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*> (obj);
      if(connectable)
      {
         chain->addFirst(connectable);
         populateChildren();
        // printChain();
      }
   }
}

void ossimGui::DataManagerImageFilterFolder::insertFilterBefore(ossimObject* newObj, ossimObject* before)
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   
   if(chain)
   {
      chain->insertLeft(dynamic_cast<ossimConnectableObject*> (newObj), dynamic_cast<ossimConnectableObject*> (before));
      populateChildren();
      //printChain();
   }
}

void ossimGui::DataManagerImageFilterFolder::insertFilterAfter(ossimObject* newObj, ossimObject* after)
{
   ossimImageChain* chain = dynamic_cast<ossimImageChain*> (object());
   
   if(chain)
   {
      chain->insertRight(dynamic_cast<ossimConnectableObject*> (newObj), dynamic_cast<ossimConnectableObject*> (after));
      populateChildren();
      //printChain();
   }
}

void ossimGui::DataManagerNodeItemListener::disconnectInputEvent(ossimConnectionEvent& /* event */)
{
   if(!m_nodeItem) return;
   if(m_nodeItem->markedForDeletion()||!m_nodeItem->m_node.valid()) return;
   DataManagerWidgetEvent* e = new DataManagerWidgetEvent(DataManagerWidgetEvent::COMMAND_DISCONNECT_INPUT);
   e->setItemList(m_nodeItem);
   QCoreApplication::postEvent(m_nodeItem->treeWidget(), e);
}

void ossimGui::DataManagerNodeItemListener::connectInputEvent(ossimConnectionEvent& /* event */)
{
   
   if(!m_nodeItem) return;
   if(m_nodeItem->markedForDeletion()||!m_nodeItem->m_node.valid()) return;
   DataManagerWidgetEvent* e = new DataManagerWidgetEvent(DataManagerWidgetEvent::COMMAND_CONNECT_INPUT);
   e->setItemList(m_nodeItem);
   QCoreApplication::postEvent(m_nodeItem->treeWidget(), e);
}

ossimGui::DataManagerNodeItem::DataManagerNodeItem(DataManager::Node* node)
:DataManagerItem(), m_listener(new DataManagerNodeItemListener(this)),m_autoDelete(true)
{
   m_inputConnectionFolder = new DataManagerInputConnectionFolder();
   m_propertyFolder = new DataManagerPropertyFolder();
   setObject(node);
}

ossimGui::DataManagerNodeItem::~DataManagerNodeItem()
{
   setNodeListenerEnabled(false);
   if(m_listener) delete m_listener;
   m_listener = 0;
   if(m_inputConnectionFolder) delete m_inputConnectionFolder;
   m_inputConnectionFolder = 0;
   if(m_propertyFolder) delete m_propertyFolder;
   m_propertyFolder = 0;
   setObject(0);
}

void ossimGui::DataManagerNodeItem::setMarkForDeletion(bool flag)
{
   DataManagerItem::setMarkForDeletion(flag);
  
   setNodeListenerEnabled(!flag);
   std::lock_guard<std::mutex> lock(m_itemMutex);

   ossim_int32 nChildren = childCount();
   ossim_int32 idx = 0;
   for(idx = 0; idx < nChildren; ++idx)
   {
      DataManagerItem* item =dynamic_cast<DataManagerItem*> (child(idx));
      if(item)
      {
         item->setMarkForDeletion(flag);
      }
   }
}

void ossimGui::DataManagerNodeItem::setNodeListenerEnabled(bool flag)
{
   std::lock_guard<std::mutex> lock(m_itemMutex);
   if(flag)
   {
      if(m_node.valid())
      {
         ossimConnectableObject* obj = m_node->getObjectAs<ossimConnectableObject>();
         if(obj)
         {
          //  obj->addListener(static_cast<ossimConnectableObjectListener*>(this));
            obj->addListener(m_listener);
         }
      }
   }
   else 
   {
      if(m_node.valid())
      {
         ossimConnectableObject* obj = m_node->getObjectAs<ossimConnectableObject>();
         if(obj)
         {
            //obj->removeListener(static_cast<ossimConnectableObjectListener*>(this));
            obj->removeListener(m_listener);
         }
      }
   }
}

bool ossimGui::DataManagerNodeItem::autoDelete()const
{
   return m_autoDelete;
}

void ossimGui::DataManagerNodeItem::setAutoDelete(bool flag)
{
   m_autoDelete = flag;
}

bool ossimGui::DataManagerNodeItem::isCombiner()const
{
   return m_combinerFlag;
}

bool ossimGui::DataManagerNodeItem::isParentCombiner()const
{
   const DataManagerNodeItem* parentItem = dynamic_cast<const DataManagerNodeItem*> (parent());
   if(parentItem)
   {
      return parentItem->isCombiner();
   }
   return false;
}

void ossimGui::DataManagerNodeItem::getInputs(DataManager::NodeListType& result)
{
   if(m_node.valid())
   {
      ossimConnectableObject* connectable = m_node->getObjectAs<ossimConnectableObject>();
      if(connectable)
      {
        ossimConnectableObject::ConnectableObjectList& inputList = connectable->getInputList();
         if(!inputList.empty())
         {
            ossimRefPtr<DataManager> manager = dataManager();
            if(manager.valid())
            {
               ossim_uint32 idx = 0;
               for(idx = 0; idx < inputList.size();++idx)
               {
                  ossimRefPtr<DataManager::Node> node = manager->findNode(inputList[idx].get());
                  if(node.valid())
                  {
                     result.push_back(node.get());
                  }
               }
            }
         }
      }
   }
}

void ossimGui::DataManagerNodeItem::setObject(ossimObject* node)
{
   m_combinerFlag = false;
   setNodeListenerEnabled(false);
   
   m_node = dynamic_cast<DataManager::Node*>(node);
   DataManagerItem::setObject(m_node.get());
   if(m_node.valid())
   {
      setText(0, m_node->name());
   }
   
   setNodeListenerEnabled(true);
   if(m_node.valid())
   {
      ossimConnectableObject* obj = m_node->getObjectAs<ossimConnectableObject>();
      if(obj)
      {
         if(dynamic_cast<const ossimImageCombiner*>(obj))
         {
            m_combinerFlag = true;
         }
         else 
         {
            ossimImageChain* chain = dynamic_cast<ossimImageChain*>(obj);
            if(chain)
            {
               m_combinerFlag = dynamic_cast<ossimImageCombiner*>(chain->getLastSource()) != 0;
            }
         }
      }
   }
   initializePropertiesFromNode();
   populateChildren();
}

void ossimGui::DataManagerNodeItem::initializePropertiesFromNode()
{
   if(!m_node.valid()) return;
   ossimSource* source = m_node->getObjectAs<ossimSource>();
   if (!source) return;
   setCheckState(0, source->isSourceEnabled()?Qt::Checked:Qt::Unchecked);
   
}

void ossimGui::DataManagerNodeItem::populateChildren()
{
   refreshChildConnections();
   refreshChildProperties();
}

void ossimGui::DataManagerNodeItem::refreshChildConnections()
{
   ossimRefPtr<ossimConnectableObject> connectable = m_node.valid()?m_node->getObjectAs<ossimConnectableObject>():0;
   bool addConnections = false;
   if(connectable.valid())
   {
      if(!connectable->getInputListIsFixedFlag()||(connectable->getNumberOfInputs() > 0))
      {
         addConnections = true;
      }
   }
   if(addConnections)
   {
      if(indexOfChild(m_inputConnectionFolder) <0)
      {
         insertChild(0, m_inputConnectionFolder);
      }
      m_inputConnectionFolder->setObject(m_node.get());
   }
   else
   {
      takeChild(indexOfChild(m_inputConnectionFolder));
   }
   
   if(m_inputConnectionFolder)
   {
      m_inputConnectionFolder->populateChildren();
   }
}

void ossimGui::DataManagerNodeItem::refreshChildProperties()
{
   if(m_propertyFolder)
   {
      if(indexOfChild(m_propertyFolder) <0)
      {
         insertChild(0, m_propertyFolder);
      }
      if(objectAsNode())
      {
         m_propertyFolder->setObject(objectAsNode()->getObject());
      }
   }
}

void ossimGui::DataManagerNodeItem::reset()
{
   refreshChildProperties();
   refreshChildConnections();
}

ossimGui::DataManagerImageChainItem::DataManagerImageChainItem(DataManager::Node* node)
:DataManagerNodeItem(node)
{
   m_filters = 0;
}

void ossimGui::DataManagerImageChainItem::dropItems(
   QList<DataManagerImageChainItem*>& /* chainItemList */, 
   DataManagerNodeItem* /* targetItem */, 
   bool /* insertBefore */)
{
}

void ossimGui::DataManagerImageChainItem::populateChildren()
{
   DataManagerNodeItem::populateChildren();
   if(m_filters)
   {
      delete m_filters;
   }
   m_filters = 0;
   if(m_node.valid())
   {
      m_filters = new DataManagerImageFilterFolder();
      addChild(m_filters);
      m_filters->setObject(m_node->getObject());
      m_filters->populateChildren();
   }
}

void ossimGui::DataManagerImageChainItem::clearChildren()
{
}

void ossimGui::DataManagerRawImageSourceItem::reset()
{
   if(m_node.valid())
   {
      ossimImageHandler* handler = m_node->getObjectAs<ossimImageHandler>();
      if(handler)
      {
         if(handler->getState())
         {
            handler->getState()->setOverviewState(0);
         }
         handler->closeOverview();
         handler->openOverview();
         ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
         ossimEventVisitor visitor(refreshEvent.get());
         handler->accept(visitor);
      }
   }
   DataManagerNodeItem::reset();
}

void ossimGui::DataManagerFolder::setMarkForDeletion(bool flag)
{
   std::lock_guard<std::mutex> lock(m_itemMutex);
   
   ossim_int32 nChildren = childCount();
   ossim_int32 idx = 0;
   for(idx = 0; idx < nChildren; ++idx)
   {
      DataManagerItem* item =dynamic_cast<DataManagerItem*> (child(idx));
      if(item)
      {
         item->setMarkForDeletion(flag);
      }
   }
}

void ossimGui::DataManagerFolder::setNodeListenerEnabled(bool flag)
{
   std::lock_guard<std::mutex> lock(m_itemMutex);
   ossim_int32 nChildren = childCount();
   ossim_int32 idx = 0;
   for(idx = 0; idx < nChildren; ++idx)
   {
      DataManagerItem* item =dynamic_cast<DataManagerItem*> (child(idx));
      if(item)
      {
         item->setNodeListenerEnabled(flag);
      }
   }
}

ossimGui::DataManagerPropertyFolder::DataManagerPropertyFolder()
:DataManagerFolder()
{
   setText(0, "Properties");
   setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
}

ossimGui::DataManagerPropertyFolder::~DataManagerPropertyFolder()
{
   m_object = 0;
}

void ossimGui::DataManagerPropertyFolder::populateChildren()
{
   if(childCount() > 0)
   {
      clearChildren();
   }
   if(m_object.valid())
   {
      ossimPropertyInterface* propertyInterface = dynamic_cast<ossimPropertyInterface*> (m_object.get());
      if(propertyInterface)
      {
         std::vector<ossimRefPtr<ossimProperty> > currentProperties;
         propertyInterface->getPropertyList(currentProperties);
         
         // ossim_uint32 propertyIdx = 0;
         DataManagerPropertyView* w = new DataManagerPropertyView(treeWidget());
         w->blockSignals(true);
         w->setObject(m_object.get());
         DataManagerPropertyItem* propertyItem = new DataManagerPropertyItem();
         propertyItem->setFlags(propertyItem->flags() & (~Qt::ItemIsSelectable));
         addChild(propertyItem);
         w->resizeColumnToContents(0);
         dataManagerWidget()->setItemWidget(propertyItem, 0, w);
         w->blockSignals(false);
      }
   }
}


ossimGui::DataManagerPropertyFolder* ossimGui::DataManagerPropertyItem::propertyFolder()
{
   return findParentItemAs<DataManagerPropertyFolder>();
}

void ossimGui::DataManagerPropertyItem::setProperty(ossimProperty* prop)
{
   m_property = prop;
   setText(0, prop->getName().c_str());
   //setText(1, prop->valueToString().c_str());
   if(!prop->isReadOnly())
   {
      setFlags(flags()|Qt::ItemIsEditable);
   }
}

ossimGui::DataManagerJobItem::DataManagerJobItem()
:m_job(0)
{
   m_progressItem = new QTreeWidgetItem();
   m_progressBar = new QProgressBar();
   m_progressBar->setMinimum(0);
   m_progressBar->setMaximum(100);
   m_progressBar->reset();
   m_progressBar->setTextVisible(true);
   setExpanded(true);
}

ossimGui::DataManagerJobItem::~DataManagerJobItem()
{
   if(m_job)
   {
      m_job->setCallback(m_jobCallback->callback());
   }
}

void ossimGui::DataManagerJobItem::setJob(std::shared_ptr<ossimJob> job)
{
   if(m_job&&m_jobCallback)
   {
      m_job->setCallback(m_jobCallback->callback());
   }
   m_job = job;
   if(m_job)
   {
      m_jobCallback = std::make_shared<JobCallback>(this, m_job->callback());
      m_job->setCallback(m_jobCallback);
   }
   if(dataManagerWidget())
   {
      addChild(m_progressItem);
      setExpanded(true);
      dataManagerWidget()->setItemWidget(m_progressItem, 0, m_progressBar);
   }
   if(job->isStopped()) m_jobCallback->finished(job);
}

void ossimGui::DataManagerJobItem::cancel()
{
   if(m_job)
   {
      m_job->cancel();
      
      DataManagerJobsFolder* folder = findParentItemAs<DataManagerJobsFolder>();
      if(folder)
      {
         folder->removeStoppedJobs();
      }
   }
}

void ossimGui::DataManagerJobItem::JobCallback::ready(std::shared_ptr<ossimJob> job)
{
   DataManagerJobsFolder* folder = m_jobItem->findParentItemAs<DataManagerJobsFolder>();
   if(folder)
   {
      DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_STATE_CHANGED);
      e->setJobList(job);
      QCoreApplication::postEvent(folder->treeWidget(), e);
   }
   ossimJobCallback::ready(job);
}

void ossimGui::DataManagerJobItem::JobCallback::started(std::shared_ptr<ossimJob> job)
{
   DataManagerJobsFolder* folder = m_jobItem->findParentItemAs<DataManagerJobsFolder>();
   if(folder)
   {
      DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_STATE_CHANGED);
      e->setJobList(job);
      QCoreApplication::postEvent(folder->treeWidget(), e);
   }
   ossimJobCallback::started(job);
}

void ossimGui::DataManagerJobItem::JobCallback::finished(std::shared_ptr<ossimJob> job)
{
   DataManagerJobsFolder* folder = m_jobItem->findParentItemAs<DataManagerJobsFolder>();
   if(folder)
   {
      DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_STATE_CHANGED);
      e->setJobList(job);
      QCoreApplication::postEvent(folder->treeWidget(), e);
   }
   ossimJobCallback::finished(job);
}

void ossimGui::DataManagerJobItem::JobCallback::canceled(std::shared_ptr<ossimJob> job)
{
   DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_STATE_CHANGED);
   e->setJobList(job);
   QCoreApplication::postEvent(m_jobItem->treeWidget(), e);
   ossimJobCallback::canceled(job);
}

void ossimGui::DataManagerJobItem::JobCallback::nameChanged(const ossimString& name, std::shared_ptr<ossimJob> job)
{
   DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_PROPERTY_CHANGED);
   e->setJobList(job);
   QCoreApplication::postEvent(m_jobItem->treeWidget(), e);
   
   ossimJobCallback::nameChanged(name, job);
}

void ossimGui::DataManagerJobItem::JobCallback::descriptionChanged(const ossimString& description, std::shared_ptr<ossimJob> job)
{
   DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_PROPERTY_CHANGED);
   e->setJobList(job);
   QCoreApplication::postEvent(m_jobItem->treeWidget(), e);
   
   ossimJobCallback::descriptionChanged(description, job);
}

void ossimGui::DataManagerJobItem::JobCallback::idChanged(const ossimString& id, std::shared_ptr<ossimJob> job)
{
   DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_PROPERTY_CHANGED);
   e->setJobList(job);
   QCoreApplication::postEvent(m_jobItem->treeWidget(), e);
   
   ossimJobCallback::idChanged(id, job);
}

void ossimGui::DataManagerJobItem::JobCallback::percentCompleteChanged(double percentValue, std::shared_ptr<ossimJob> job)
{
   ossimJobCallback::percentCompleteChanged(percentValue, job);
   DataManagerWidgetJobEvent* e = new DataManagerWidgetJobEvent(DataManagerWidgetJobEvent::COMMAND_JOB_PERCENT_COMPLETE);
   e->setJobList(percentValue, job);
   
   QCoreApplication::postEvent(m_jobItem->treeWidget(), e);
}


ossimGui::DataManagerJobsFolder::DataManagerJobsFolder()
:DataManagerFolder()
{
   setExpanded(true);
   setText(0, "Jobs");
   m_jobQueueCallback = std::make_shared<JobQueueCallback>(this);
}
ossimGui::DataManagerJobsFolder::DataManagerJobsFolder(QTreeWidget* parent)
:DataManagerFolder(parent)
{
   setExpanded(true);
   setText(0, "Jobs");
   m_jobQueueCallback = std::make_shared<JobQueueCallback>(this);
}
ossimGui::DataManagerJobsFolder::DataManagerJobsFolder(QTreeWidgetItem* parent)
:DataManagerFolder(parent)
{
   setExpanded(true);
   setText(0, "Jobs");
   m_jobQueueCallback = std::make_shared<JobQueueCallback>(this);
}

ossimGui::DataManagerJobsFolder::~DataManagerJobsFolder()
{
}

void ossimGui::DataManagerJobsFolder::removeStoppedJobs()
{
   m_jobsFolderMutex.lock();
   QueueListType::iterator iter = m_queues.begin();
   while(iter != m_queues.end())
   {
      (*iter)->removeStoppedJobs();
      ++iter;
   }
   m_jobsFolderMutex.unlock();
}

void ossimGui::DataManagerJobsFolder::setQueue(std::shared_ptr<ossimJobQueue> q)
{
   std::lock_guard<std::mutex> lock(m_jobsFolderMutex);
   m_queues.clear();
   m_queues.push_back(q);
   q->setCallback(m_jobQueueCallback);
}


ossimGui::DataManagerWidget::DataManagerCallback::DataManagerCallback(DataManagerWidget* widget)
:m_dataManagerWidget(widget)
{
}

void ossimGui::DataManagerWidget::DataManagerCallback::nodesRemoved(DataManager::NodeListType& nodes)
{
   QTreeWidgetItemIterator iter(m_dataManagerWidget);
   
   // see if we are removing the planetary node
   //
   if(std::find(nodes.begin(), nodes.end(), m_dataManagerWidget->m_planetaryDisplayNode.get()) != nodes.end())
   {
      m_dataManagerWidget->m_planetaryDisplayNode = 0;
   }
   while(*iter)
   {
      DataManagerItem* item = dynamic_cast<DataManagerItem*> (*iter);
      if(item)
      {
         if(std::find(nodes.begin(), nodes.end(), item->objectAsNode()) != nodes.end())
         {
            if(item->parent()) item->parent()->removeChild(item);
            item->setMarkForDeletion(true);
            if(item->objectAsNode()&&item->objectAsNode()->getObjectAsConnectable()) item->objectAsNode()->getObjectAsConnectable()->disconnect();
         }
      }
      ++iter;
   }
   
   DataManagerEvent* e = new DataManagerEvent(DataManagerEvent::COMMAND_NODE_REMOVED);
   e->setNodeList(nodes);
   QCoreApplication::postEvent(m_dataManagerWidget, e);
}

void ossimGui::DataManagerWidget::DataManagerCallback::nodesAdded(DataManager::NodeListType& nodes)
{
   DataManagerEvent* e = new DataManagerEvent(DataManagerEvent::COMMAND_NODE_ADDED);
   e->setNodeList(nodes);
   QCoreApplication::postEvent(m_dataManagerWidget, e);
}

ossimGui::DataManagerWidget::DataManagerWidget(QWidget* parent)
   : QTreeWidget(parent),
     m_dataManager( new DataManager() ),
     m_dataManagerCallback( std::make_shared<DataManagerCallback>(this) ),
     m_jobQueue(0),
     m_displayQueue(std::make_shared<DisplayTimerJobQueue>()),
     m_rootImageFolder(0),
     m_rootJobsFolder(0),
     m_rawImageSources(0),
     m_imageChains(0),
     m_imageDisplays(0),
     m_registrationSources(0),
     m_imageWriters(0),
     m_dragStartPosition(),
     m_activeItems(),
     m_activeItemsMutex(),
     m_miDialog(0),
     m_amDialog(0),
     m_tGen(0),
     m_tGenObj(0),
     m_amDialogAvailable(false),
     m_amDialogActive(false),
     m_filterList(),
     m_combinerList(),
     m_planetaryDisplayNode(),
     m_lastOpenedDirectory()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   ensureRegistrationSourceFactoryRegistered();
#endif

   setAcceptDrops(true);
   
   m_dataManager->setCallback(m_dataManagerCallback);
   
   initialize();
   //connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*, int)));
   connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChanged(QTreeWidgetItem*, int)));
   connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));
   connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));

}

bool ossimGui::DataManagerWidget::openDataManager(const ossimFilename& file)
{
   bool result = false;
   ossimKeywordlist kwl;
   if(kwl.addFile(file))
   {
      // If this is an OMAR project export, the project file will not have absolute
      // paths, so update the kwl with project file path.  This is based on the project
      // file being at the same level as the folder containing the data, the
      // structure resulting from unzipping a zip file exported from OMAR.
      std::vector<ossimString> fn = kwl.findAllKeysThatContains("filename");
 
      ossimFilename firstFilename(kwl[fn[0]]);

      // Check for relative path and make corrections
      if (firstFilename.isRelative())
      {
         ossim_uint32 idx;

         for (idx=0; idx<fn.size(); ++idx)
         {
            ossimFilename newFn = file.path().dirCat(kwl[fn[idx]]);
            kwl.add(fn[idx], newFn.data(), true);
         }
         std::vector<ossimString> ov = kwl.findAllKeysThatContains("overview_file");
         for (idx=0; idx<ov.size(); ++idx)
         {
            ossimFilename newOv = file.path().dirCat(kwl[ov[idx]]);
            kwl.add(ov[idx], newOv.data(), true);
         }
         std::vector<ossimString> nm = kwl.findAllKeysThatContains(".name");
         for (idx=0; idx<nm.size(); ++idx)
         {
            ossimString newNm = " Entry 0: " + kwl[fn[idx]];
            kwl.add(nm[idx], newNm.data(), true);
         }
      }

      m_dataManagerCallback->setEnabled(false);
      m_rootImageFolder->setMarkForDeletion(true);
      m_dataManager->clear();
      result =m_dataManager->loadState(kwl,"dataManager.");
      m_activeItemsMutex.lock();
      m_activeItems.clear();
      m_activeItemsMutex.unlock();
      refresh();
      m_dataManagerCallback->setEnabled(true);
   }

   return result;
}

void ossimGui::DataManagerWidget::refresh()
{
   QTreeWidgetItemIterator iter1(m_rawImageSources);
   m_rootImageFolder->setMarkForDeletion(true);
   m_rawImageSources->clearChildren();
   m_imageChains->clearChildren();
   m_imageDisplays->clearChildren();
   m_registrationSources->clearChildren();
   m_imageWriters->clearChildren();
   RefreshVisitor visitor(this);
   
   if(m_dataManager.valid())
   {
      m_dataManager->accept(visitor);
   }
}

void ossimGui::DataManagerWidget::initialize()
{
   clear();
   setColumnCount(1);
   if(!headerItem())
   {
      setHeaderItem(new QTreeWidgetItem());
   }
   
   headerItem()->setText(0, "Data Manager");
   
   m_rootImageFolder   = new DataManagerImageFolder((QTreeWidget*)this);
   addTopLevelItem(m_rootImageFolder);
   
   m_rootJobsFolder = new DataManagerJobsFolder((QTreeWidget*)this);
   addTopLevelItem(m_rootJobsFolder);
   
   m_rawImageSources   = new DataManagerRawImageSourceFolder();
   m_imageChains       = new DataManagerImageChainFolder();
   m_imageDisplays = new DataManagerDisplayFolder();
   m_registrationSources = new DataManagerRegistrationFolder();
   m_imageWriters  = new DataManagerImageWriterFolder();
   m_rootImageFolder->setText(0, "Image Folder");
   m_rawImageSources->setText(0, "Sources");
   m_imageChains->setText(0, "Chains");
   m_rootImageFolder->setExpanded(true);
   
   m_rootImageFolder->addChild(m_rawImageSources);
   m_rootImageFolder->addChild(m_imageChains);
   m_rootImageFolder->addChild(m_imageDisplays);
   m_rootImageFolder->addChild(m_registrationSources);
   m_rootImageFolder->addChild(m_imageWriters);
   
   std::vector<ossimString> objects;
   ossimImageSourceFactoryRegistry::instance()->getTypeNameList(objects);
   
   ossim_uint32 idx = 0;
   m_filterList.clear();
   m_combinerList.clear();

   for(idx = 0; idx < objects.size(); ++idx)
   {

      //ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(objects[idx]);
      ossimRefPtr<ossimObject> obj = ossimImageSourceFactoryRegistry::instance()->createObject(objects[idx]);
      if(dynamic_cast<ossimImageSourceFilter*>(obj.get()))
      {
         m_filterList.push_back(objects[idx].c_str());
      }
      else if(dynamic_cast<ossimImageCombiner*>(obj.get()))
      {
         m_combinerList.push_back(objects[idx].c_str());
      }
   }
}


QMainWindow* ossimGui::DataManagerWidget::mainWindow()
{
   QMainWindow* result = 0;
   QWidget* parent = parentWidget();
   
   while(parent&&!result)
   {
      result = dynamic_cast<QMainWindow*> (parent);
      parent = parent->parentWidget();
   }
   
   return result;
}

void ossimGui::DataManagerWidget::incrementScrollBars(const QPoint& pos)
{
   int autoScrollMargin = 10;
   QRect r = contentsRect();
   int hAdjusted = r.height()-autoScrollMargin-(header()?header()->size().height():0);
   QScrollBar* verticalbar   = verticalScrollBar();
   // QScrollBar* horizontalBar = horizontalScrollBar();
   int verticalAdjustment = 0;
   // int horizontalAdjustment = 0;
   
   if(pos.y() > hAdjusted)
   {
      verticalAdjustment = (pos.y()-hAdjusted);
   }
   else if(pos.y() < autoScrollMargin)
   {
      verticalAdjustment = pos.y() - autoScrollMargin;
   }
   
   if(verticalbar) verticalbar->setValue(verticalbar->value() + verticalAdjustment);
}


void	ossimGui::DataManagerWidget::dragEnterEvent ( QDragEnterEvent * event )
{
   event->acceptProposedAction();
}

void	ossimGui::DataManagerWidget::dragLeaveEvent ( QDragLeaveEvent * /* event */ )
{
}

void	ossimGui::DataManagerWidget::dragMoveEvent ( QDragMoveEvent * e )
{
   incrementScrollBars(e->pos());
}

void	ossimGui::DataManagerWidget::dropEvent ( QDropEvent * e )
{
   DataManagerItem* targetItem                = dynamic_cast<DataManagerItem*>(itemAt(e->pos()));
   DataManagerImageChainItem* targetChainItem = dynamic_cast<DataManagerImageChainItem*>(targetItem);
   if(targetItem)
   {
      targetItem->setSelected(false); // make sure it is unselected for we will use current selections for the drop
      if(dynamic_cast<DataManagerImageChainFolder*> (targetItem))
      {
         QList<DataManagerRawImageSourceItem*> result = grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
         if(!result.empty())
         {
            QList<DataManagerRawImageSourceItem*>::iterator iter = result.begin();
            while(iter != result.end())
            {
               m_dataManager->createDefaultImageChain((*iter)->objectAsNode());
               ++iter;
            }
         }
      }
      else if(targetChainItem)
      {
         DataManagerImageChainItem* targetParentChainItem = targetChainItem->parentAs<DataManagerImageChainItem>();
         QList<DataManagerImageChainItem*> selectedItems = grabSelectedChildItemsOfType<DataManagerImageChainItem>();
         
        if(targetChainItem->isParentCombiner()&&targetParentChainItem)
         {
            targetParentChainItem->dropItems(selectedItems, targetChainItem);
         }
         else if(targetChainItem->isCombiner())
         {
            targetChainItem->dropItems(selectedItems);
         }
      }
      else if(dynamic_cast<DataManagerRegistrationItem*>(targetItem))
      {
         QList<DataManagerItem*> itemList =
            grabSelectedChildItemsOfType<DataManagerItem>();
         dynamic_cast<DataManagerRegistrationItem*>(targetItem)->
            dropItems(itemList);
      }
      else
      {
         DataManagerInputConnectionFolder* targetConnectionFolder = dynamic_cast<DataManagerInputConnectionFolder*>(itemAt(e->pos()));
         DataManagerInputConnectionItem*   targetConnectionItem   = dynamic_cast<DataManagerInputConnectionItem*>(itemAt(e->pos()));
         if(!targetConnectionFolder)
         {
            if(targetConnectionItem)
            {
               targetConnectionFolder = targetConnectionItem->parentItemAs<DataManagerInputConnectionFolder>();
            }
         }
         if(targetConnectionFolder)
         {
            QList<DataManagerItem*>     itemList = grabSelectedChildItemsOfType<DataManagerItem>();
            
            targetConnectionFolder->connect(itemList, targetConnectionItem);
         }
      }
   }
}


void ossimGui::DataManagerWidget::deleteSelected()
{
   if(QMessageBox::question(this, "Delete Items", "Do you wish to delete the currently\nselected items?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
   {
      deleteSelectedItems();
   }
}

void ossimGui::DataManagerWidget::deleteSelectedItems()
{  
   // these are not managed and are shared links so we can just delete them
   QList<DataManagerInputConnectionItem*> connectionItems = grabSelectedChildItemsOfType<DataManagerInputConnectionItem>();
   if(!connectionItems.empty())
   {
      std::set<DataManagerInputConnectionFolder*> folderSet;
      
      QList<DataManagerInputConnectionItem*>::iterator iter = connectionItems.begin();
      while(iter!=connectionItems.end())
      {
         DataManagerInputConnectionFolder* folder =(*iter)->parentItemAs<DataManagerInputConnectionFolder>();
         if(folder)
         {
            folderSet.insert(folder);
         }
         ++iter;
      }
      std::set<DataManagerInputConnectionFolder*>::iterator folderSetIterator = folderSet.begin();
      while(folderSetIterator != folderSet.end())
      {
         (*folderSetIterator)->disconnectSelected();
         ++folderSetIterator;
      }
   }
   
   QList<DataManagerNodeItem*> result = grabSelectedChildItemsOfType<DataManagerNodeItem>();
   DataManager::NodeListType itemsToDelete;
   if(!result.empty())
   {
      QList<DataManagerNodeItem*>::iterator iter = result.begin();
      
      while(iter != result.end())
      {
         DataManager::Node* node = (*iter)->objectAsNode();
         ConnectableDisplayObject* obj = node->getObjectAs<ConnectableDisplayObject>();
         
         if(obj&&obj->display()&&!obj->display()->testAttribute(Qt::WA_DeleteOnClose))
         {
            // display is not deleted on close.
            obj->display()->close();
         }
         else
         {
            (*iter)->setMarkForDeletion(true);
            (*iter)->setSelected(false);
            if((*iter)->objectAsNode())
            {
               DataManager::Node* node = (*iter)->objectAsNode();
               if(node&&node->getObjectAsConnectable()) 
               {
                  node->getObjectAsConnectable()->disconnect();
               }
               itemsToDelete.push_back((*iter)->objectAsNode());
            }
            if(m_activeItems.erase((*iter))>0)
            {
               delete (*iter);
            }
         }
         ++iter;
      }
   }
   
   m_dataManager->remove(itemsToDelete, false);
   itemsToDelete.clear();
}

void ossimGui::DataManagerWidget::cancelSelected()
{
   if(QMessageBox::question(this, "Cancel Jobs", "Do you wish to cancel the currently\nselected jobs?", QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
   {
      QList<DataManagerJobItem*> jobItems = grabSelectedChildItemsOfType<DataManagerJobItem>();
      QList<DataManagerJobItem*>::iterator iter = jobItems.begin();
      while(iter != jobItems.end())
      {
         (*iter)->cancel();
         ++iter;
      }
   }
}

void ossimGui::DataManagerWidget::keyPressEvent ( QKeyEvent * e)
{
   switch(e->key())
   {
      case Qt::Key_Delete:
      {
         deleteSelected();
         break;
      }

      case Qt::Key_S:
      {
         if(m_miDialog) m_miDialog->show();
      }
   }
}

void ossimGui::DataManagerWidget::mousePressEvent(QMouseEvent *e)
{
   QTreeWidget::mousePressEvent(e);
   QTreeWidgetItem* itemSelected = itemAt(e->pos());
   bool dropDownMenuFlag = (e->buttons() & Qt::RightButton) ||
                          ((e->buttons() & Qt::LeftButton) &&
                           (e->modifiers()&Qt::MetaModifier));
   if(itemSelected) itemSelected->setSelected(true);
   if(dropDownMenuFlag)
   {
      QList<DataManagerItem*> items = grabSelectedChildItemsOfType<DataManagerItem>();
      if(!items.empty())
      {
         QMenu* menu = createMenu(items, dynamic_cast<DataManagerItem*>(itemSelected));
         if(menu)
         {
            menu->popup(mapToGlobal(e->pos()), menu->menuAction());
         }
      }
   }
   else if ((e->buttons() & Qt::LeftButton)
       && itemSelected) 
   {
      m_dragStartPosition = e->pos();
   }
   
}

void ossimGui::DataManagerWidget::mouseMoveEvent(QMouseEvent *e)
{
   if (e->buttons() == Qt::LeftButton)
   {
      // QPoint pos = e->pos();
      // ossim_int32 len = (pos - m_dragStartPosition).manhattanLength();
      if ((e->pos() - m_dragStartPosition).manhattanLength()
          >= QApplication::startDragDistance())
      {
         QDrag *drag = new QDrag(this);
         QMimeData *mimeData = new QMimeData;
      
         //mimeData->setText(commentEdit->toPlainText());
         drag->setMimeData(mimeData);
         //drag->setPixmap(iconPixmap);
      
         // Qt::DropAction dropAction = drag->exec();
         drag->exec();
      }
   }
}

void ossimGui::DataManagerWidget::mouseReleaseEvent(QMouseEvent * e)
{
   QTreeWidget::mouseReleaseEvent(e);
}

void ossimGui::DataManagerWidget::showSelected()
{
   QList<DataManagerItem*> items= grabSelectedChildItemsOfType<DataManagerItem>();
   
   QList<DataManagerItem*>::iterator itemsIter = items.begin();
   DataManagerEvent* event = new DataManagerEvent();
   while(itemsIter != items.end())
   {
      event->nodeList().push_back((*itemsIter)->objectAsNode());
      
      ++itemsIter;
   }
   QCoreApplication::postEvent(mainWindow(), event);
}

void ossimGui::DataManagerWidget::exportSelected()
{
   QList<DataManagerImageChainItem*> chainItems= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   QList<DataManagerRawImageSourceItem*> rawItems= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   
   ossim_uint32 count = chainItems.size()+rawItems.size();
   if(count>1)
   {
      QMessageBox::warning(this, "Warning", "Please select only one item to export.  This can be a chain or a raw source.");
      return;
   }
   // need support to output the 8 bit chain layer.  
   // we will do that later
   //
   ossimRefPtr<DataManager::Node> node;
   
   if(!chainItems.empty())
   {
      node = (*chainItems.begin())->objectAsNode();
   }
   else if(!rawItems.empty())
   {
      node = (*rawItems.begin())->objectAsNode();
   }

   if(node.valid())
   {
      ossimConnectableObject* obj = node->getObjectAs<ossimConnectableObject>();
      if(obj)
      {
         ExportImageDialog* dialog = new ExportImageDialog(this);
         dialog->setObject(obj);
         dialog->exec();
      }
   }
}

void ossimGui::DataManagerWidget::swipeSelected()
{
   QList<DataManagerItem*> items= grabSelectedChildItemsOfType<DataManagerItem>();
   
   QList<DataManagerItem*>::iterator itemsIter = items.begin();
   ossimRefPtr<ossimGui::DataManager::Node> node =  m_dataManager->createDefault2dImageDisplay(0, true);
   ConnectableDisplayObject* connectableDisplay = node->getObjectAs<ConnectableDisplayObject>();
   
   while(itemsIter != items.end())
   {
      DataManager::Node* iterNode=(*itemsIter)->objectAsNode();
      if(iterNode)
      {
         if(iterNode->getObjectAs<ossimImageSource>())
         {
            connectableDisplay->connectMyInputTo(iterNode->getObjectAs<ossimConnectableObject>());
         }
      }
      ++itemsIter;
   }   
   DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
   event->setNodeList(node.get());
   QCoreApplication::postEvent(mainWindow(), event);
   
}

void ossimGui::DataManagerWidget::buildOverviewsForSelected(QAction* action)
{
   buildOverviewsForSelected(action->text());
}

void ossimGui::DataManagerWidget::buildOverviewsForSelected(const QString& type)
{
   QList<DataManagerRawImageSourceItem*> items= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   if(!items.empty())
   {
      QList<DataManagerRawImageSourceItem*>::iterator item = items.begin();
      while(item!=items.end())
      {
         DataManager::Node* node = (*item)->objectAsNode();
         if(node)
         {
            ossimRefPtr<ossimImageHandler> isource = node->getObjectAs<ossimImageHandler>();
            if(isource.valid())
            {
               std::shared_ptr<ImageStagerJob> stagerJob = std::make_shared<ImageStagerJob>(isource.get(), ImageStagerJob::STAGE_OVERVIEWS);
               stagerJob->setCallback(std::make_shared<ImageStagerJobCallback>(this, (*item)));
               stagerJob->setOverviewType(type.toStdString());
               m_jobQueue->add(stagerJob);
               stagerJob = 0; isource = 0;
            }
         }
         ++item;
      }
   }
}

void ossimGui::DataManagerWidget::buildOverviewsForSelected()
{
   buildOverviewsForSelected(QString("ossim_tiff_box"));
}

void ossimGui::DataManagerWidget::createFullHistogramsForSelected()
{
}

void ossimGui::DataManagerWidget::createFastHistogramsForSelected()
{
}


void ossimGui::DataManagerWidget::createDefaultChain()
{
   QList<DataManagerRawImageSourceItem*> items= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   DataManager::NodeListType nodeList;
   DataManager::NodeListType displayList;
   QList<DataManagerRawImageSourceItem*>::iterator item = items.begin();
   while(item != items.end())
   {
      DataManager::Node* node = (*item)->objectAsNode();
      ossimRefPtr<DataManager::Node> newNode = m_dataManager->createDefaultImageChain(node, false).get();
   
      if(newNode.valid())
      {
         nodeList.push_back(newNode.get());
         ossimRefPtr<DataManager::Node> newDisplayNode = m_dataManager->createDefault2dImageDisplay(newNode.get(), false);
         nodeList.push_back(newDisplayNode.get());
         displayList.push_back(newDisplayNode.get());
      }
      ++item;
   }
   populateTreeWithNodes(nodeList);
   if(!displayList.empty())
   {
      DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
      event->setNodeList(displayList);
      QCoreApplication::postEvent(mainWindow(), event);
   }
   
}

void ossimGui::DataManagerWidget::createAffineChain()
{
   QList<DataManagerRawImageSourceItem*> items= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   DataManager::NodeListType nodeList;
   DataManager::NodeListType displayList;
   QList<DataManagerRawImageSourceItem*>::iterator item = items.begin();
   ossimKeywordlist kwl;
   if(kwl.parseString(m_dataManager->defaultAffineChain()))
   {
      while(item != items.end())
      {
         DataManager::Node* node = (*item)->objectAsNode();
         ossimRefPtr<DataManager::Node> newNode = m_dataManager->createChainFromTemplate(kwl, node, false).get();
         
         if(newNode.valid())
         {
            newNode->setName( "Image Chain: " + newNode->name());
            nodeList.push_back(newNode.get());
            ossimRefPtr<DataManager::Node> newDisplayNode = m_dataManager->createDefault2dImageDisplay(newNode.get(), false);
            nodeList.push_back(newDisplayNode.get());
            displayList.push_back(newDisplayNode.get());
         }
         ++item;
      }
      populateTreeWithNodes(nodeList);
   }
   if(!displayList.empty())
   {
      DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
      event->setNodeList(displayList);
      QCoreApplication::postEvent(mainWindow(), event);
   }
}

void ossimGui::DataManagerWidget::createMapProjectedChain()
{
   QList<DataManagerRawImageSourceItem*> items= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   DataManager::NodeListType nodeList;
   DataManager::NodeListType displayList;
   QList<DataManagerRawImageSourceItem*>::iterator item = items.begin();
   ossimKeywordlist kwl;
   if(kwl.parseString(m_dataManager->defaultReprojectionChain()))
   {
      while(item != items.end())
      {
         DataManager::Node* node = (*item)->objectAsNode();
         ossimRefPtr<DataManager::Node> newNode = m_dataManager->createChainFromTemplate(kwl, node, false).get();
         
         if(newNode.valid())
         {
            newNode->setName( "Reprojection Chain: " + newNode->name());
            nodeList.push_back(newNode.get());
            ossimRefPtr<DataManager::Node> newDisplayNode = m_dataManager->createDefault2dImageDisplay(newNode.get(), false);
            nodeList.push_back(newDisplayNode.get());
            displayList.push_back(newDisplayNode.get());
         }
         ++item;
      }
      populateTreeWithNodes(nodeList);
   }
   if(!displayList.empty())
   {
      DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
      event->setNodeList(displayList);
      QCoreApplication::postEvent(mainWindow(), event);
   }
}

void ossimGui::DataManagerWidget::createImageNormalsChain()
{
   QList<DataManagerRawImageSourceItem*> items= grabSelectedChildItemsOfType<DataManagerRawImageSourceItem>();
   ossimString templateChain = "type: ossimImageChain\n";
   templateChain += "object0.type: ossimImageToPlaneNormalFilter\n";
   templateChain += "object1.type: ossimCacheTileSource\n";
   templateChain += "object2.type: ossimImageRenderer\n";
   templateChain += "object3.type: ossimCacheTileSource\n";
   
   DataManager::NodeListType nodeList;
   DataManager::NodeListType displayList;
   QList<DataManagerRawImageSourceItem*>::iterator item = items.begin();
   ossimKeywordlist kwl;
   
   if(kwl.parseString(templateChain))
   {
      while(item != items.end())
      {
         DataManager::Node* node = (*item)->objectAsNode();
         ossimRefPtr<DataManager::Node> newNode = m_dataManager->createChainFromTemplate(kwl, node, false).get();
         
         if(newNode.valid())
         {
            newNode->setName( "Normals Chain: " + newNode->name());
            nodeList.push_back(newNode.get());
            ossimRefPtr<DataManager::Node> newDisplayNode = m_dataManager->createDefault2dImageDisplay(newNode.get(), false);
            nodeList.push_back(newDisplayNode.get());
            displayList.push_back(newDisplayNode.get());
         }
         ++item;
      }
      populateTreeWithNodes(nodeList);
   }
   if(!displayList.empty())
   {
      DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
      event->setNodeList(displayList);
      QCoreApplication::postEvent(mainWindow(), event);
   }
}

void ossimGui::DataManagerWidget::registrationExploitationSelected()
{
   m_dataManager->setExploitationMode(DataManager::REGISTRATION_MODE);
   this->miDialog(DataManager::REGISTRATION_MODE);
}

void ossimGui::DataManagerWidget::geoPositioningExploitationSelected()
{
   m_dataManager->setExploitationMode(DataManager::GEOPOSITIONING_MODE);
   this->miDialog(DataManager::GEOPOSITIONING_MODE);
}

void ossimGui::DataManagerWidget::mensurationExploitationSelected()
{
   m_dataManager->setExploitationMode(DataManager::MENSURATION_MODE);
   this->miDialog(DataManager::MENSURATION_MODE);
}

void ossimGui::DataManagerWidget::aOverBMosaic()
{
   combineImagesWithType("ossimImageMosaic");
}

void ossimGui::DataManagerWidget::blendMosaic()
{
   combineImagesWithType("ossimBlendMosaic");
}

void ossimGui::DataManagerWidget::featherMosaic()
{
   combineImagesWithType("ossimFeatherMosaic");
}

void ossimGui::DataManagerWidget::hillShadeCombiner()
{
   QList<DataManagerImageChainItem*> items= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   
   DataManager::NodeListType newNodeList;
   DataManager::NodeListType displayList;
   DataManager::NodeListType nodeToConnectList;
   DataManagerImageChainItem* normalItem = 0;
   DataManagerImageChainItem* colorItem  = 0;
   if(items.size() == 1)
   {
      normalItem = *items.begin();
      
      if(normalItem->objectAsNode())
      {
         ossimImageSource* source = normalItem->objectAsNode()->getObjectAs<ossimImageSource>();
         if(source)
         {
            if((source->getNumberOfOutputBands() !=3)||
               (source->getMinPixelValue() < -1) ||
               (source->getMaxPixelValue() > 1))
            {
               normalItem = 0;
            }
         }
      }
   }
   else if(items.size() == 2)
   {
      DataManagerImageChainItem* item1 = *items.begin();
      DataManagerImageChainItem* item2 = *(items.begin()+1);
      if(item1&&item2)
      {
         ossimImageSource* source = item1->objectAsNode()->getObjectAs<ossimImageSource>();
         if((source->getNumberOfOutputBands() == 3)&&
            (source->getMinPixelValue() >= -1) &&
            (source->getMaxPixelValue() <= 1))
         {
            normalItem = item1;
            colorItem  = item2;
         }      
         else
         {
            source = item2->objectAsNode()->getObjectAs<ossimImageSource>();
            if((source->getNumberOfOutputBands() == 3)&&
               (source->getMinPixelValue() >= -1) &&
               (source->getMaxPixelValue() <= 1))
            {
               normalItem = item2;
               colorItem  = item1;
            }      
         }
      }
   }
   
   if(normalItem)
   {
      nodeToConnectList.push_back(normalItem->objectAsNode());
      if(colorItem) nodeToConnectList.push_back(colorItem->objectAsNode());
      
      ossimRefPtr<DataManager::Node> newNode = dataManager()->createDefaultCombinerChain("ossimBumpShadeTileSource", nodeToConnectList, false);
      newNodeList.push_back(newNode.get());
      ossimRefPtr<DataManager::Node> newDisplayNode = m_dataManager->createDefault2dImageDisplay(newNode.get(), false);
      newNodeList.push_back(newDisplayNode.get());
      displayList.push_back(newDisplayNode.get());
   }

   if(!newNodeList.empty())
   {
      populateTreeWithNodes(newNodeList);
   }
   if(!displayList.empty())
   {
      DataManagerEvent* event = new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
      event->setNodeList(displayList);
      QCoreApplication::postEvent(mainWindow(), event);
   }
   
}

void ossimGui::DataManagerWidget::factoryCombiner()
{
   bool ok = false;
   // we will allocate a new combiner
   QString combinerType = QInputDialog::getItem(this, "Image Combiner Selection", "Image Combiner:", m_combinerList, 0, false, &ok);
   if(ok && (combinerType != ""))
   {
      combineImagesWithType(combinerType);
   }
}

void ossimGui::DataManagerWidget::combineImagesWithType(const QString& combinerType)
{
   QList<DataManagerImageChainItem*> items= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   DataManager::NodeListType nodeList;
   QList<DataManagerImageChainItem*>::iterator iter = items.begin();
   while(iter != items.end())
   {
      if(dynamic_cast<DataManagerImageChainItem*> ((*iter)))
      {
         nodeList.push_back((*iter)->objectAsNode());
      }
      ++iter;
   }
   if(!nodeList.empty())
   {
      ossimRefPtr<DataManager::Node> newNode = dataManager()->createDefaultCombinerChain(combinerType.toStdString(), nodeList);
      if(newNode.valid())
      {
         DataManagerEvent* event = new DataManagerEvent();
         event->setNodeList(newNode.get());
         QCoreApplication::postEvent(mainWindow(), event);      
      }
   }
}


void ossimGui::DataManagerWidget::geographicView()
{
   QList<DataManagerImageChainItem*> items= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   if(!items.empty())
   {
      ossimTypeNameVisitor visitor("ossimViewInterface");
      ossimTypeNameVisitor displayVisitor("ConnectableDisplayObject", false, ossimVisitor::VISIT_OUTPUTS);
      QList<DataManagerImageChainItem*>::iterator iter = items.begin();
      while(iter != items.end())
      {
         if((*iter)->objectAsNode())
         {
            ossimObject* obj = (*iter)->objectAsNode()->getObject();
            obj->accept(visitor);
         }
         ++iter;
      }
      ossimCollectionVisitor::ListRef& listRef = visitor.getObjects();
      ossimCollectionVisitor::ListRef::iterator viewIter = listRef.begin();
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimDpt metersPerPixel(1.0, 1.0);
      ossimRefPtr<ossimMapProjection> mapProj = new ossimEquDistCylProjection();
      if(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         // grab information from the first view and then set everyone relative to that
         if(viewInterface)
         {
            ossimRefPtr<ossimImageGeometry> currentViewGeom = dynamic_cast<ossimImageGeometry*>(viewInterface->getView());
            if(currentViewGeom.valid())
            {
               metersPerPixel = currentViewGeom->getMetersPerPixel();
               metersPerPixel.x = metersPerPixel.y;
            }
         }
      }
      mapProj->setMetersPerPixel(metersPerPixel);
      ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry(0, mapProj.get());
      while(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         if(viewInterface)
         {
            ossimRefPtr<View> view = new View(View::SYNC_TYPE_GEOM, geom.get());
            {
               displayVisitor.reset();
               (*viewIter)->accept(displayVisitor);
               ossimCollectionVisitor::ListRef& displayListRef = displayVisitor.getObjects();
               ossimCollectionVisitor::ListRef::iterator displayListIter = displayListRef.begin();
               if(!displayListRef.empty())
               {
                  while(displayListIter != displayListRef.end())
                  {
                     ConnectableDisplayObject* displayObject = dynamic_cast<ConnectableDisplayObject*> ((*displayListIter).get());
                     if(displayObject&&displayObject->display())
                     {
                        displayObject->display()->sync(*view);
                     }

                     ++displayListIter;
                  }
               }
               else 
               {
                  viewInterface->setView(geom.get());
                  ossimEventVisitor visitor(refreshEvent.get());
                  (*viewIter)->accept(visitor);
               }

            }
         }
         ++viewIter;
      }
      visitor.reset();
      displayVisitor.reset();
   }
}

void ossimGui::DataManagerWidget::scaledGeographicView()
{
   QList<DataManagerImageChainItem*> items= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   if(!items.empty())
   {
      ossimTypeNameVisitor visitor("ossimViewInterface");
      ossimTypeNameVisitor displayVisitor("ConnectableDisplayObject", false, ossimVisitor::VISIT_OUTPUTS);
      QList<DataManagerImageChainItem*>::iterator iter = items.begin();
      while(iter != items.end())
      {
         if((*iter)->objectAsNode())
         {
            ossimObject* obj = (*iter)->objectAsNode()->getObject();
            obj->accept(visitor);
         }
         ++iter;
      }
      ossimCollectionVisitor::ListRef& listRef = visitor.getObjects();
      ossimCollectionVisitor::ListRef::iterator viewIter = listRef.begin();
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimDpt metersPerPixel(1.0, 1.0);
      ossimRefPtr<ossimMapProjection> mapProj = new ossimEquDistCylProjection();
      ossimGpt centerGround;
      if(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         // grab information from the first view and then set everyone relative to that
         if(viewInterface)
         {
            ossimRefPtr<ossimImageGeometry> currentViewGeom = dynamic_cast<ossimImageGeometry*>(viewInterface->getView());
            if(currentViewGeom.valid())
            {
               metersPerPixel = currentViewGeom->getMetersPerPixel();
               metersPerPixel.x = metersPerPixel.y;
               ossimImageSource* imgSource=dynamic_cast<ossimImageSource*> ((*viewIter).get());
               if(imgSource)
               {
                  currentViewGeom->localToWorld(imgSource->getBoundingRect().midPoint(), centerGround);
               }
            }
         }
      }
      mapProj->setMetersPerPixel(metersPerPixel);
      mapProj->setOrigin(centerGround);
      ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry(0, mapProj.get());
      while(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         if(viewInterface)
         {
            ossimGpt centerGround;
            centerGround.makeNan();
            ossimDpt metersPerPixel(1.0, 1.0);
            ossimRefPtr<ossimMapProjection> mapProj = new ossimEquDistCylProjection();
            mapProj->setMetersPerPixel(metersPerPixel);
            
            ossimRefPtr<View> view = new View(View::SYNC_TYPE_GEOM, geom.get());
            {
               displayVisitor.reset();
               (*viewIter)->accept(displayVisitor);
               ossimCollectionVisitor::ListRef& displayListRef = displayVisitor.getObjects();
               ossimCollectionVisitor::ListRef::iterator displayListIter = displayListRef.begin();
               if(!displayListRef.empty())
               {
                  while(displayListIter != displayListRef.end())
                  {
                     ConnectableDisplayObject* displayObject = dynamic_cast<ConnectableDisplayObject*> ((*displayListIter).get());
                     if(displayObject&&displayObject->display())
                     {
                        displayObject->display()->sync(*view);
                     }
                     
                     ++displayListIter;
                  }
               }
               else 
               {
                  viewInterface->setView(geom.get());
                  ossimEventVisitor visitor(refreshEvent.get());
                  (*viewIter)->accept(visitor);
               }
               
            }
         }
         ++viewIter;
      }
      visitor.reset();
      displayVisitor.reset();
   }
}

void ossimGui::DataManagerWidget::utmView()
{
   QList<DataManagerImageChainItem*> items= grabSelectedChildItemsOfType<DataManagerImageChainItem>();
   if(!items.empty())
   {
      ossimTypeNameVisitor visitor("ossimViewInterface");
      ossimTypeNameVisitor displayVisitor("ConnectableDisplayObject", false, ossimVisitor::VISIT_OUTPUTS);
      QList<DataManagerImageChainItem*>::iterator iter = items.begin();
      while(iter != items.end())
      {
         if((*iter)->objectAsNode())
         {
            ossimObject* obj = (*iter)->objectAsNode()->getObject();
            obj->accept(visitor);
         }
         ++iter;
      }
      ossimCollectionVisitor::ListRef& listRef = visitor.getObjects();
      ossimCollectionVisitor::ListRef::iterator viewIter = listRef.begin();
      ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
      ossimDpt metersPerPixel(1.0, 1.0);
      ossimRefPtr<ossimUtmProjection> mapProj = new ossimUtmProjection();
      ossimGpt centerGround;
      if(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         // grab information from the first view and then set everyone relative to that
         if(viewInterface)
         {
            ossimRefPtr<ossimImageGeometry> currentViewGeom = dynamic_cast<ossimImageGeometry*>(viewInterface->getView());
            if(currentViewGeom.valid())
            {
               metersPerPixel = currentViewGeom->getMetersPerPixel();
               metersPerPixel.x = metersPerPixel.y;
               ossimImageSource* imgSource=dynamic_cast<ossimImageSource*> ((*viewIter).get());
               if(imgSource)
               {
                  currentViewGeom->localToWorld(imgSource->getBoundingRect().midPoint(), centerGround);
               }
            }
         }
      }
      mapProj->setMetersPerPixel(metersPerPixel);
      mapProj->setZone(centerGround);
      ossimRefPtr<ossimImageGeometry> geom = new ossimImageGeometry(0, mapProj.get());
      while(viewIter != listRef.end())
      {
         ossimViewInterface* viewInterface = dynamic_cast<ossimViewInterface*>((*viewIter).get());
         if(viewInterface)
         {
            ossimRefPtr<View> view = new View(View::SYNC_TYPE_GEOM, geom.get());
            {
               displayVisitor.reset();
               (*viewIter)->accept(displayVisitor);
               ossimCollectionVisitor::ListRef& displayListRef = displayVisitor.getObjects();
               ossimCollectionVisitor::ListRef::iterator displayListIter = displayListRef.begin();
               if(!displayListRef.empty())
               {
                  while(displayListIter != displayListRef.end())
                  {
                     ConnectableDisplayObject* displayObject = dynamic_cast<ConnectableDisplayObject*> ((*displayListIter).get());
                     if(displayObject&&displayObject->display())
                     {
                        displayObject->display()->sync(*view);
                     }
                     
                     ++displayListIter;
                  }
               }
               else 
               {
                  viewInterface->setView(geom.get());
                  ossimEventVisitor visitor(refreshEvent.get());
                  (*viewIter)->accept(visitor);
               }
               
            }
         }
         ++viewIter;
      }
      visitor.reset();
      displayVisitor.reset();
   }
}

void ossimGui::DataManagerWidget::openLocalImage()
{
   if(m_jobQueue)
   {
      QStringList fileNames = QFileDialog::getOpenFileNames(
         this,
         tr("Open Image(s)"),
         m_lastOpenedDirectory.c_str() );

      if (fileNames.size() > 0) 
      {
         for (int i = 0; i < fileNames.size(); ++i)
         {
            QUrl url = QUrl::fromLocalFile(fileNames.at(i));
            std::shared_ptr<OpenImageUrlJob> job = std::make_shared<OpenImageUrlJob>(url);
            job->setName(ossimString("open ") + url.toString().toStdString());
            std::shared_ptr<ImageOpenJobCallback> callback = std::make_shared<ImageOpenJobCallback>(this, m_dataManager);
            job->setCallback(callback);
            m_jobQueue->add(job);

            // Capture the directory for next QFileDialog::getOpenFileNames(...)
            ossimFilename f = fileNames.at(i).toStdString();
            if ( f.size() )
            {
               ossimFilename d;
               if ( f.isDir() )
               {
                  d = f;
               }
               else
               {
                  d = f.expand().path();
               }
               if ( d.isDir() )
               {
                  m_lastOpenedDirectory = d;
               }
            }
         }
      }
   }
}

void ossimGui::DataManagerWidget::openLocalImageInteractive()
{
   if(m_jobQueue)
   {
      QStringList fileNames = QFileDialog::getOpenFileNames(
         this,
         tr("Open Image(s)"),
         m_lastOpenedDirectory.c_str() );
      
      if (fileNames.size() > 0) 
      {
         for (int i = 0; i < fileNames.size(); ++i)
         {
            ossimFilename file = fileNames.at(i).toStdString();
            ossimRefPtr<ossimImageHandler> ih =
               ossimImageHandlerRegistry::instance()->open(file);
            if(ih.valid())
            {
               ossimGui::HandlerList handlers;
               
               ossim_uint32 nEntries = ih->getNumberOfEntries();
               if ( nEntries == 1 )
               {
                  handlers.push_back( ih.get() );
               }
               else
               {
                  // Let the user select entries.
                  ossimGui::OpenImageDialog oid( ih.get() );
                  oid.exec();
                  oid.handlerList( handlers );
               }

               if ( handlers.size() )
               {
                  ossim_uint32 idx = 0;
                  ossim_uint32 nImages = handlers.size();
                  DataManager::NodeListType nodeList;;
                  for(idx = 0; idx < nImages; ++idx)
                  {
                     ossimRefPtr<DataManager::Node> node =
                        m_dataManager->addSource( handlers[idx].get() );
                     if(node.valid())
                     {
                        ossimRefPtr<DataManager::Node> chain =
                           m_dataManager->createDefaultImageChain(node.get());
                        if(chain.valid())
                        {
                           nodeList.push_back(chain.get());
                        }
                     }
                  }
                  if(!nodeList.empty())
                  {
                     DataManagerEvent* e =
                        new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
                     e->setNodeList(nodeList);
                     QCoreApplication::postEvent( mainWindow(), e );
                  }
               }
            }
            
            // Capture the directory for next QFileDialog::getOpenFileNames(...)
            ossimFilename f = fileNames.at(i).toStdString();
            if ( f.size() )
            {
               ossimFilename d;
               if ( f.isDir() )
               {
                  d = f;
               }
               else
               {
                  d = f.expand().path();
               }
               if ( d.isDir() )
               {
                  m_lastOpenedDirectory = d;
               }
            }
         }
      }
   }
   
} // End: ossimGui::DataManagerWidget::openLocalImageInteractive()

void ossimGui::DataManagerWidget::openImageUrl()
{
   bool ok;
   QString text = QInputDialog::getText(
      this, tr("Open image from url."),
      tr("URL:"), QLineEdit::Normal,
      QString(""), &ok);
   if (ok && !text.isEmpty())
   {
      ossimUrl url( ossimString(text.toStdString()) );
      if ( ( url.getProtocol().downcase().string() == std::string("s3") ) ||
           ( url.getProtocol().downcase().string() == std::string("file") ) )
      {
         // Note: ossimUrl::toString() was stripping a '/' so using original.
         ossimRefPtr<ossimImageHandler> ih =
            ossimImageHandlerRegistry::instance()->openConnection(  text.toStdString(), true );
         if(ih.valid())
         {
            ossimGui::HandlerList handlers;
            
            ossim_uint32 nEntries = ih->getNumberOfEntries();
            if ( nEntries == 1 )
            {
               handlers.push_back( ih.get() );
            }
            else
            {
               // Let the user select entries.
               ossimGui::OpenImageDialog oid( ih.get() );
               oid.exec();
                  oid.handlerList( handlers );
            }
            
            if ( handlers.size() )
            {
               ossim_uint32 idx = 0;
               ossim_uint32 nImages = handlers.size();
               DataManager::NodeListType nodeList;;
               for(idx = 0; idx < nImages; ++idx)
               {
                  ossimRefPtr<DataManager::Node> node =
                     m_dataManager->addSource( handlers[idx].get() );
                  if(node.valid())
                  {
                     ossimRefPtr<DataManager::Node> chain =
                        m_dataManager->createDefaultImageChain(node.get());
                     if(chain.valid())
                     {
                        nodeList.push_back(chain.get());
                     }
                  }
               }
               if(!nodeList.empty())
               {
                  DataManagerEvent* e =
                     new DataManagerEvent(DataManagerEvent::COMMAND_DISPLAY_NODE);
                  e->setNodeList(nodeList);
                  QCoreApplication::postEvent( mainWindow(), e );
               }
            }
         }
      }
   }
   
} // End: ossimGui::DataManagerWidget::openImageURL()

void ossimGui::DataManagerWidget::openJpipImage()
{
}

void ossimGui::DataManagerWidget::createTiffWriter()
{
   createWriterFromType("ossimTiffWriter");
}

void ossimGui::DataManagerWidget::createJpegWriter()
{
   createWriterFromType("ossimJpegWriter");
}

void ossimGui::DataManagerWidget::createWriterFromFactory()
{
   QStringList writerList;
   Util::imageWriterTypes(writerList);
   if(!writerList.empty())
   {
      bool ok = false;
      // we will allocate a new combiner
      QString writerType = QInputDialog::getItem(this, "Image Writer Selection", "Image writer:", writerList, 0, false, &ok);
      if(ok && (writerType != ""))
      {
         createWriterFromType(writerType);
      }
   }
}

void ossimGui::DataManagerWidget::createWriterFromType(const QString& type)
{
   m_activeItemsMutex.lock();
   ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(type.toStdString()));
   if(obj.valid())
   {
      ossimRefPtr<DataManager::Node> node = m_dataManager->addSource(obj.get(), false);
      if(node.valid())
      {
         DataManagerImageFileWriterItem* item = new DataManagerImageFileWriterItem(node.get());
         m_imageWriters->addChild(item);
         m_activeItems.insert(item);
      }
   }
   m_activeItemsMutex.unlock();
}

void ossimGui::DataManagerWidget::createFixedRegistration()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   createDefaultFixedRegistrationItem();
#else
   QMessageBox::warning(this,
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

ossimGui::DataManagerRegistrationItem*
ossimGui::DataManagerWidget::createDefaultFixedRegistrationItem()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   ossimRefPtr<ossimFixedRegistrationSource> registration =
      new ossimFixedRegistrationSource();
   ossim_autoreg::AutoRegistrationOptions registrationOptions =
      registration->autoRegistrationOptions();
   RegistrationSetupOptions setupOptions =
      registrationSetupDefaults(REGISTRATION_SETUP_FIXED_AUTO,
                                preferredFixedAutoMatchMethod());
   ossim_autoreg::TiePointGenerationOptions tiePointOptions =
      registrationOptions.generator();
   applyRegistrationSetupTieOptions(tiePointOptions, setupOptions);
   tiePointOptions.setMatchMethod(std::string());
   registrationOptions.setAutoRegister(true);
   registrationOptions.setGenerateTiePoints(true);
   registrationOptions.setSkipOptimization(false);
   registrationOptions.setRegistrationPasses(4);
   registrationOptions.setTargetRmsePixels(4.0);
   registrationOptions.setRmseImprovementTolerance(0.01);
   registrationOptions.setGenerator(tiePointOptions);
   registration->setAutoRegistrationOptions(registrationOptions);

   ossimRefPtr<ossimObject> obj = registration.get();
   if(obj.valid())
   {
      std::lock_guard<std::mutex> lock(m_activeItemsMutex);
      ossimRefPtr<DataManager::Node> node = m_dataManager->addSource(obj.get(), false);
      if(node.valid())
      {
         node->setName("Registered: adaptive fixed auto");
         DataManagerRegistrationItem* item = new DataManagerRegistrationItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         m_registrationSources->addChild(item);
         m_activeItems.insert(item);
         return item;
      }
   }
#endif
   return 0;
}

void ossimGui::DataManagerWidget::createFixedOpenCvAutoRegistration()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   if(!ossim_autoreg::TiePointGeneratorFactory::instance()->
         create("opencv-phase-correlation"))
   {
      QMessageBox::warning(
         this,
         "Registration",
         "The OpenCV phase-correlation tie-point generator is not available.");
      return;
   }

   ossimRefPtr<ossimFixedRegistrationSource> registration =
      new ossimFixedRegistrationSource();
   registration->applyAutoRegistrationPreset("fixed:opencv-phase-ransac");
   ossimRefPtr<ossimObject> obj = registration.get();
   if(obj.valid())
   {
      std::lock_guard<std::mutex> lock(m_activeItemsMutex);
      ossimRefPtr<DataManager::Node> node = m_dataManager->addSource(obj.get(), false);
      if(node.valid())
      {
         node->setName(
            registeredNodeName("opencv-phase-correlation").
               toStdString().c_str());
         DataManagerRegistrationItem* item = new DataManagerRegistrationItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         item->setToolTip(
            0,
            QString("%1\nTie point generators: %2")
               .arg(registration->autoRegistrationSettingsSummary().c_str())
               .arg(tiePointGeneratorSummary()));
         m_registrationSources->addChild(item);
         m_activeItems.insert(item);
      }
   }
#else
   QMessageBox::warning(this,
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

void ossimGui::DataManagerWidget::createBundleFloatingRegistration()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   createDefaultBundleFloatingRegistrationItem();
#else
   QMessageBox::information(this,
                            "Registration",
                            "ossim-registration-source is not enabled in this build.");
#endif
}

ossimGui::DataManagerRegistrationItem*
ossimGui::DataManagerWidget::createDefaultBundleFloatingRegistrationItem()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   ossimRefPtr<ossimBundleAdjustmentRegistrationSource> bundle =
      new ossimBundleAdjustmentRegistrationSource();
   bundle->setAllInputsFloating(true);
   applyBundleDefaultsToSource(bundle.get(), false);
   ossimRefPtr<ossimObject> obj = bundle.get();
   if(obj.valid())
   {
      std::lock_guard<std::mutex> lock(m_activeItemsMutex);
      ossimRefPtr<DataManager::Node> node =
         m_dataManager->addSource(obj.get(), false);
      if(node.valid())
      {
         node->setName("Bundle All-Floating Registration");
         DataManagerRegistrationItem* item =
            new DataManagerRegistrationItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         m_registrationSources->addChild(item);
         m_activeItems.insert(item);
         return item;
      }
   }
#endif
   return 0;
}

QList<ossimGui::DataManagerItem*>
ossimGui::DataManagerWidget::selectedRegistrationInputItems() const
{
   QList<DataManagerItem*> result;
   QList<QTreeWidgetItem*> selectedNodes = selectedItems();
   QList<QTreeWidgetItem*>::iterator iter = selectedNodes.begin();
   while(iter != selectedNodes.end())
   {
      DataManagerItem* item = dynamic_cast<DataManagerItem*>(*iter);
      if(item &&
         (item->itemAs<DataManagerRawImageSourceItem>() ||
          item->itemAs<DataManagerImageChainItem>()) &&
         item->objectAsNode())
      {
         result.push_back(item);
      }
      ++iter;
   }
   return result;
}

void ossimGui::DataManagerWidget::connectAndExecuteSelectedRegistration(
   DataManagerRegistrationItem* item)
{
   if(!item)
      return;

   QList<DataManagerItem*> inputs = selectedRegistrationInputItems();
   if(inputs.size() < 2)
   {
      QMessageBox::warning(this,
                           "Registration",
                           "Select at least two Sources or Chains before "
                           "starting registration.");
      return;
   }

   item->dropItems(inputs);
   item->setSelected(true);
   item->execute();
}

void ossimGui::DataManagerWidget::createFixedRegistrationFromSelection()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   connectAndExecuteSelectedRegistration(createDefaultFixedRegistrationItem());
#else
   QMessageBox::warning(this,
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

void ossimGui::DataManagerWidget::createBundleFloatingRegistrationFromSelection()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   connectAndExecuteSelectedRegistration(
      createDefaultBundleFloatingRegistrationItem());
#else
   QMessageBox::information(this,
                            "Registration",
                            "ossim-registration-source is not enabled in this build.");
#endif
}

void ossimGui::DataManagerWidget::createRegistrationFromDialog()
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   RegistrationSetupDialog dialog(this);
   if(dialog.exec() != QDialog::Accepted)
      return;

   const RegistrationSetupOptions setupOptions = dialog.options();
   if(!setupOptions.matchMethod.empty() &&
      !ossim_autoreg::TiePointGeneratorFactory::instance()->
         create(setupOptions.matchMethod))
   {
      QMessageBox::warning(
         this,
         "Registration",
         QString("The selected tie-point generator is not available: %1\n\n"
                 "Available generators: %2")
            .arg(QString::fromStdString(setupOptions.matchMethod))
            .arg(tiePointGeneratorSummary()));
      return;
   }

   ossimRefPtr<ossimObject> obj;
   QString nodeName;
   QString toolTip;

   if(setupOptions.approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING ||
      setupOptions.approach == REGISTRATION_SETUP_BUNDLE_ANCHORED)
   {
      ossimRefPtr<ossimBundleAdjustmentRegistrationSource> bundle =
         new ossimBundleAdjustmentRegistrationSource();
      bundle->setAllInputsFloating(
         setupOptions.approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING);
      applyBundleDefaultsToSource(
         bundle.get(),
         setupOptions.approach == REGISTRATION_SETUP_BUNDLE_ANCHORED);
      ossim_autoreg::AutoRegistrationOptions registrationOptions =
         bundle->autoRegistrationOptions();
      ossim_autoreg::TiePointGenerationOptions tiePointOptions =
         registrationOptions.generator();
      applyRegistrationSetupTieOptions(tiePointOptions, setupOptions);
      registrationOptions.setGenerator(tiePointOptions);
      bundle->setAutoRegistrationOptions(registrationOptions);
      obj = bundle.get();
      nodeName = bundle->allInputsFloating() ?
         "Bundle All-Floating Registration" :
         "Bundle Anchored Registration";
      toolTip =
         QString("Matcher: %1\nResampler: %2\nView GSD: %3")
            .arg(QString::fromStdString(setupOptions.matchMethod))
            .arg(QString::fromStdString(setupOptions.resamplerType))
            .arg(setupOptions.viewGsd);
   }
   else
   {
      ossimRefPtr<ossimFixedRegistrationSource> registration =
         new ossimFixedRegistrationSource();
      ossim_autoreg::AutoRegistrationOptions registrationOptions =
         registration->autoRegistrationOptions();
      if(setupOptions.approach == REGISTRATION_SETUP_FIXED_AUTO)
      {
         if(setupOptions.matchMethod == "opencv-phase-correlation")
            registration->applyAutoRegistrationPreset(
               "fixed:opencv-phase-ransac");
         else if(setupOptions.matchMethod == "spatial-ncc")
            registration->applyAutoRegistrationPreset("fixed:spatial-ncc");
         else if(setupOptions.matchMethod == "hybrid-phase-ncc")
            registration->applyAutoRegistrationPreset("fixed:hybrid");
         else if(setupOptions.matchMethod == "phase-correlation")
            registration->applyAutoRegistrationPreset("fixed:phase");
         registration->setAutoRegistrationEnabled(true);
         registration->setRegistrationPasses(4);
         registration->setTargetRmsePixels(4.0);
         registration->setRmseImprovementTolerance(0.01);
         registrationOptions = registration->autoRegistrationOptions();
      }

      ossim_autoreg::TiePointGenerationOptions tiePointOptions =
         registrationOptions.generator();
      applyRegistrationSetupTieOptions(tiePointOptions, setupOptions);
      if(setupOptions.approach == REGISTRATION_SETUP_FIXED_AUTO &&
         setupOptions.matchMethod.empty())
      {
         tiePointOptions.setMatchMethod(std::string());
      }
      registrationOptions.setGenerator(tiePointOptions);
      registration->setAutoRegistrationOptions(registrationOptions);
      obj = registration.get();
      nodeName = setupOptions.matchMethod.empty() ?
         QString("Registered: adaptive fixed auto") :
         registeredNodeName(setupOptions.matchMethod);
      toolTip =
         QString("%1\nMatcher: %2\nResampler: %3\nView GSD: %4")
            .arg(registration->autoRegistrationSettingsSummary().c_str())
            .arg(QString::fromStdString(
               setupOptions.matchMethod.empty() ?
                  std::string("adaptive auto") :
                  setupOptions.matchMethod))
            .arg(QString::fromStdString(setupOptions.resamplerType))
            .arg(setupOptions.viewGsd);
   }

   if(obj.valid())
   {
      std::lock_guard<std::mutex> lock(m_activeItemsMutex);
      ossimRefPtr<DataManager::Node> node =
         m_dataManager->addSource(obj.get(), false);
      if(node.valid())
      {
         node->setName(nodeName.toStdString().c_str());
         DataManagerRegistrationItem* item =
            new DataManagerRegistrationItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         item->setToolTip(0, toolTip);
         m_registrationSources->addChild(item);
         m_activeItems.insert(item);
      }
   }
#else
   QMessageBox::warning(this,
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

void ossimGui::DataManagerWidget::setSelectedBundleAllFloating(bool enabled)
{
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
   QList<DataManagerRegistrationItem*> result =
      grabSelectedChildItemsOfType<DataManagerRegistrationItem>();
   QList<DataManagerRegistrationItem*>::iterator iter = result.begin();
   while(iter != result.end())
   {
      DataManagerRegistrationItem* item = *iter;
      if(item && item->objectAsNode())
      {
         ossimBundleAdjustmentRegistrationSource* bundle =
            item->objectAsNode()->
               getObjectAs<ossimBundleAdjustmentRegistrationSource>();
         if(bundle)
         {
            bundle->setAllInputsFloating(enabled);
            if(enabled)
               item->setToolTip(0, "All bundle inputs participate in adjustment.");
            else
               item->setToolTip(0, "Input 0 is held as the bundle anchor.");
         }
      }
      ++iter;
   }
#else
   (void)enabled;
   QMessageBox::warning(this,
                        "Registration",
                        "ossim-registration-source is not enabled in this build.");
#endif
}

void ossimGui::DataManagerWidget::registerSelected()
{
   QList<DataManagerRegistrationItem*> result =
      grabSelectedChildItemsOfType<DataManagerRegistrationItem>();
   if(!result.empty())
   {
      QList<DataManagerRegistrationItem*>::iterator iter = result.begin();
      while(iter != result.end())
      {
         (*iter)->execute();
         ++iter;
      }
   }
}

void ossimGui::DataManagerWidget::executeSelected()
{
   QList<DataManagerImageWriterItem*> result = grabSelectedChildItemsOfType<DataManagerImageWriterItem>();
   if(!result.empty())
   {
      QList<DataManagerImageWriterItem*>::iterator iter = result.begin();
      while(iter != result.end())
      {
         (*iter)->execute();
         
         ++iter;
      }
   }

}

void ossimGui::DataManagerWidget::displayCropViewport()
{
   QList<DataManagerDisplayItem*> result = grabSelectedChildItemsOfType<DataManagerDisplayItem>();
   if(!result.empty())
   {
      QList<DataManagerDisplayItem*>::iterator iter = result.begin();
      while(iter != result.end())
      {
         DataManager::Node* node = (*iter)->objectAsNode();
         if(node)
         {
            ConnectableDisplayObject* diplayObj = node->getObjectAs<ConnectableDisplayObject>();
            
            if(diplayObj&&diplayObj->display())
            {
               ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(diplayObj->display());
               if(subWindow&&subWindow->scrollWidget()&&subWindow->scrollWidget()->layers())
               {
                  ossimIrect rect = subWindow->scrollWidget()->viewportBoundsInSceneSpace();
                  
                  QRect scrollRect(rect.ul().x, rect.ul().y, rect.width(), rect.height());//subWindow->scrollWidget()->viewToScroll().mapRect(QRect(rect.ul().x, rect.ul().y, rect.width(), rect.height()));
                  
                  ossimGui::Image image(scrollRect.size(), QImage::Format_RGB32);
                  image.setOffset(QPoint(scrollRect.x(), scrollRect.y()));
                  ossimGui::ImageScrollView::Layer* layer = subWindow->scrollWidget()->layers()->layer((ossim_uint32)0);
                  if(layer&&layer->tileCache()&&layer->chain())
                  {
                     layer->tileCache()->getSubImage(image);
                     ossimRefPtr<ossimImageData> data = image.toOssimImage();
                     data->setOrigin(rect.ul()); // change to the view plane origin
                     if(data.valid())
                     {
                        ossimRefPtr<ossimImageGeometry> geom = layer->chain()->getImageGeometry();
                        if(geom.valid())
                        {
                           geom = static_cast<ossimImageGeometry*>(geom->dup());
                           ossimMapProjection* mapProj = dynamic_cast<ossimMapProjection*>(geom->getProjection());
                           if(mapProj)
                           {
                              ossimGpt ulGpt;
                              geom->localToWorld(data->getImageRectangle().ul(), ulGpt);
                              mapProj->setUlTiePoints(ulGpt);
                              
                              data->setOrigin(ossimIpt(0,0));
                           }
                           else 
                           {
                              
                           }
                        }
                        
                        
                        ossimRefPtr<ossimMemoryImageSource> imgSource = new ossimMemoryImageSource();
                        imgSource->setImage(data.get());
                        imgSource->setImageGeometry(geom.get());
                        ossimRefPtr<DataManager::Node> rawNode = dataManager()->addSource(imgSource.get()).get();
                        ossimRefPtr<DataManager::Node> chainNode = dataManager()->createDefaultImageChain(rawNode.get()).get();
                        
                        
                        DataManagerEvent* event = new DataManagerEvent();
                        event->setNodeList(chainNode.get());
                        QCoreApplication::postEvent(mainWindow(), event);
                     }
                  }
               }
            }
         }
         ++iter;
      }
   }
}

void ossimGui::DataManagerWidget::removeAllFilters()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   while(iter != items.end())
   {
      DataManagerImageFilterFolder* folder = dynamic_cast<DataManagerImageFilterFolder*> (*iter);
      if(folder)
      {
         folder->removeFilters();
         
         if(folder->object())
         {
            ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
            ossimEventVisitor visitor(refreshEvent.get());
            folder->object()->accept(visitor);
         }
      }
      ++iter;
   }
}

void ossimGui::DataManagerWidget::addFilterToFront()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();

   if(iter != items.end())
   {
      DataManagerImageFilterFolder* folder = dynamic_cast<DataManagerImageFilterFolder*> (*iter);
      if(folder)
      {         
         bool ok=false;
         QString filterType = QInputDialog::getItem(this, "Image Filter Selection", "Image Filter:", m_filterList, 0, false, &ok);
         if(ok &&(filterType != ""))
         {
            ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(filterType.toStdString()));
            if(obj.valid())
            {
               folder->addFilterToFront(obj.get());
               
               ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
               ossimEventVisitor visitor(refreshEvent.get());
               obj->accept(visitor);
               // std::cout << "Will add " << filterType.toAscii().data() << " to front " << std::endl;
            }
         }
      }
   }
}

void ossimGui::DataManagerWidget::addFilterToEnd()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   
   if(iter != items.end())
   {
      DataManagerImageFilterFolder* folder = dynamic_cast<DataManagerImageFilterFolder*> (*iter);
      if(folder)
      {
         bool ok=false;
         QString filterType = QInputDialog::getItem(this, "Image Filter Selection", "Image Filter:", m_filterList, 0, false, &ok);
         if(ok &&(filterType != ""))
         {
            ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(filterType.toStdString()));
            if(obj.valid())
            {
               folder->addFilterToEnd(obj.get());
               
               ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
               ossimEventVisitor visitor(refreshEvent.get());
               obj->accept(visitor);
               // std::cout << "Will add " << filterType.toAscii().data() << " to front " << std::endl;
            }
         }
      }
   }
}

void ossimGui::DataManagerWidget::insertFilterBefore()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   
   if(iter != items.end())
   {
      DataManagerImageFilterItem* item = dynamic_cast<DataManagerImageFilterItem*> (*iter);
      if(item)
      {
         DataManagerImageFilterFolder* folder = item->folder();
         if(folder)
         {
            bool ok=false;
            QString filterType = QInputDialog::getItem(this, "Image Filter Selection", "Image Filter:", m_filterList, 0, false, &ok);
            if(ok &&(filterType != ""))
            {
               ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(filterType.toStdString()));
               if(obj.valid())
               {
                  folder->insertFilterBefore(obj.get(), item->object());
                  
                  ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
                  ossimEventVisitor visitor(refreshEvent.get());
                  obj->accept(visitor);
                  // std::cout << "Will add " << filterType.toAscii().data() << " to front " << std::endl;
               }
            }
         }
      }
   }
}

void ossimGui::DataManagerWidget::insertFilterAfter()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   
   if(iter != items.end())
   {
      DataManagerImageFilterItem* item = dynamic_cast<DataManagerImageFilterItem*> (*iter);
      if(item)
      {
         DataManagerImageFilterFolder* folder = item->folder();
         if(folder)
         {
            bool ok=false;
            QString filterType = QInputDialog::getItem(this, "Image Filter Selection", "Image Filter:", m_filterList, 0, false, &ok);
            if(ok &&(filterType != ""))
            {
               ossimRefPtr<ossimObject> obj = ossimObjectFactoryRegistry::instance()->createObject(ossimString(filterType.toStdString()));
               if(obj.valid())
               {
                  folder->insertFilterAfter(obj.get(), item->object());
                  
                  ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
                  ossimEventVisitor visitor(refreshEvent.get());
                  obj->accept(visitor);
                  // std::cout << "Will add " << filterType.toAscii().data() << " to front " << std::endl;
               }
            }
         }
      }
   }
}

void ossimGui::DataManagerWidget::removeFilter()
{
   QList<QTreeWidgetItem *> items=	selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   
   if(iter != items.end())
   {
      DataManagerImageFilterItem* item = dynamic_cast<DataManagerImageFilterItem*> (*iter);
      DataManagerImageFilterFolder* folder = item->folder();
      
      if(folder)
      {
         ossimConnectableObject* connectable = dynamic_cast<ossimConnectableObject*> (item->object());
         ossimConnectableObject::ConnectableObjectList outputList;
         if(connectable)
         {
            outputList = connectable->getOutputList();
         }
         folder->removeFilter(item->object());
         
         
         ossimConnectableObject::ConnectableObjectList::iterator iter = outputList.begin();
         ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
         ossimEventVisitor visitor(refreshEvent.get());
         while(iter!=outputList.end())
         {
            (*iter)->accept(visitor);
            ++iter;
         }
      }
   }
}

void ossimGui::DataManagerWidget::planetaryView()
{
   if(m_dataManager.valid()&&!m_planetaryDisplayNode.valid())
   {
      
      m_planetaryDisplayNode = m_dataManager->createDefault3dPlanetaryDisplay();
   }
   if(m_planetaryDisplayNode.valid())
   {
      ConnectableDisplayObject* connectableDisplay = m_planetaryDisplayNode->getObjectAs<ConnectableDisplayObject>();
      if(connectableDisplay)
      {
         if(connectableDisplay->display())
         {
            connectableDisplay->display()->show();
            connectableDisplay->display()->raise();
         }
      }
      QList<DataManagerImageChainItem*> items = grabSelectedChildItemsOfType<DataManagerImageChainItem>();
      QList<DataManagerImageChainItem*>::iterator itemsIter = items.begin();
      while(itemsIter != items.end())
      {
         DataManager::Node* iterNode=(*itemsIter)->objectAsNode();
         if(iterNode)
         {
            if(iterNode->getObjectAs<ossimImageSource>())
            {
               connectableDisplay->connectMyInputTo(iterNode->getObjectAs<ossimConnectableObject>());
            }
         }
         ++itemsIter;
      }   
   }
}

void ossimGui::DataManagerWidget::unselectAll()
{
   QList<QTreeWidgetItem *>	items = selectedItems();
   QList<QTreeWidgetItem *>::iterator iter = items.begin();
   while(iter != items.end())
   {
      (*iter)->setSelected(false);
      ++iter;
   }
}

void	ossimGui::DataManagerWidget::itemChanged( QTreeWidgetItem * item, int column )
{
   if(column == 0)
   {
      DataManagerNodeItem* nodeItem = dynamic_cast<DataManagerNodeItem*>(item);
      if(nodeItem&&nodeItem->objectAsNode())
      {
         ossimRefPtr<DataManager::Node> node = nodeItem->objectAsNode();
         if(node.valid())
         {
            ossimSource* source = node->getObjectAs<ossimSource>();
            if(source)
            {
               // check if the enable disable flag has changed
               //
               bool itemEnabled = item->checkState(0)!=Qt::Unchecked;
               if(itemEnabled !=source->isSourceEnabled())
               {
                  source->setEnableFlag(itemEnabled);
                  
                  ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
                  ossimEventVisitor visitor(refreshEvent.get());
                  source->accept(visitor);
               }
               QString name=item->text(0);
               // now check to see if the text changed
               //
               if(node->name()!=name)
               {
                  node->setName(name);
               }
            }
         }
      }
   }
}

void	ossimGui::DataManagerWidget::itemCollapsed (QTreeWidgetItem* item)
{
   QTreeWidget::collapseItem(item);

   DataManagerInputConnectionFolder* connectionFolder = dynamic_cast<DataManagerInputConnectionFolder*> (item);
   if(connectionFolder)
   {
      connectionFolder->clearChildren();
   }
}

void	ossimGui::DataManagerWidget::itemExpanded (QTreeWidgetItem* item)      
{
   QTreeWidget::expandItem(item);
   
   DataManagerFolder* connectionFolder = dynamic_cast<DataManagerFolder*> (item);
   if(connectionFolder)
   {
      connectionFolder->populateChildren();
   }
}

bool	ossimGui::DataManagerWidget::event( QEvent * e )
{
   switch(e->type())
   {
      case DATA_MANAGER_WIDGET_JOB_EVENT_ID:
      {
         DataManagerWidgetJobEvent* jEvent = dynamic_cast<DataManagerWidgetJobEvent*> (e);
         if(jEvent)
         {
            switch(jEvent->command())
            {
               case DataManagerWidgetJobEvent::COMMAND_JOB_ADD:
               {
                  DataManagerWidgetJobEvent::JobListType& jobList       = jEvent->jobList();
                  DataManagerWidgetJobEvent::JobListType::iterator iter = jobList.begin();
                  while(iter!=jobList.end())
                  {
                     m_jobQueue->add((*iter));
                     ++iter;
                  }
                  break;
               }
               case DataManagerWidgetJobEvent::COMMAND_JOB_ADDED:
               {
                  DataManagerWidgetJobEvent::JobListType& jobList       = jEvent->jobList();
                  DataManagerWidgetJobEvent::JobListType::iterator iter = jobList.begin();
                  while(iter!=jobList.end())
                  {
                     m_rootJobsFolder->addJob((*iter));
                     ++iter;
                  }
                  break;
               }
               case DataManagerWidgetJobEvent::COMMAND_JOB_STATE_CHANGED:
               {
                  DataManagerWidgetJobEvent::JobListType& jobList       = jEvent->jobList();
                  DataManagerWidgetJobEvent::JobListType::iterator iter = jobList.begin();
                  while(iter!=jobList.end())
                  {
                     m_rootJobsFolder->stateChanged(*iter);
                     ++iter;
                  }
                  
                  break;
               }
               case DataManagerWidgetJobEvent::COMMAND_JOB_PROPERTY_CHANGED:
               {
                  DataManagerWidgetJobEvent::JobListType& jobList       = jEvent->jobList();
                  DataManagerWidgetJobEvent::JobListType::iterator iter = jobList.begin();
                  while(iter!=jobList.end())
                  {
                     m_rootJobsFolder->propertyChanged(*iter);
                     ++iter;
                  }
                  break;
               }
               case DataManagerWidgetJobEvent::COMMAND_JOB_PERCENT_COMPLETE:
               {
                  DataManagerWidgetJobEvent::JobListType& jobList       = jEvent->jobList();
                  DataManagerWidgetJobEvent::JobListType::iterator iter = jobList.begin();
                  while(iter!=jobList.end())
                  {
                     m_rootJobsFolder->percentCompleteChanged(*iter, jEvent->percentComplete());
                     ++iter;
                  }
                  break;
               }
               default:
               {
                  break;
               }
            }
         }
         break;
      }
      case DATA_MANAGER_WIDGET_EVENT_ID:
      {
         DataManagerWidgetEvent* wEvent = dynamic_cast<DataManagerWidgetEvent*> (e);
         if(wEvent)
         {
            switch(wEvent->command())
            {
               case DataManagerWidgetEvent::COMMAND_CONNECT_INPUT:
               {
                  m_activeItemsMutex.lock();
                  DataManagerWidgetEvent::ItemListType& itemList = wEvent->itemList();
                  DataManagerWidgetEvent::ItemListType::iterator iter = itemList.begin();
                  while(iter!=itemList.end())
                  {
                     if(m_activeItems.find(*iter) != m_activeItems.end())
                     {
                        (*iter)->refreshChildConnections();
                        if((*iter)->objectAsNode())
                        {
                           ossimConnectableObject* obj = (*iter)->objectAsNode()->getObjectAs<ossimConnectableObject>();
                           
                           if(obj)
                           {
                              ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_ALL);
                              ossimEventVisitor visitor(refreshEvent.get());
                              obj->accept(visitor);
                           }
                        }
                     }
                     ++iter;
                  }
                  m_activeItemsMutex.unlock();
                  break;
               }
               case DataManagerWidgetEvent::COMMAND_DISCONNECT_INPUT:
               {
                  m_activeItemsMutex.lock();
                  DataManagerWidgetEvent::ItemListType& itemList = wEvent->itemList();
                  DataManagerWidgetEvent::ItemListType::iterator iter = itemList.begin();
                  while(iter!=itemList.end())
                  {
                     // first make sure this pointer is still active
                     if(m_activeItems.find(*iter) != m_activeItems.end())
                     {
                        if((*iter)->objectAsNode())
                        {
                           ossimRefPtr<ossimConnectableObject> obj = (*iter)->objectAsNode()->getObjectAs<ossimConnectableObject>();
                           if(obj.valid())
                           {
                              ConnectableDisplayObject* connectableDisplay = dynamic_cast<ConnectableDisplayObject*>(obj.get());
                              
                              if(connectableDisplay&&connectableDisplay->display()&&!connectableDisplay->display()->testAttribute(Qt::WA_DeleteOnClose))
                              {
                                 // do nothing for we don't want to auto delete an object that can't be deleted.
                                 // lets just call close for it will just hide the display
                                 //
                                 connectableDisplay->display()->close();
                              }
                              else if((!obj->isConnected())&&((*iter)->autoDelete()))
                              {
                                 m_dataManager->remove((*iter)->objectAsNode());
                              }
                              else 
                              {
                                 (*iter)->refreshChildConnections();
                                 ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_ALL);
                                 ossimEventVisitor visitor(refreshEvent.get());
                                 obj->accept(visitor);                              
                              }
                           }
                        }
                     }
                     ++iter;
                  }
                  m_activeItemsMutex.unlock();
                  break;
               }
               case DataManagerWidgetEvent::COMMAND_DELETE_NODE:
               {
                  m_activeItemsMutex.lock();
                  
                  DataManagerWidgetEvent::ItemListType& itemList = wEvent->itemList();
                  DataManagerWidgetEvent::ItemListType::iterator iter = itemList.begin();
                  while(iter != itemList.end())
                  {
                     if((*iter)->parent()) (*iter)->parent()->removeChild(*iter);
                     m_dataManager->remove((*iter)->objectAsNode(), false);
                     (*iter)->setObject(0);
                     ++iter;
                  }
                 iter = itemList.begin();
                  while(iter != itemList.end())
                  {
                     if(m_activeItems.erase(*iter)>0)
                     {
                        delete *iter;
                     }
                     ++iter;
                  }
                  ++iter;
                  m_activeItemsMutex.unlock();
                  break;
               }
               case DataManagerWidgetEvent::COMMAND_REFRESH:
               {
                  if(!wEvent->warningMessage().empty())
                  {
                     QMessageBox::warning(this,
                                          wEvent->warningTitle().c_str(),
                                          wEvent->warningMessage().c_str());
                  }

                  DataManagerWidgetEvent::HandlerListType& handlerList =
                     wEvent->handlerList();
                  DataManagerWidgetEvent::HandlerListType::iterator handlerIter =
                     handlerList.begin();
                  while(handlerIter != handlerList.end())
                  {
                     if((*handlerIter).valid())
                     {
                        (*handlerIter)->setImageGeometry(0);
                        (*handlerIter)->getImageGeometry();
                        ossimRefPtr<ossimRefreshEvent> refreshEvent =
                           new ossimRefreshEvent(
                              ossimRefreshEvent::REFRESH_GEOMETRY);
                        ossimEventVisitor visitor(refreshEvent.get(),
                                                  ossimVisitor::VISIT_ALL);
                        (*handlerIter)->accept(visitor);
                     }
                     ++handlerIter;
                  }

                  m_activeItemsMutex.lock();
                  DataManagerWidgetEvent::ItemListType& itemList = wEvent->itemList();
                  DataManagerWidgetEvent::ItemListType::iterator iter = itemList.begin();
                  while(iter != itemList.end())
                  {
                     if(m_activeItems.find(*iter) != m_activeItems.end())
                     {
                        ossimConnectableObject* connectable = (*iter)->objectAsNode()->getObjectAs<ossimConnectableObject>();
                        if(connectable)
                        {
                           ossimRefPtr<ossimRefreshEvent> refreshEvent = new ossimRefreshEvent(ossimRefreshEvent::REFRESH_GEOMETRY);
                           ossimEventVisitor visitor(refreshEvent.get(),
                                                     ossimVisitor::VISIT_ALL);
                           connectable->accept(visitor);

                           ossimConnectableObject::ConnectableObjectList& inputList =
                              connectable->getInputList();
                           ossim_uint32 inputIdx = 0;
                           for(inputIdx = 0; inputIdx < inputList.size(); ++inputIdx)
                           {
                              if(inputList[inputIdx].valid())
                              {
                                 ossimEventVisitor inputVisitor(
                                    refreshEvent.get(),
                                    ossimVisitor::VISIT_ALL);
                                 inputList[inputIdx]->accept(inputVisitor);
                              }
                           }
                        }
                     }
                     ++iter;
                  }
                  m_activeItemsMutex.unlock();
                  break;
               }
               case DataManagerWidgetEvent::COMMAND_RESET:
               {
                  m_activeItemsMutex.lock();
                  DataManagerWidgetEvent::ItemListType& itemList = wEvent->itemList();
                  DataManagerWidgetEvent::ItemListType::iterator iter = itemList.begin();
                  if(m_activeItems.find(*iter) != m_activeItems.end())
                  {
                     (*iter)->reset();
                  }
                  m_activeItemsMutex.unlock();
                 break;
               }
               default:
               {
                  break;
               }
            }
         }
         e->accept();
         break;
      }
      case DATA_MANAGER_EVENT_ID:
      {
         DataManagerEvent* wEvent = dynamic_cast<DataManagerEvent*> (e);
         if(wEvent)
         {
            DataManager::NodeListType& nodeList = wEvent->nodeList();
            switch(wEvent->command())
            {
               case DataManagerEvent::COMMAND_NODE_ADDED:
               {
                  populateTreeWithNodes(nodeList);
                  break;
               }
               case DataManagerEvent::COMMAND_NODE_REMOVED:
               {
                  m_activeItemsMutex.lock();
                  QList<DataManagerItem*> itemsToDelete;
                  std::set<DataManagerItem*>::iterator activeItemsIter = m_activeItems.begin();
                  while(activeItemsIter != m_activeItems.end())
                  {
                     if(std::find(nodeList.begin(), nodeList.end(), (*activeItemsIter)->objectAsNode()) != nodeList.end())
                     {
                        itemsToDelete.push_back(*activeItemsIter);
                     }
                     ++activeItemsIter;
                  }
                  QList<DataManagerItem*>::iterator itemsToDeleteIter = itemsToDelete.begin();
                  while(itemsToDeleteIter != itemsToDelete.end())
                  {
                     if(m_activeItems.erase(*itemsToDeleteIter)>0)
                     {
                        delete *itemsToDeleteIter;
                     }
                     ++itemsToDeleteIter;
                  }
                 m_activeItemsMutex.unlock();
                 break;
               }
            }
         }
         wEvent->accept();
         break;
      }
      default:
      {
         return QTreeWidget::event(e);
         break;
      }
   }
   
   return true;
}

void ossimGui::DataManagerWidget::populateTreeWithNodes(DataManager::NodeListType& nodes)
{
   ossim_uint32 idx = 0;
   for(idx = 0; idx < nodes.size();++idx)
   {
      ossimRefPtr<DataManager::Node> node = nodes[idx];
      if(node->getObjectAs<ossimImageHandler>())
      {
         DataManagerRawImageSourceItem* source = new DataManagerRawImageSourceItem(node.get());
         source->setFlags(source->flags()|Qt::ItemIsEditable);
         m_rawImageSources->addChild(source);
         m_activeItemsMutex.lock();
         m_activeItems.insert(source);
         m_activeItemsMutex.unlock();
      }
      else if(node->getObjectAs<ossimImageChain>())
      {
         DataManagerImageChainItem* chainItem = new DataManagerImageChainItem(node.get()); 
         chainItem->setFlags(chainItem->flags()|Qt::ItemIsEditable);
         m_imageChains->addChild(chainItem);
         //chainItem->refreshChildConnections();
         chainItem->populateChildren();
         m_activeItemsMutex.lock();
         m_activeItems.insert(chainItem);
         m_activeItemsMutex.unlock();
      }
      else if(node->getObjectAs<ConnectableDisplayObject>())
      {
         
         DataManagerDisplayItem* item = new DataManagerDisplayItem(node.get()); 
         item->setFlags(item->flags()|Qt::ItemIsEditable);

         ossimConnectableObject* conn = node->getObjectAs<ossimConnectableObject>();
         if(conn)
         {
            ossimTypeNameVisitor visitor("ossimImageHandler");
            conn->accept(visitor);
            if(!visitor.getObjects().empty())
            {
               ossimRefPtr<ossimImageHandler> input = dynamic_cast<ossimImageHandler*> (visitor.getObjects()[0].get());
               ossimString source = input->getFilename();
               QString windowTitle = QFontMetrics(QFont()).elidedText
                  (source.data(), Qt::ElideLeft, 400);
                  item->setText(0, windowTitle);
               m_imageDisplays->addChild(item);
               item->refreshChildConnections();
            }
         }

         m_activeItemsMutex.lock();
         m_activeItems.insert(item);
         m_activeItemsMutex.unlock();
         
         
         ConnectableDisplayObject* connectable = node->getObjectAs<ConnectableDisplayObject>();
         if(connectable)
         {
            item->setAutoDelete(connectable->autoDelete());
         }
      }
      else if(node->getObjectAs<ossimImageFileWriter>())
      {
         DataManagerImageFileWriterItem* item = new DataManagerImageFileWriterItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         m_imageWriters->addChild(item);
         item->refreshChildConnections();
         m_activeItemsMutex.lock();
         m_activeItems.insert(item);
         m_activeItemsMutex.unlock();
      }
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
      else if(node->getObjectAs<ossimFixedRegistrationSource>() ||
              node->getObjectAs<ossimBundleAdjustmentRegistrationSource>())
      {
         DataManagerRegistrationItem* item =
            new DataManagerRegistrationItem(node.get());
         item->setFlags(item->flags()|Qt::ItemIsEditable);
         m_registrationSources->addChild(item);
         item->refreshChildConnections();
         m_activeItemsMutex.lock();
         m_activeItems.insert(item);
         m_activeItemsMutex.unlock();
      }
#endif
      else if(node->getObjectAs<ossimImageSource>()) // we will default to a raw source for now
      {
         DataManagerRawImageSourceItem* source = new DataManagerRawImageSourceItem(node.get());
         source->setFlags(source->flags()|Qt::ItemIsEditable);
         m_rawImageSources->addChild(source);
         m_activeItemsMutex.lock();
         m_activeItems.insert(source);
         m_activeItemsMutex.unlock();
      }
   }
}

QModelIndex ossimGui::DataManagerWidget::indexFromDataManagerItem(DataManagerItem* item, int col)
{
   return indexFromItem(static_cast<QTreeWidgetItem*>(item), col);
}

QMenu* ossimGui::DataManagerWidget::createMenu(QList<DataManagerItem*>& selection, DataManagerItem* activeItem)
{
   ossim_uint32 nImageChainSelections = 0;
   ossim_uint32 nRawSourceSelections  = 0;
   ossim_uint32 nInputConnections     = 0;
   ossim_uint32 nDisplayItems         = 0;
   bool hasItems = false;
   QMenu* menu = 0;
   menu = new QMenu(this);
   if(dynamic_cast<DataManagerRawImageSourceFolder*>(activeItem))
   {
      QMenu* openMenu =new QMenu("Open Image");
      
      QAction* localOpenImageAction = openMenu->addAction("Local");
      QAction* localOpenImageInteractiveAction = openMenu->addAction("Local Interactive");
      QAction* jpipOpenImageAction = openMenu->addAction("JPIP");
      // openMenu->addAction(localOpenImageAction);
      // openMenu->addAction(jpipOpenImageAction);
      menu->addMenu(openMenu);
      connect(localOpenImageAction, SIGNAL(triggered(bool)), this, SLOT(openLocalImage()));
      connect(localOpenImageInteractiveAction, SIGNAL(triggered(bool)),
              this, SLOT(openLocalImageInteractive()));      
      connect(jpipOpenImageAction, SIGNAL(triggered(bool)), this, SLOT(openJpipImage()));
      
   }
   else if(dynamic_cast<DataManagerImageWriterFolder*> (activeItem))
   {
      QMenu* openMenu =new QMenu("Image File Writer");
      QAction* geotiffImageAction = openMenu->addAction("Tiff");
      QAction* jpegImageAction    = openMenu->addAction("Jpeg");
      QAction* factoryImageAction    = openMenu->addAction("Factory");
      openMenu->addAction(geotiffImageAction);
      openMenu->addAction(jpegImageAction);
      openMenu->addAction(factoryImageAction);
      menu->addMenu(openMenu);
      connect(geotiffImageAction, SIGNAL(triggered(bool)), this, SLOT(createTiffWriter()));
      connect(jpegImageAction, SIGNAL(triggered(bool)), this, SLOT(createJpegWriter()));
      connect(factoryImageAction, SIGNAL(triggered(bool)), this, SLOT(createWriterFromFactory()));
   }
   else if(dynamic_cast<DataManagerRegistrationFolder*> (activeItem))
   {
      QMenu* registrationMenu = new QMenu("Registration");
      QAction* setupAction = registrationMenu->addAction("Setup...");
      registrationMenu->addSeparator();
      QAction* fixedAction = registrationMenu->addAction("Fixed");
      QAction* fixedOpenCvAction =
         registrationMenu->addAction("Fixed OpenCV Auto");
      QAction* bundleFloatingAction =
         registrationMenu->addAction("Bundle/Floating");
#ifndef OSSIM_REGISTRATION_SOURCE_ENABLED
      setupAction->setEnabled(false);
      fixedAction->setEnabled(false);
      fixedOpenCvAction->setEnabled(false);
      bundleFloatingAction->setEnabled(false);
#endif
      menu->addMenu(registrationMenu);
      connect(setupAction,
              SIGNAL(triggered(bool)),
              this,
              SLOT(createRegistrationFromDialog()));
      connect(fixedAction, SIGNAL(triggered(bool)), this, SLOT(createFixedRegistration()));
      connect(fixedOpenCvAction,
              SIGNAL(triggered(bool)),
              this,
              SLOT(createFixedOpenCvAutoRegistration()));
      connect(bundleFloatingAction,
              SIGNAL(triggered(bool)),
              this,
              SLOT(createBundleFloatingRegistration()));
   }
   else if(dynamic_cast<DataManagerRegistrationItem*> (activeItem))
   {
      QAction* registerAction = menu->addAction("Register");
      connect(registerAction, SIGNAL(triggered(bool)), this, SLOT(registerSelected()));
      activeItem->setSelected(true);
#ifdef OSSIM_REGISTRATION_SOURCE_ENABLED
      DataManagerRegistrationItem* registrationItem =
         dynamic_cast<DataManagerRegistrationItem*> (activeItem);
      ossimBundleAdjustmentRegistrationSource* bundleRegistration = 0;
      if(registrationItem && registrationItem->objectAsNode())
      {
         bundleRegistration = registrationItem->objectAsNode()->
            getObjectAs<ossimBundleAdjustmentRegistrationSource>();
      }
      if(bundleRegistration)
      {
         QAction* allFloatingAction = menu->addAction("All Images Float");
         allFloatingAction->setCheckable(true);
         allFloatingAction->setChecked(
            bundleRegistration->allInputsFloating());
         connect(allFloatingAction,
                 SIGNAL(triggered(bool)),
                 this,
                 SLOT(setSelectedBundleAllFloating(bool)));
      }
#endif
      QAction* deleteAction = menu->addAction("Delete");
      connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteSelected()));
   }
   else if(dynamic_cast<DataManagerImageWriterItem*> (activeItem))
   {
      QAction* executeAction = menu->addAction("Execute");
      connect(executeAction, SIGNAL(triggered(bool)), this, SLOT(executeSelected()));
      activeItem->setSelected(true);
      QAction* deleteAction = menu->addAction("Delete");
      connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteSelected()));
   }
   else if(dynamic_cast<DataManagerJobItem*>(activeItem))
   {
      QAction* cancelAction = menu->addAction("Cancel");
      connect(cancelAction, SIGNAL(triggered(bool)), this, SLOT(cancelSelected()));
   }
   else if(dynamic_cast<DataManagerDisplayItem*> (activeItem))
   {
      QMenu* displayMenu = new QMenu("Display");
      QAction* exportViewportAction = displayMenu->addAction("Crop viewport");
      connect(exportViewportAction, SIGNAL(triggered(bool)), this, SLOT(displayCropViewport()));
      menu->addMenu(displayMenu);
   

      QMenu* exploitationMenu = new QMenu("Exploitation Mode");
      QAction* registrationAction = exploitationMenu->addAction("Registration");
      QAction* geoPositioningAction = exploitationMenu->addAction("Geo Positioning");
      QAction* mensurationAction = exploitationMenu->addAction("Mensuration");
      menu->addMenu(exploitationMenu);
      connect(registrationAction, SIGNAL(triggered(bool)), this, SLOT(registrationExploitationSelected()));
      connect(geoPositioningAction, SIGNAL(triggered(bool)), this, SLOT(geoPositioningExploitationSelected()));
      connect(mensurationAction, SIGNAL(triggered(bool)), this, SLOT(mensurationExploitationSelected()));

     
      // Multi-image sync right-click
      QAction* miSync = menu->addAction("Sync All to Selected");
      connect(miSync, SIGNAL(triggered(bool)), this, SLOT(miSync()));
   }
   else if(dynamic_cast<DataManagerImageFilterFolder*>(activeItem))
   {
      unselectAll();
      selection.clear();
      selection.push_back(activeItem);
      activeItem->setSelected(true);
      QAction* removeAllFilters = menu->addAction("Remove All Filters");
      QAction* addFilterFront = menu->addAction("Add Filter To Front");
      QAction* addFilterEnd   = menu->addAction("Add Filter To End");
      connect(removeAllFilters, SIGNAL(triggered(bool)), this, SLOT(removeAllFilters()));
      connect(addFilterFront, SIGNAL(triggered(bool)), this, SLOT(addFilterToFront()));
      connect(addFilterEnd, SIGNAL(triggered(bool)), this, SLOT(addFilterToEnd()));
   }
   else if(dynamic_cast<DataManagerImageFilterItem*> (activeItem))
   {
      unselectAll();
      selection.clear();
      selection.push_back(activeItem);
      activeItem->setSelected(true);
      QAction* insertFilterBef = menu->addAction("Insert Filter Before");
      QAction* insertFilterAft  = menu->addAction("Insert Filter After");
      QAction* deleteFilterItem   = menu->addAction("Remove Filter");
      connect(insertFilterBef, SIGNAL(triggered(bool)), this, SLOT(insertFilterBefore()));
      connect(insertFilterAft, SIGNAL(triggered(bool)), this, SLOT(insertFilterAfter()));
      connect(deleteFilterItem, SIGNAL(triggered(bool)), this, SLOT(removeFilter()));
   }
   QList<DataManagerItem*>::iterator iter = selection.begin();
   while(iter != selection.end())
   {
      if((*iter)->itemAs<DataManagerImageChainItem>())
      {
         hasItems = true;
         ++nImageChainSelections;
      }
      else if((*iter)->itemAs<DataManagerRawImageSourceItem>())
      {
         hasItems = true;
        ++nRawSourceSelections;
      }
      else if((*iter)->itemAs<DataManagerInputConnectionItem>())
      {
         hasItems = true;
         ++nInputConnections;
      }
      else if((*iter)->itemAs<DataManagerDisplayItem>())
      {
         hasItems = true;
         ++nDisplayItems;
      }
      ++iter;
   }
   if(hasItems)
   {
      QAction* showAction = menu->addAction("Show");
      connect(showAction, SIGNAL(triggered(bool)), this, SLOT(showSelected()));
      
     if(nImageChainSelections>0||nRawSourceSelections>0)
     {
        QMenu* registrationMenu = new QMenu("Registration");
        QAction* fixedRegistrationAction =
           registrationMenu->addAction("Default Fixed");
        QAction* bundleRegistrationAction =
           registrationMenu->addAction("Default Bundle All-Floating");
#ifndef OSSIM_REGISTRATION_SOURCE_ENABLED
        fixedRegistrationAction->setEnabled(false);
        bundleRegistrationAction->setEnabled(false);
#endif
        menu->addMenu(registrationMenu);
        connect(fixedRegistrationAction,
                SIGNAL(triggered(bool)),
                this,
                SLOT(createFixedRegistrationFromSelection()));
        connect(bundleRegistrationAction,
                SIGNAL(triggered(bool)),
                this,
                SLOT(createBundleFloatingRegistrationFromSelection()));

        if(nRawSourceSelections>0)
        {
           QMenu* chainMenu = new QMenu("Chains");
           
           QAction* defaultChain = chainMenu->addAction("Default");
           QAction* affineChain = chainMenu->addAction("Affine");
           QAction* projectedChain = chainMenu->addAction("Map Projection");
           QAction* normalsChain = chainMenu->addAction("Image Normals");

           menu->addMenu(chainMenu);
           
           QMenu* stageMenu =new QMenu("Stage");
           QAction* buildOverviewsAction  = stageMenu->addAction("Build Default Overviews");
           std::vector<ossimString> typeList;
           ossimOverviewBuilderFactoryRegistry::instance()->getTypeNameList(typeList);
           ossim_uint32 idx = 0;
           if(!typeList.empty())
           {
              QMenu* overviewTypeMenu =new QMenu("Build Overviews");
              for(idx = 0; idx < typeList.size();++idx)
              {
                 // QAction* action = overviewTypeMenu->addAction(typeList[idx].c_str());
                 overviewTypeMenu->addAction(typeList[idx].c_str());
              }
              stageMenu->addMenu(overviewTypeMenu);
              connect(overviewTypeMenu, SIGNAL(triggered(QAction*)), this, SLOT(buildOverviewsForSelected(QAction*)));
           }
           QAction* createFullHistogramAction = stageMenu->addAction("Build Full Histograms");
           QAction* createFastHistogramAction = stageMenu->addAction("Build Fast Histograms");
           QAction* scanMinMaxAction      = stageMenu->addAction("Scan Min Max");
           // QAction* buildAction           = menu->addMenu(stageMenu);
           menu->addMenu(stageMenu);
           scanMinMaxAction->setEnabled(false);
           createFastHistogramAction->setEnabled(false);
           createFullHistogramAction->setEnabled(false);
           
           
           connect(buildOverviewsAction, SIGNAL(triggered(bool)), this, SLOT(buildOverviewsForSelected()));
           connect(createFullHistogramAction, SIGNAL(triggered(bool)), this, SLOT(createFullHistogramsForSelected()));
           connect(createFastHistogramAction, SIGNAL(triggered(bool)), this, SLOT(createFastHistogramsForSelected()));
           
           connect(defaultChain, SIGNAL(triggered(bool)), this, SLOT(createDefaultChain()));
           connect(affineChain, SIGNAL(triggered(bool)), this, SLOT(createAffineChain()));
           connect(projectedChain, SIGNAL(triggered(bool)), this, SLOT(createMapProjectedChain()));
           connect(normalsChain, SIGNAL(triggered(bool)), this, SLOT(createImageNormalsChain()));
        }
        QAction* exportAction = menu->addAction("Export");
        connect(exportAction, SIGNAL(triggered(bool)), this, SLOT(exportSelected()));
        QAction* swipeAction = menu->addAction("Swipe");
        connect(swipeAction, SIGNAL(triggered(bool)), this, SLOT(swipeSelected()));
     }
      QAction* deleteAction = menu->addAction("Delete");
      connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(deleteSelected()));
   
      
      if(nImageChainSelections > 0)
      {
         QMenu* combineMenu =new QMenu("Combine");
         QAction* aoverbAction  = combineMenu->addAction("A over B");
         QAction* blendAction   = combineMenu->addAction("Blend");
         QAction* featherAction = combineMenu->addAction("Feather");
         QAction* hillShadeAction = combineMenu->addAction("Hill Shade");
         QAction* combinerFactoryAction = combineMenu->addAction("Select from factory");            
         // QAction* combine = menu->addMenu(combineMenu);
         menu->addMenu(combineMenu);
         
         QMenu* viewMenu = new QMenu("View");
         QAction* geographicAction       = viewMenu->addAction("Geographic");
         QAction* scaledGeographicAction = viewMenu->addAction("Scaled Geographic");
         QAction* utmAction              = viewMenu->addAction("Utm");
         // QAction* view = menu->addMenu(viewMenu);
         menu->addMenu(viewMenu);
         
         connect(aoverbAction, SIGNAL(triggered(bool)), this, SLOT(aOverBMosaic()));
         connect(blendAction, SIGNAL(triggered(bool)), this, SLOT(blendMosaic()));
         connect(featherAction, SIGNAL(triggered(bool)), this, SLOT(featherMosaic()));
         connect(hillShadeAction, SIGNAL(triggered(bool)), this, SLOT(hillShadeCombiner()));
         connect(combinerFactoryAction, SIGNAL(triggered(bool)), this, SLOT(factoryCombiner()));
         
         connect(geographicAction, SIGNAL(triggered(bool)), this, SLOT(geographicView()));
         connect(scaledGeographicAction, SIGNAL(triggered(bool)), this, SLOT(scaledGeographicView()));
         connect(utmAction, SIGNAL(triggered(bool)), this, SLOT(utmView()));
      }
   }

#ifdef OSSIM_PLANET_ENABLED
   QAction* planetaryView = menu->addAction("Planetary View");
   connect(planetaryView, SIGNAL(triggered(bool)), this, SLOT(planetaryView()));
#endif
   return menu;
}


// Multi-image dialog
void ossimGui::DataManagerWidget::miDialog(const int& mode)
{
   ossim_uint32 numImages;

   if(!m_miDialog)
   {
//   }
//   if(!m_miDialog->isActive())
//   {
      // Attempt to instantiate the auto measurement generator
      // ossimRefPtr<ossimObject> obj =
      m_tGenObj = ossimObjectFactoryRegistry::instance()->
         createObject(ossimString("ossimTieMeasurementGenerator"));
      if(m_tGenObj.valid())
      {
         m_tGen = dynamic_cast<ossimTieMeasurementGeneratorInterface*> (m_tGenObj.get());
         if (m_tGen)
         {   
            m_amDialogAvailable = true;
         }
      }

      // Populate the nodeList
      QList<DataManagerDisplayItem*> result = grabSelectedChildItemsOfType<DataManagerDisplayItem>();
      numImages = result.size();

      DataManager::NodeListType nodeList;

      for(ossim_uint32 idx = 0; idx < numImages; ++idx)
      {
         DataManager::Node* node = result[idx]->objectAsNode();
         nodeList.push_back(node);
      }

      // Check for required open displays
      bool displaysOK = true;
      for(ossim_uint32 idx = 0; idx < nodeList.size(); ++idx)
      {
         ConnectableDisplayObject* displayObj = nodeList[idx]->getObjectAs<ConnectableDisplayObject>();
         ImageMdiSubWindow* subWindow = dynamic_cast<ImageMdiSubWindow*>(displayObj->display());
         if(subWindow==NULL)
         {
            displaysOK = false;
         }
      }

      if (displaysOK)
      {
         m_miDialog = new MultiImageDialog(this);
         m_miDialog->initContent(nodeList, m_amDialogAvailable);

         // Establish connections from MultiImageDialog to DataManager
         connect(m_miDialog, SIGNAL(registrationExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miReg(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(autoMeasInitiated(DataManager::NodeListType&)),
                 this,       SLOT(miAutoMeas(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(pointDropExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miDrop(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(syncExecuted(ossimGui::RegPoint*, ossimRefPtr<DataManager::Node>)),
                 this,       SLOT(miSync(ossimGui::RegPoint*, ossimRefPtr<DataManager::Node>)));
         connect(m_miDialog, SIGNAL(resetModeExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miResetMode(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(clearPointExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miClearCurrentPoint(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(acceptRegExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miAcceptReg(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(resetRegExecuted(DataManager::NodeListType&)),
                 this,       SLOT(miResetReg(DataManager::NodeListType&)));
         connect(m_miDialog, SIGNAL(destroyed()), this, SLOT(miDialogDestroyed()));


         m_miDialog->setMode(mode);
         m_dataManager->setDialog(m_miDialog);
        // Check minimums
         DataManager::ExploitationModeType expMode = static_cast<DataManager::ExploitationModeType> (mode);
         if (expMode != DataManager::NO_MODE)
         {
            bool okToShow = false;
            if (expMode == DataManager::REGISTRATION_MODE && numImages>1)
            {
               okToShow = true;
            }
            else if (expMode == DataManager::GEOPOSITIONING_MODE && numImages>0)
            {
               okToShow = true;
            }
            else if (expMode == DataManager::MENSURATION_MODE && numImages>0)
            {
               okToShow = true;
            }

            if (okToShow)
            {
               m_miDialog->setWindowFlags(m_miDialog->windowFlags() | Qt::WindowStaysOnTopHint);
               m_miDialog->show();
            }
            else
            {
               QString text("Minimum number of images required...");
               QString geoMin("\n  - Geopositioning: 1 image");
               QString regMin("\n  - Registration: 2 images");
               QString menMin("\n  - Mensuration: 1 image");
               text += geoMin;
               text += regMin;
               text += menMin;
               QMessageBox::critical(this, "ERROR", text);
               m_miDialog->close();
            }
         }
      }
      else
      {
         QString text("First select 'Show' on all selected displays...");
         QMessageBox::critical(this, "ERROR", text);
      }
   }
   else
   {
      m_miDialog->show();  
   }
}


// Auto measurement dialog
void ossimGui::DataManagerWidget::amDialog(DataManager::NodeListType& nodes)
{

   m_amDialog = new AutoMeasurementDialog(this, nodes, m_tGen);

   // Establish connections between AutoMeasurementDialog and DataManager
   connect(m_amDialog, SIGNAL(acceptMeasExecuted(DataManager::NodeListType&)),
           this,       SLOT(miAcceptMeas(DataManager::NodeListType&)));
   connect(m_amDialog, SIGNAL(dismissMeasExecuted()),
           this,       SLOT(miDismissMeas()));

   m_amDialog->setWindowFlags(m_amDialog->windowFlags() | Qt::WindowStaysOnTopHint);
   m_amDialog->show();

   // Set active flag
   m_dataManager->setAutoMeasActive(true);
}


void ossimGui::DataManagerWidget::miDialogDestroyed()
{
   m_miDialog = 0;
   m_dataManager->setDialog(0);
}

// Accept auto measurement
bool ossimGui::DataManagerWidget::miAcceptMeas(DataManager::NodeListType& nodes)
{
   bool measAccOK = false;

   if(m_dataManager.valid())
   {
      measAccOK = m_dataManager->setAutoMeasureResults(nodes, m_tGen);
   }

   if(m_miDialog) m_miDialog->show();
   miDismissMeas();

   return measAccOK;
}


// Reset auto measurement
bool ossimGui::DataManagerWidget::miDismissMeas()
{
   bool dismissMeasOK = true;

   delete m_amDialog;
   m_amDialog = 0;
   m_amDialogActive = false;
   if(m_miDialog) m_miDialog->show();

   // Set active flag
   m_dataManager->setAutoMeasActive(false);

   return dismissMeasOK;
}


// Multi-image auto measurement button
bool ossimGui::DataManagerWidget::miAutoMeas(DataManager::NodeListType& nodes)
{
   // bool autoMeasOK = false;
   bool autoMeasOK = true;

   // Open AutoMeasurmentDialog
   amDialog(nodes);
   m_amDialogActive = true;

   // Hide multi-image window during operation
   if(m_miDialog) m_miDialog->hide();

   // return true;

   return autoMeasOK;
}


// Multi-image syncronization
void ossimGui::DataManagerWidget::miSync(ossimGui::RegPoint* syncPt, ossimRefPtr<DataManager::Node> node)
{
   if (node)
   {
      ossimDpt sPt(syncPt->x(), syncPt->y());
      m_dataManager->syncImagesTo(sPt, node);
   }
}


// Multi-image syncronization
void ossimGui::DataManagerWidget::miSync()
{
   QList<DataManagerDisplayItem*> result = grabSelectedChildItemsOfType<DataManagerDisplayItem>();

   // Sync source is one image, so use result[0]
   if(m_dataManager.valid())
   {
      DataManager::Node* node = result[0]->objectAsNode();
      if (node)
      {
         m_dataManager->syncImagesTo(node);
      }
   }
}


// Multi-image intersection
bool ossimGui::DataManagerWidget::miDrop(DataManager::NodeListType& nodes)
{
   bool dropOK = false;

   if(m_dataManager.valid())
   {
      dropOK = m_dataManager->intersectRays(nodes);
   }

   return dropOK;
}


// Multi-image registration
bool ossimGui::DataManagerWidget::miReg(DataManager::NodeListType& nodes)
{
   bool regOK = false;

   if(m_dataManager.valid())
   {
      regOK = m_dataManager->registerImages(nodes);
   }

   return regOK;
}


// Multi-image registration accept
bool ossimGui::DataManagerWidget::miAcceptReg(DataManager::NodeListType& nodes)
{
   bool regAccOK = false;

   if(m_dataManager.valid())
   {
      regAccOK = m_dataManager->saveImageGeometries(nodes);
   }

   miSync();

   return regAccOK;
}


// Multi-image registration reject
bool ossimGui::DataManagerWidget::miResetReg(DataManager::NodeListType& nodes)
{
   bool resetRegOK = false;

   if(m_dataManager.valid())
   {
      resetRegOK = m_dataManager->resetAdj(nodes);
   }

   return resetRegOK;
}


// Multi-image mode reset
bool ossimGui::DataManagerWidget::miResetMode(DataManager::NodeListType& nodes)
{
   //std::cout << "ossimGui::DataManagerWidget::miResetMode: ................entered!!\n";
   bool resetModeOK = false;

   if(m_dataManager.valid())
   {
      resetModeOK = m_dataManager->resetMode(nodes);
   }

   emit resetMode();

   return resetModeOK;
}


// Multi-image geopositioning clear current point measurements
bool ossimGui::DataManagerWidget::miClearCurrentPoint(DataManager::NodeListType& nodes)
{
   bool clearOK = false;

   if(m_dataManager.valid())
   {
      clearOK = m_dataManager->clearCurrentPoint(nodes);
   }

   return clearOK;
}


ossimGui::DataManager::NodeListType ossimGui::DataManagerWidget::getSelectedNodeList()
{
   QList<DataManagerDisplayItem*> result = grabSelectedChildItemsOfType<DataManagerDisplayItem>();

   DataManager::NodeListType nodes;

   if(m_dataManager.valid())
   {
      if(result.size() > 1)
      {
         QList<DataManagerDisplayItem*>::iterator iter = result.begin();

         while(iter != result.end())
         {
            DataManager::Node* node = (*iter)->objectAsNode();
            if(node)
            {
               nodes.push_back(node);
            }
            ++iter;
         }
      }
   }

   return nodes;
}
