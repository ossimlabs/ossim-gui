#ifndef ossimGui_RegistrationSetupDialog_HEADER
#define ossimGui_RegistrationSetupDialog_HEADER

#include <ossim_autoreg/RegistrationSourceSetup.h>

class QWidget;

namespace ossimGui
{
   using RegistrationSetupApproach =
      ossim_autoreg::RegistrationSetupApproach;
   using BundlePairPolicy =
      ossim_autoreg::RegistrationSetupBundlePairPolicy;
   using RegistrationSetupOptions =
      ossim_autoreg::RegistrationSetupOptions;

   constexpr RegistrationSetupApproach REGISTRATION_SETUP_FIXED_AUTO =
      ossim_autoreg::REGISTRATION_SETUP_FIXED_AUTO;
   constexpr RegistrationSetupApproach REGISTRATION_SETUP_FIXED_MANUAL =
      ossim_autoreg::REGISTRATION_SETUP_FIXED_MANUAL;
   constexpr RegistrationSetupApproach
      REGISTRATION_SETUP_BUNDLE_ALL_FLOATING =
         ossim_autoreg::REGISTRATION_SETUP_BUNDLE_ALL_FLOATING;
   constexpr RegistrationSetupApproach REGISTRATION_SETUP_BUNDLE_ANCHORED =
      ossim_autoreg::REGISTRATION_SETUP_BUNDLE_ANCHORED;

   constexpr BundlePairPolicy BUNDLE_PAIR_POLICY_ALL_PAIRS =
      ossim_autoreg::REGISTRATION_SETUP_BUNDLE_PAIR_POLICY_ALL_PAIRS;
   constexpr BundlePairPolicy BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN =
      ossim_autoreg::REGISTRATION_SETUP_BUNDLE_PAIR_POLICY_NEIGHBOR_SPAN;
   constexpr BundlePairPolicy BUNDLE_PAIR_POLICY_AUTO =
      ossim_autoreg::REGISTRATION_SETUP_BUNDLE_PAIR_POLICY_AUTO;
   constexpr BundlePairPolicy BUNDLE_PAIR_POLICY_EXPLICIT =
      ossim_autoreg::REGISTRATION_SETUP_BUNDLE_PAIR_POLICY_EXPLICIT;

   bool promptForRegistrationSetup(
      QWidget* parent,
      RegistrationSetupOptions& options);

}

#endif
