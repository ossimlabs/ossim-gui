#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include "RegistrationSetupDialog.h"

#include <ossim/base/ossimString.h>
#include <ossim_autoreg/BundleLinearSolverFactory.h>
#include <ossim_autoreg/TiePointGenerator.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace ossimGui
{
   bool parseRegistrationInputIndex(const std::string& text,
                                    std::size_t& value)
   {
      if(text.empty())
         return false;
      char* end = 0;
      const unsigned long long parsed = std::strtoull(text.c_str(), &end, 10);
      if(!end || *end)
         return false;
      value = static_cast<std::size_t>(parsed);
      return true;
   }

   bool parseRegistrationInputIndexes(
      const std::string& text,
      std::vector<ossim_uint32>& indexes)
   {
      indexes.clear();
      if(text.empty())
         return true;
      std::istringstream in(text);
      std::string token;
      while(std::getline(in, token, ','))
      {
         ossimString trimmed(token);
         trimmed.trim();
         std::size_t value = 0;
         if(!parseRegistrationInputIndex(trimmed.string(), value))
            return false;
         const ossim_uint32 index = static_cast<ossim_uint32>(value);
         if(std::find(indexes.begin(), indexes.end(), index) == indexes.end())
            indexes.push_back(index);
      }
      return true;
   }

   bool parseRegistrationInputPairs(
      const std::string& text,
      std::vector<ossim_autoreg::BundleImagePair>& pairs)
   {
      pairs.clear();
      if(text.empty())
         return true;
      std::istringstream in(text);
      std::string token;
      while(std::getline(in, token, ';'))
      {
         const std::size_t comma = token.find(',');
         if(comma == std::string::npos || token.find(',', comma + 1) != std::string::npos)
            return false;
         ossimString firstText(token.substr(0, comma));
         ossimString secondText(token.substr(comma + 1));
         firstText.trim();
         secondText.trim();
         std::size_t first = 0;
         std::size_t second = 0;
         if(!parseRegistrationInputIndex(firstText.string(), first) ||
            !parseRegistrationInputIndex(secondText.string(), second) ||
            first == second)
         {
            return false;
         }
         if(second < first)
            std::swap(first, second);
         const auto duplicate = std::find_if(
            pairs.begin(), pairs.end(),
            [first, second](const ossim_autoreg::BundleImagePair& pair) {
               return pair.firstImageIndex() == first &&
                      pair.secondImageIndex() == second;
            });
         if(duplicate == pairs.end())
            pairs.push_back(ossim_autoreg::BundleImagePair(first, second));
      }
      std::sort(pairs.begin(), pairs.end(),
                [](const ossim_autoreg::BundleImagePair& lhs,
                   const ossim_autoreg::BundleImagePair& rhs) {
                   if(lhs.firstImageIndex() != rhs.firstImageIndex())
                      return lhs.firstImageIndex() < rhs.firstImageIndex();
                   return lhs.secondImageIndex() < rhs.secondImageIndex();
                });
      return true;
   }

   QString formatRegistrationInputIndexes(
      const std::vector<ossim_uint32>& indexes)
   {
      QStringList values;
      for(ossim_uint32 index : indexes)
         values.push_back(QString::number(index));
      return values.join(",");
   }

   QString formatRegistrationInputPairs(
      const std::vector<ossim_autoreg::BundleImagePair>& pairs)
   {
      QStringList values;
      for(const ossim_autoreg::BundleImagePair& pair : pairs)
      {
         values.push_back(QString("%1,%2")
                             .arg(pair.firstImageIndex())
                             .arg(pair.secondImageIndex()));
      }
      return values.join(";");
   }

   class RegistrationSetupDialog : public QDialog
   {
   public:
      RegistrationSetupDialog(QWidget* parent = 0)
      : QDialog(parent),
        m_approach(0),
        m_matchMethod(0),
        m_resampler(0),
        m_supportPassResampler(0),
        m_chipSize(0),
        m_searchRadius(0),
        m_gridSpacing(0),
        m_minScore(0),
        m_minScoreMargin(0),
        m_viewGsd(0),
        m_maxTiePoints(0),
        m_denseGridSeedBudget(0),
        m_autoDenseGridSeedBudget(0),
        m_tiePointTimingDiagnostics(0),
        m_bundlePairPolicy(0),
        m_bundleNeighborSpan(0),
        m_bundleAnchorInputIndexes(0),
        m_bundleInputPairs(0),
        m_bundleLinearSolver(0),
        m_maxConcurrentRegistrations(0),
        m_adaptiveBankThreadCount(0),
        m_adaptiveFullPostBankRefinement(0),
        m_nativeLowGridPolicy(0),
        m_opencvRansacPrefilter(0),
        m_opencvRansacThresholdPixels(0)
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

         const std::vector<ossim_autoreg::RegistrationComponentDescriptor>
            matcherTypes = ossim_autoreg::TiePointGeneratorFactory::instance()->
               typeDescriptors();
         for(const ossim_autoreg::RegistrationComponentDescriptor& matcherType :
             matcherTypes)
         {
            const QString typeName =
               QString::fromStdString(matcherType.typeName());
            const QString displayName = QString::fromStdString(
               matcherType.displayName().empty() ? matcherType.typeName() :
                                                   matcherType.displayName());
            addMatchMethod(displayName, typeName);
            const int itemIndex = m_matchMethod->count() - 1;
            if(!matcherType.description().empty())
            {
               m_matchMethod->setItemData(
                  itemIndex,
                  QString::fromStdString(matcherType.description()),
                  Qt::ToolTipRole);
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

         m_supportPassResampler = new QComboBox(this);
         m_supportPassResampler->addItem("default", "");
         m_supportPassResampler->addItem("cubic", "cubic");
         m_supportPassResampler->addItem("bilinear", "bilinear");
         m_supportPassResampler->addItem("nearest", "nearest_neighbor");
         m_supportPassResampler->addItem("sinc", "sinc");
         m_supportPassResampler->setToolTip(
            "Optional fixed-auto support-pass matcher resampler. "
            "Default keeps the main resampler.");

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

         m_minScoreMargin = new QDoubleSpinBox(this);
         m_minScoreMargin->setRange(0.0, 1.0);
         m_minScoreMargin->setDecimals(3);
         m_minScoreMargin->setSingleStep(0.01);
         m_minScoreMargin->setValue(0.03);
         m_minScoreMargin->setToolTip(
            "Minimum native-affine NCC peak separation. "
            "Use 0 to disable ambiguity filtering.");

         m_viewGsd = new QDoubleSpinBox(this);
         m_viewGsd->setRange(-100.0, 1000000.0);
         m_viewGsd->setDecimals(3);
         m_viewGsd->setSingleStep(0.25);
         m_viewGsd->setValue(0.0);

         m_maxTiePoints = new QSpinBox(this);
         m_maxTiePoints->setRange(0, 100000);
         m_maxTiePoints->setValue(300);

         m_denseGridSeedBudget = new QSpinBox(this);
         m_denseGridSeedBudget->setRange(0, 1000000);
         m_denseGridSeedBudget->setValue(0);

         m_autoDenseGridSeedBudget = new QCheckBox(this);
         m_autoDenseGridSeedBudget->setChecked(false);

         m_tiePointTimingDiagnostics = new QCheckBox(this);
         m_tiePointTimingDiagnostics->setChecked(false);
         m_tiePointTimingDiagnostics->setToolTip(
            "Collect renderer/tile timing counters for profiling. "
            "Leave off for faster normal registration.");

         m_bundlePairPolicy = new QComboBox(this);
         m_bundlePairPolicy->addItem("All pairs",
                                     BUNDLE_PAIR_POLICY_ALL_PAIRS);
         m_bundlePairPolicy->addItem("Neighbor span",
                                     BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN);
         m_bundlePairPolicy->addItem("Auto",
                                     BUNDLE_PAIR_POLICY_AUTO);
         m_bundlePairPolicy->addItem("Explicit pairs",
                                     BUNDLE_PAIR_POLICY_EXPLICIT);
         m_bundlePairPolicy->setToolTip(
            "Bundle only: choose whether to test every image pair or only "
            "nearby image-index neighbors. Auto promotes clear strip-like "
            "overlap chains and falls back to all pairs when needed.");

         m_bundleNeighborSpan = new QSpinBox(this);
         m_bundleNeighborSpan->setRange(1, 100000);
         m_bundleNeighborSpan->setValue(1);
         m_bundleNeighborSpan->setToolTip(
            "Bundle only: neighbor image-index distance. 1 tests adjacent "
            "pairs for strip-style datasets.");

         m_bundleAnchorInputIndexes = new QLineEdit(this);
         m_bundleAnchorInputIndexes->setPlaceholderText("0 or 0,2");
         m_bundleAnchorInputIndexes->setToolTip(
            "Bundle anchored mode only: comma-separated connected input "
            "indexes held fixed.");

         m_bundleInputPairs = new QLineEdit(this);
         m_bundleInputPairs->setPlaceholderText("0,1;1,2");
         m_bundleInputPairs->setToolTip(
            "Explicit-pairs mode only: semicolon-separated connected input "
            "index pairs.");

         m_bundleLinearSolver = new QComboBox(this);
         m_bundleLinearSolver->addItem("Auto (Recommended)", "auto");
         const std::vector<ossim_autoreg::RegistrationComponentDescriptor>
            solverTypes =
               ossim_autoreg::BundleLinearSolverFactory::instance()->
                  typeDescriptors();
         for(const ossim_autoreg::RegistrationComponentDescriptor& solverType :
             solverTypes)
         {
            const QString typeName =
               QString::fromStdString(solverType.typeName());
            const QString displayName = QString::fromStdString(
               solverType.displayName().empty() ? solverType.typeName() :
                                                  solverType.displayName());
            m_bundleLinearSolver->addItem(displayName, typeName);
            if(!solverType.description().empty())
            {
               m_bundleLinearSolver->setItemData(
                  m_bundleLinearSolver->count() - 1,
                  QString::fromStdString(solverType.description()),
                  Qt::ToolTipRole);
            }
         }

         m_maxConcurrentRegistrations = new QSpinBox(this);
         m_maxConcurrentRegistrations->setRange(1, 64);
         m_maxConcurrentRegistrations->setValue(1);

         m_adaptiveBankThreadCount = new QSpinBox(this);
         m_adaptiveBankThreadCount->setRange(0, 64);
         m_adaptiveBankThreadCount->setValue(4);

         m_adaptiveFullPostBankRefinement = new QCheckBox(this);
         m_adaptiveFullPostBankRefinement->setChecked(true);

         m_nativeLowGridPolicy = new QComboBox(this);
         m_nativeLowGridPolicy->addItem("Advisory", "advisory");
         m_nativeLowGridPolicy->addItem("Reject", "reject");
         m_nativeLowGridPolicy->setToolTip(
            "Handling for native image-space matches with low control-grid "
            "occupancy.");

         m_opencvRansacPrefilter = new QCheckBox(this);
         m_opencvRansacPrefilter->setChecked(true);
         m_opencvRansacPrefilter->setToolTip(
            "Use OpenCV affine RANSAC as a tie-point coherence prefilter.");

         m_opencvRansacThresholdPixels = new QDoubleSpinBox(this);
         m_opencvRansacThresholdPixels->setRange(0.0, 100000.0);
         m_opencvRansacThresholdPixels->setDecimals(2);
         m_opencvRansacThresholdPixels->setSingleStep(1.0);
         m_opencvRansacThresholdPixels->setValue(25.0);
         m_opencvRansacThresholdPixels->setToolTip(
            "RANSAC inlier threshold in pixels for the OpenCV affine "
            "prefilter.");

         connect(m_approach,
                 static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                 [this](int) { applySelectedDefaults(); });
         connect(m_matchMethod,
                 static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                 [this](int) { applySelectedDefaults(); });
         connect(m_bundlePairPolicy,
                 static_cast<void (QComboBox::*)(int)>(
                    &QComboBox::currentIndexChanged),
                 [this](int) { updateBundlePairPolicyControls(); });

         QFormLayout* form = new QFormLayout();
         form->addRow("Approach", m_approach);
         form->addRow("Matcher", m_matchMethod);
         form->addRow("Resampler", m_resampler);
         form->addRow("Support pass resampler", m_supportPassResampler);
         form->addRow("Chip size", m_chipSize);
         form->addRow("Search radius", m_searchRadius);
         form->addRow("Grid spacing", m_gridSpacing);
         form->addRow("Minimum score", m_minScore);
         form->addRow("Minimum score margin", m_minScoreMargin);
         form->addRow("View GSD", m_viewGsd);
         form->addRow("Max ties", m_maxTiePoints);
         form->addRow("Dense seed budget", m_denseGridSeedBudget);
         form->addRow("Auto dense seed budget",
                      m_autoDenseGridSeedBudget);
         form->addRow("Tie timing diagnostics",
                      m_tiePointTimingDiagnostics);
         form->addRow("Bundle pair policy",
                      m_bundlePairPolicy);
         form->addRow("Bundle neighbor span",
                      m_bundleNeighborSpan);
         form->addRow("Bundle anchor input indexes",
                      m_bundleAnchorInputIndexes);
         form->addRow("Bundle explicit input pairs",
                      m_bundleInputPairs);
         form->addRow("Bundle linear solver",
                      m_bundleLinearSolver);
         form->addRow("Parallel floating inputs",
                      m_maxConcurrentRegistrations);
         form->addRow("Adaptive bank threads",
                      m_adaptiveBankThreadCount);
         form->addRow("Full post-bank refinement",
                      m_adaptiveFullPostBankRefinement);
         form->addRow("Native low-grid policy",
                      m_nativeLowGridPolicy);
         form->addRow("OpenCV RANSAC prefilter",
                      m_opencvRansacPrefilter);
         form->addRow("OpenCV RANSAC threshold",
                      m_opencvRansacThresholdPixels);

         QGroupBox* optionsBox = new QGroupBox("Options", this);
         optionsBox->setLayout(form);

         QDialogButtonBox* buttons =
            new QDialogButtonBox(QDialogButtonBox::Ok |
                                 QDialogButtonBox::Cancel,
                                 Qt::Horizontal,
                                 this);
         connect(buttons, &QDialogButtonBox::accepted, [this]() {
            if(validateBundleControls())
               accept();
         });
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
         result.supportPassMatcherResampler =
            m_supportPassResampler->itemData(
               m_supportPassResampler->currentIndex()).toString().toStdString();
         result.chipSize = m_chipSize->value();
         if((result.chipSize % 2) == 0)
            ++result.chipSize;
         result.searchRadius = m_searchRadius->value();
         result.gridSpacing = m_gridSpacing->value();
         result.minScore = m_minScore->value();
         result.minScoreMargin = m_minScoreMargin->value();
         result.viewGsd = m_viewGsd->value();
         result.maxTiePoints =
            static_cast<std::size_t>(m_maxTiePoints->value());
         result.denseGridSeedBudget =
            static_cast<std::size_t>(
               m_denseGridSeedBudget->value());
         result.autoDenseGridSeedBudget =
            m_autoDenseGridSeedBudget->isChecked();
         result.tiePointTimingDiagnostics =
            m_tiePointTimingDiagnostics->isChecked();
         result.bundlePairPolicy =
            static_cast<BundlePairPolicy>(
               m_bundlePairPolicy->itemData(
                  m_bundlePairPolicy->currentIndex()).toInt());
         result.bundleNeighborSpan =
            result.bundlePairPolicy == BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN ?
               static_cast<std::size_t>(m_bundleNeighborSpan->value()) :
               0;
         parseRegistrationInputIndexes(
            m_bundleAnchorInputIndexes->text().trimmed().toStdString(),
            result.bundleAnchorInputIndexes);
         parseRegistrationInputPairs(
            m_bundleInputPairs->text().trimmed().toStdString(),
            result.bundleInputPairs);
         result.bundleLinearSolverType =
            m_bundleLinearSolver->itemData(
               m_bundleLinearSolver->currentIndex()).toString().toStdString();
         result.maxConcurrentRegistrations =
            static_cast<std::size_t>(
               m_maxConcurrentRegistrations->value());
         result.adaptiveBankThreadCount =
            static_cast<std::size_t>(
               m_adaptiveBankThreadCount->value());
         result.adaptiveFullPostBankRefinement =
            m_adaptiveFullPostBankRefinement->isChecked();
         result.nativeLowGridPolicy =
            m_nativeLowGridPolicy->itemData(
               m_nativeLowGridPolicy->currentIndex()).toString().toStdString();
         result.opencvRansacPrefilter =
            m_opencvRansacPrefilter->isChecked();
         result.opencvRansacThresholdPixels =
            m_opencvRansacThresholdPixels->value();
         return result;
      }

   private:
      bool validateBundleControls()
      {
         const RegistrationSetupApproach approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         const bool bundle =
            approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING ||
            approach == REGISTRATION_SETUP_BUNDLE_ANCHORED;
         if(!bundle)
            return true;

         std::vector<ossim_uint32> anchors;
         if(!parseRegistrationInputIndexes(
               m_bundleAnchorInputIndexes->text().trimmed().toStdString(),
               anchors) ||
            (approach == REGISTRATION_SETUP_BUNDLE_ANCHORED &&
             anchors.empty()))
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "Anchor indexes must be a comma-separated list such as 0 or "
               "0,2. Anchored mode requires at least one index.");
            return false;
         }

         const bool explicitPairs =
            m_bundlePairPolicy->itemData(
               m_bundlePairPolicy->currentIndex()).toInt() ==
            BUNDLE_PAIR_POLICY_EXPLICIT;
         std::vector<ossim_autoreg::BundleImagePair> pairs;
         if(!parseRegistrationInputPairs(
               m_bundleInputPairs->text().trimmed().toStdString(), pairs) ||
            (explicitPairs && pairs.empty()))
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "Explicit pairs must use semicolon-separated input-index "
               "pairs such as 0,1;1,2.");
            return false;
         }
         return true;
      }

      void applySelectedDefaults()
      {
         const RegistrationSetupApproach approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         const std::string matchMethod =
            m_matchMethod->itemData(m_matchMethod->currentIndex()).
               toString().toStdString();
         const RegistrationSetupOptions defaults =
            ossim_autoreg::registrationSetupDefaults(approach, matchMethod);

         const int resamplerIndex =
            m_resampler->findData(QString::fromStdString(
               defaults.resamplerType));
         if(resamplerIndex >= 0)
            m_resampler->setCurrentIndex(resamplerIndex);
         const int supportPassResamplerIndex =
            m_supportPassResampler->findData(QString::fromStdString(
               defaults.supportPassMatcherResampler));
         if(supportPassResamplerIndex >= 0)
            m_supportPassResampler->setCurrentIndex(
               supportPassResamplerIndex);
         m_chipSize->setValue(defaults.chipSize);
         m_searchRadius->setValue(defaults.searchRadius);
         m_gridSpacing->setValue(defaults.gridSpacing);
         m_minScore->setValue(defaults.minScore);
         m_minScoreMargin->setValue(defaults.minScoreMargin);
         m_viewGsd->setValue(defaults.viewGsd);
         m_maxTiePoints->setValue(
            static_cast<int>(defaults.maxTiePoints));
         m_denseGridSeedBudget->setValue(
            static_cast<int>(defaults.denseGridSeedBudget));
         m_autoDenseGridSeedBudget->setChecked(
            defaults.autoDenseGridSeedBudget);
         m_tiePointTimingDiagnostics->setChecked(
            defaults.tiePointTimingDiagnostics);
         {
            const int pairPolicyIndex =
               m_bundlePairPolicy->findData(
                  defaults.bundleNeighborSpan ?
                     BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN :
                     BUNDLE_PAIR_POLICY_ALL_PAIRS);
            if(pairPolicyIndex >= 0)
               m_bundlePairPolicy->setCurrentIndex(pairPolicyIndex);
         }
         m_bundleNeighborSpan->setValue(
            static_cast<int>(
               defaults.bundleNeighborSpan ?
                  defaults.bundleNeighborSpan :
                  1));
         m_bundleAnchorInputIndexes->setText(
            formatRegistrationInputIndexes(
               defaults.bundleAnchorInputIndexes));
         m_bundleInputPairs->setText(
            formatRegistrationInputPairs(defaults.bundleInputPairs));
         const int solverIndex = m_bundleLinearSolver->findData(
            QString::fromStdString(defaults.bundleLinearSolverType));
         if(solverIndex >= 0)
            m_bundleLinearSolver->setCurrentIndex(solverIndex);
         updateBundlePairPolicyControls();
         m_maxConcurrentRegistrations->setValue(
            static_cast<int>(defaults.maxConcurrentRegistrations));
         m_adaptiveBankThreadCount->setValue(
            static_cast<int>(defaults.adaptiveBankThreadCount));
         m_adaptiveFullPostBankRefinement->setChecked(
            defaults.adaptiveFullPostBankRefinement);
         {
            const int nativeLowGridIndex =
               m_nativeLowGridPolicy->findData(QString::fromStdString(
                  defaults.nativeLowGridPolicy));
            if(nativeLowGridIndex >= 0)
               m_nativeLowGridPolicy->setCurrentIndex(nativeLowGridIndex);
         }
         m_opencvRansacPrefilter->setChecked(
            defaults.opencvRansacPrefilter);
         m_opencvRansacThresholdPixels->setValue(
            defaults.opencvRansacThresholdPixels);
      }

      void addMatchMethod(const QString& label, const QString& method)
      {
         m_matchMethod->addItem(label, method);
      }

      void updateBundlePairPolicyControls()
      {
         const RegistrationSetupApproach approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         const bool bundle =
            approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING ||
            approach == REGISTRATION_SETUP_BUNDLE_ANCHORED;
         const bool neighborSpan =
            m_bundlePairPolicy->itemData(
               m_bundlePairPolicy->currentIndex()).toInt() ==
            BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN;
         const bool explicitPairs =
            m_bundlePairPolicy->itemData(
               m_bundlePairPolicy->currentIndex()).toInt() ==
            BUNDLE_PAIR_POLICY_EXPLICIT;
         m_bundlePairPolicy->setEnabled(bundle);
         m_bundleNeighborSpan->setEnabled(bundle && neighborSpan);
         m_bundleAnchorInputIndexes->setEnabled(
            approach == REGISTRATION_SETUP_BUNDLE_ANCHORED);
         m_bundleInputPairs->setEnabled(bundle && explicitPairs);
         m_bundleLinearSolver->setEnabled(bundle);
      }

      QComboBox* m_approach;
      QComboBox* m_matchMethod;
      QComboBox* m_resampler;
      QComboBox* m_supportPassResampler;
      QSpinBox* m_chipSize;
      QSpinBox* m_searchRadius;
      QSpinBox* m_gridSpacing;
      QDoubleSpinBox* m_minScore;
      QDoubleSpinBox* m_minScoreMargin;
      QDoubleSpinBox* m_viewGsd;
      QSpinBox* m_maxTiePoints;
      QSpinBox* m_denseGridSeedBudget;
      QCheckBox* m_autoDenseGridSeedBudget;
      QCheckBox* m_tiePointTimingDiagnostics;
      QComboBox* m_bundlePairPolicy;
      QSpinBox* m_bundleNeighborSpan;
      QLineEdit* m_bundleAnchorInputIndexes;
      QLineEdit* m_bundleInputPairs;
      QComboBox* m_bundleLinearSolver;
      QSpinBox* m_maxConcurrentRegistrations;
      QSpinBox* m_adaptiveBankThreadCount;
      QCheckBox* m_adaptiveFullPostBankRefinement;
      QComboBox* m_nativeLowGridPolicy;
      QCheckBox* m_opencvRansacPrefilter;
      QDoubleSpinBox* m_opencvRansacThresholdPixels;
   };

   bool promptForRegistrationSetup(
      QWidget* parent,
      RegistrationSetupOptions& options)
   {
      RegistrationSetupDialog dialog(parent);
      if(dialog.exec() != QDialog::Accepted)
         return false;
      options = dialog.options();
      return true;
   }

}

#endif
