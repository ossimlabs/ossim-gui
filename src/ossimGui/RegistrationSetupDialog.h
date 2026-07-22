#ifndef ossimGui_RegistrationSetupDialog_HEADER
#define ossimGui_RegistrationSetupDialog_HEADER

#include <ossim/base/ossimConstants.h>
#include <ossim_autoreg/AutoRegistration.h>

#include <cstddef>
#include <string>
#include <vector>

class QWidget;

namespace ossim_autoreg
{
   class TiePointGenerationOptions;
}

namespace ossimGui
{
   enum RegistrationSetupApproach
   {
      REGISTRATION_SETUP_FIXED_AUTO = 0,
      REGISTRATION_SETUP_FIXED_MANUAL = 1,
      REGISTRATION_SETUP_BUNDLE_ALL_FLOATING = 2,
      REGISTRATION_SETUP_BUNDLE_ANCHORED = 3
   };

   enum BundlePairPolicy
   {
      BUNDLE_PAIR_POLICY_ALL_PAIRS = 0,
      BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN = 1,
      BUNDLE_PAIR_POLICY_AUTO = 2,
      BUNDLE_PAIR_POLICY_EXPLICIT = 3
   };

   struct RegistrationSetupOptions
   {
      RegistrationSetupApproach approach;
      std::string matchMethod;
      std::string resamplerType;
      std::string supportPassMatcherResampler;
      int chipSize;
      int searchRadius;
      int gridSpacing;
      double minScore;
      double minScoreMargin;
      double viewGsd;
      std::size_t maxTiePoints;
      std::size_t denseGridSeedBudget;
      bool autoDenseGridSeedBudget;
      bool tiePointTimingDiagnostics;
      BundlePairPolicy bundlePairPolicy;
      std::size_t bundleNeighborSpan;
      std::vector<ossim_uint32> bundleAnchorInputIndexes;
      std::vector<ossim_autoreg::BundleImagePair> bundleInputPairs;
      std::string bundleLinearSolverType;
      std::size_t maxConcurrentRegistrations;
      std::size_t adaptiveBankThreadCount;
      bool adaptiveFullPostBankRefinement;
      std::string nativeLowGridPolicy;
      bool opencvRansacPrefilter;
      double opencvRansacThresholdPixels;

      RegistrationSetupOptions()
      : approach(REGISTRATION_SETUP_FIXED_AUTO),
        matchMethod(),
        resamplerType("cubic"),
        supportPassMatcherResampler(),
        chipSize(31),
        searchRadius(64),
        gridSpacing(128),
        minScore(0.6),
        minScoreMargin(0.03),
        viewGsd(0.0),
        maxTiePoints(300),
        denseGridSeedBudget(0),
        autoDenseGridSeedBudget(false),
        tiePointTimingDiagnostics(false),
        bundlePairPolicy(BUNDLE_PAIR_POLICY_ALL_PAIRS),
        bundleNeighborSpan(0),
        bundleAnchorInputIndexes(1, 0),
        bundleInputPairs(),
        bundleLinearSolverType("auto"),
        maxConcurrentRegistrations(1),
        adaptiveBankThreadCount(4),
        adaptiveFullPostBankRefinement(true),
        nativeLowGridPolicy("advisory"),
        opencvRansacPrefilter(true),
        opencvRansacThresholdPixels(25.0)
      {
      }
   };

   RegistrationSetupOptions registrationSetupDefaults(
      RegistrationSetupApproach approach,
      const std::string& matchMethod);

   bool promptForRegistrationSetup(
      QWidget* parent,
      RegistrationSetupOptions& options);

   void applyRegistrationSetupTieOptions(
      ossim_autoreg::TiePointGenerationOptions& tiePointOptions,
      const RegistrationSetupOptions& setupOptions);
}

#endif
