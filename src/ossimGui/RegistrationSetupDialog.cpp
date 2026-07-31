#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include "RegistrationSetupDialog.h"

#include <ossim/registration/ossimBundleAdjustmentRegistrationSource.h>
#include <ossim/registration/ossimFixedRegistrationSource.h>
#include <ossim/base/ossimString.h>
#include <ossim/imaging/ossimImageGeometry.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim_autoreg/BundleLinearSolverFactory.h>
#include <ossim_autoreg/TiePointGenerator.h>

#include <QAbstractSpinBox>
#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QFormLayout>
#include <QGuiApplication>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace ossimGui
{
   void configureRegistrationField(QWidget* field,
                                   int minimumWidth = 180)
   {
      if(!field)
         return;
      field->setMinimumWidth(minimumWidth);
      field->setMinimumHeight(30);
      QSizePolicy policy = field->sizePolicy();
      policy.setHorizontalPolicy(QSizePolicy::Expanding);
      field->setSizePolicy(policy);
   }

   void configureRegistrationSpinBox(QAbstractSpinBox* spinBox)
   {
      configureRegistrationField(spinBox, 140);
      spinBox->setMinimumHeight(34);
      spinBox->setAccelerated(true);
   }

   void configureRegistrationForm(QFormLayout* form)
   {
      form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
      form->setRowWrapPolicy(QFormLayout::DontWrapRows);
   }

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
      RegistrationSetupDialog(
         QWidget* parent = 0,
         ossimObject* object = 0,
         const RegistrationSetupOptions* initialOptions = 0,
         bool showLaunchActions = false,
         const std::vector<RegistrationSetupInput>& inputs = {})
      : QDialog(parent),
        m_object(object),
        m_executeAfterCreate(false),
        m_executionEligible(true),
        m_setupInputs(inputs),
        m_preflightTable(0),
        m_preflightSummary(0),
        m_excludeNonOverlappingButton(0),
        m_runButton(0),
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
        m_adaptiveFullPostBankRefinement(0)
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
            "Minimum matcher score separation. "
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

         configureRegistrationField(m_approach, 220);
         configureRegistrationField(m_matchMethod, 220);
         configureRegistrationField(m_resampler);
         configureRegistrationField(m_supportPassResampler);
         configureRegistrationField(m_bundlePairPolicy);
         configureRegistrationField(m_bundleAnchorInputIndexes, 220);
         configureRegistrationField(m_bundleInputPairs, 220);
         configureRegistrationField(m_bundleLinearSolver, 220);
         configureRegistrationSpinBox(m_chipSize);
         configureRegistrationSpinBox(m_searchRadius);
         configureRegistrationSpinBox(m_gridSpacing);
         configureRegistrationSpinBox(m_minScore);
         configureRegistrationSpinBox(m_minScoreMargin);
         configureRegistrationSpinBox(m_viewGsd);
         configureRegistrationSpinBox(m_maxTiePoints);
         configureRegistrationSpinBox(m_denseGridSeedBudget);
         configureRegistrationSpinBox(m_bundleNeighborSpan);
         configureRegistrationSpinBox(m_maxConcurrentRegistrations);
         configureRegistrationSpinBox(m_adaptiveBankThreadCount);

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
         connect(m_bundleAnchorInputIndexes, &QLineEdit::textChanged,
                 [this](const QString&) { updatePreflight(); });

         QFormLayout* basicLeftForm = new QFormLayout();
         configureRegistrationForm(basicLeftForm);
         basicLeftForm->addRow("Approach", m_approach);
         basicLeftForm->addRow("Matcher", m_matchMethod);
         basicLeftForm->addRow("Resampler", m_resampler);
         basicLeftForm->addRow(
            "Parallel floating inputs", m_maxConcurrentRegistrations);
         QFormLayout* basicRightForm = new QFormLayout();
         configureRegistrationForm(basicRightForm);
         basicRightForm->addRow("Bundle pair policy", m_bundlePairPolicy);
         basicRightForm->addRow(
            "Bundle neighbor span", m_bundleNeighborSpan);
         basicRightForm->addRow(
            "Bundle anchor input indexes", m_bundleAnchorInputIndexes);
         basicRightForm->addRow(
            "Bundle explicit input pairs", m_bundleInputPairs);
         QHBoxLayout* basicLayout = new QHBoxLayout();
         basicLayout->addLayout(basicLeftForm, 1);
         basicLayout->addSpacing(20);
         basicLayout->addLayout(basicRightForm, 1);
         QWidget* basicOptionsPage = new QWidget(this);
         basicOptionsPage->setLayout(basicLayout);

         QFormLayout* advancedLeftForm = new QFormLayout();
         configureRegistrationForm(advancedLeftForm);
         advancedLeftForm->addRow(
            "Support pass resampler", m_supportPassResampler);
         advancedLeftForm->addRow("Chip size", m_chipSize);
         advancedLeftForm->addRow("Search radius", m_searchRadius);
         advancedLeftForm->addRow("Grid spacing", m_gridSpacing);
         advancedLeftForm->addRow("Minimum score", m_minScore);
         advancedLeftForm->addRow(
            "Minimum score margin", m_minScoreMargin);
         advancedLeftForm->addRow("View GSD", m_viewGsd);
         QFormLayout* advancedRightForm = new QFormLayout();
         configureRegistrationForm(advancedRightForm);
         advancedRightForm->addRow("Max ties", m_maxTiePoints);
         advancedRightForm->addRow(
            "Dense seed budget", m_denseGridSeedBudget);
         advancedRightForm->addRow(
            "Auto dense seed budget", m_autoDenseGridSeedBudget);
         advancedRightForm->addRow(
            "Tie timing diagnostics", m_tiePointTimingDiagnostics);
         advancedRightForm->addRow(
            "Bundle linear solver", m_bundleLinearSolver);
         advancedRightForm->addRow(
            "Adaptive bank threads", m_adaptiveBankThreadCount);
         advancedRightForm->addRow(
            "Full post-bank refinement",
            m_adaptiveFullPostBankRefinement);
         QHBoxLayout* advancedLayout = new QHBoxLayout();
         advancedLayout->addLayout(advancedLeftForm, 1);
         advancedLayout->addSpacing(20);
         advancedLayout->addLayout(advancedRightForm, 1);
         QWidget* advancedOptionsPage = new QWidget(this);
         advancedOptionsPage->setLayout(advancedLayout);
         QTabWidget* optionsTabs = new QTabWidget(this);
         optionsTabs->addTab(basicOptionsPage, "Basic Options");
         optionsTabs->addTab(advancedOptionsPage, "Advanced Options");
         optionsTabs->setCurrentIndex(0);

         if(!inputs.empty())
         {
            m_preflightTable =
               new QTableWidget(static_cast<int>(inputs.size()), 6, this);
            m_preflightTable->setHorizontalHeaderLabels(
               QStringList() << "#" << "Input" << "Entry" << "Role"
                             << "Geometry" << "Overlap");
            m_preflightTable->verticalHeader()->setVisible(false);
            m_preflightTable->setEditTriggers(
               QAbstractItemView::NoEditTriggers);
            m_preflightTable->setSelectionMode(
               QAbstractItemView::SingleSelection);
            m_preflightTable->setSelectionBehavior(
               QAbstractItemView::SelectRows);
            m_preflightTable->setTextElideMode(Qt::ElideLeft);
            m_preflightTable->horizontalHeader()->setStretchLastSection(true);
            m_preflightTable->horizontalHeader()->setSectionResizeMode(
               0, QHeaderView::ResizeToContents);
            m_preflightTable->horizontalHeader()->setSectionResizeMode(
               1, QHeaderView::Stretch);
            m_preflightTable->horizontalHeader()->setSectionResizeMode(
               2, QHeaderView::ResizeToContents);
            m_preflightTable->horizontalHeader()->setSectionResizeMode(
               3, QHeaderView::ResizeToContents);
            m_preflightTable->horizontalHeaderItem(1)->setToolTip(
               "Long input names show the end of the path. "
               "Hover over an input to see its complete name and source.");
            m_preflightTable->setMaximumHeight(190);

            refreshPreflightInputs();
            m_preflightTable->selectRow(0);

            m_preflightSummary = new QLabel(this);
            m_preflightSummary->setWordWrap(true);
         }

         QDialogButtonBox* buttons = new QDialogButtonBox(this);
         buttons->setOrientation(Qt::Horizontal);
         if(showLaunchActions)
         {
            QPushButton* createButton = buttons->addButton(
               "Create Setup", QDialogButtonBox::AcceptRole);
            m_runButton = buttons->addButton(
               "Run Registration", QDialogButtonBox::AcceptRole);
            buttons->addButton(QDialogButtonBox::Cancel);
            connect(createButton, &QPushButton::clicked,
                    [this]() { acceptLaunch(false); });
            connect(m_runButton, &QPushButton::clicked,
                    [this]() { acceptLaunch(true); });
         }
         else
         {
            buttons->addButton(QDialogButtonBox::Ok);
            buttons->addButton(QDialogButtonBox::Cancel);
            connect(buttons, &QDialogButtonBox::accepted,
                    [this]() { acceptLaunch(false); });
         }
         connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

         QVBoxLayout* layout = new QVBoxLayout();
         if(m_preflightTable)
         {
            QVBoxLayout* preflightLayout = new QVBoxLayout();
            preflightLayout->addWidget(m_preflightTable);
            QHBoxLayout* orderingLayout = new QHBoxLayout();
            orderingLayout->addStretch();
            QPushButton* moveUpButton =
               new QPushButton("Move Up", this);
            QPushButton* moveDownButton =
               new QPushButton("Move Down", this);
            m_excludeNonOverlappingButton =
               new QPushButton("Exclude Non-overlapping", this);
            moveUpButton->setMinimumHeight(30);
            moveDownButton->setMinimumHeight(30);
            m_excludeNonOverlappingButton->setMinimumHeight(30);
            moveUpButton->setToolTip(
               "Move the selected image earlier in registration input order.");
            moveDownButton->setToolTip(
               "Move the selected image later in registration input order.");
            m_excludeNonOverlappingButton->setToolTip(
               "Exclude images with determinable geometry that does not "
               "overlap any other selected image. Images remain in the Data "
               "Manager.");
            connect(moveUpButton, &QPushButton::clicked,
                    [this]() { moveSelectedInput(-1); });
            connect(moveDownButton, &QPushButton::clicked,
                    [this]() { moveSelectedInput(1); });
            connect(m_excludeNonOverlappingButton, &QPushButton::clicked,
                    [this]() { excludeNonOverlappingInputs(); });
            orderingLayout->addWidget(m_excludeNonOverlappingButton);
            orderingLayout->addWidget(moveUpButton);
            orderingLayout->addWidget(moveDownButton);
            preflightLayout->addLayout(orderingLayout);
            preflightLayout->addWidget(m_preflightSummary);
            QGroupBox* preflightBox =
               new QGroupBox("Selected Image Preflight", this);
            preflightBox->setLayout(preflightLayout);
            layout->addWidget(preflightBox);
         }
         layout->addWidget(optionsTabs);
         layout->addWidget(buttons);
         setLayout(layout);
         setSizeGripEnabled(true);
         applySelectedDefaults();
         loadObjectOptions();
         if(initialOptions)
            setOptions(*initialOptions);
         updatePreflight();
         QTimer::singleShot(
            0, this, [this]() { resizeForContent(); });
      }

      RegistrationSetupOptions options() const
      {
         RegistrationSetupOptions result = m_baseOptions;
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
         return result;
      }

      bool executeAfterCreate() const
      {
         return m_executeAfterCreate;
      }

      std::vector<RegistrationSetupInput> setupInputs() const
      {
         return m_setupInputs;
      }

   private:
      void resizeForContent()
      {
         if(layout())
         {
            layout()->invalidate();
            layout()->activate();
         }
         QSize target = sizeHint();
         target.setWidth(std::max(target.width(), 760));

         QScreen* screen = QGuiApplication::screenAt(
            frameGeometry().center());
         if(!screen)
            screen = QGuiApplication::primaryScreen();
         if(screen)
         {
            const QSize available = screen->availableGeometry().size();
            target.setWidth(std::min(
               target.width(), std::max(320, available.width() - 60)));
            target.setHeight(std::min(
               target.height(), std::max(320, available.height() - 80)));
         }
         resize(target);
      }

      static std::size_t remapInputIndex(
         std::size_t value,
         std::size_t first,
         std::size_t second)
      {
         if(value == first)
            return second;
         if(value == second)
            return first;
         return value;
      }

      void refreshPreflightInputs()
      {
         m_preflightLabels.clear();
         m_preflightInputs.clear();
         m_preflightOverlaps.clear();
         m_preflightIsolated.clear();

         m_preflightLabels.reserve(m_setupInputs.size());
         m_preflightInputs.reserve(m_setupInputs.size());
         m_preflightOverlaps.reserve(m_setupInputs.size());
         m_preflightIsolated.reserve(m_setupInputs.size());
         for(std::size_t index = 0; index < m_setupInputs.size(); ++index)
         {
            m_preflightLabels.push_back(
               QString::fromStdString(m_setupInputs[index].label));
            m_preflightInputs.push_back(
               ossim_autoreg::registrationSetupInputInfo(
                  m_setupInputs[index].source));

            int availablePeerCount = 0;
            int overlapPeerCount = 0;
            double maximumOverlap = 0.0;
            for(std::size_t peerIndex = 0;
                peerIndex < m_setupInputs.size();
                ++peerIndex)
            {
               if(peerIndex == index)
                  continue;
               const ossim_autoreg::RegistrationSetupOverlapInfo overlap =
                  ossim_autoreg::registrationSetupOverlapInfo(
                     m_setupInputs[index].source,
                     m_setupInputs[peerIndex].source);
               if(!overlap.available)
                  continue;
               ++availablePeerCount;
               maximumOverlap =
                  std::max(maximumOverlap, overlap.normalizedAreaRatio);
               if(overlap.normalizedAreaRatio > 0.0)
                  ++overlapPeerCount;
            }

            const int expectedPeerCount =
               static_cast<int>(m_setupInputs.size()) - 1;
            const bool isolated =
               expectedPeerCount > 0 &&
               availablePeerCount == expectedPeerCount &&
               overlapPeerCount == 0;
            m_preflightIsolated.push_back(isolated);
            if(isolated)
            {
               m_preflightOverlaps.push_back("No overlap");
            }
            else if(!availablePeerCount)
            {
               m_preflightOverlaps.push_back("Unavailable");
            }
            else
            {
               QString overlapText = QString("%1 peer(s), max %2%")
                  .arg(overlapPeerCount)
                  .arg(maximumOverlap * 100.0, 0, 'f', 0);
               if(availablePeerCount != expectedPeerCount)
                  overlapText += "; some unavailable";
               m_preflightOverlaps.push_back(overlapText);
            }
         }
      }

      std::vector<std::size_t> isolatedInputIndexes() const
      {
         std::vector<std::size_t> result;
         for(std::size_t index = 0;
             index < m_preflightIsolated.size();
             ++index)
         {
            if(m_preflightIsolated[index])
               result.push_back(index);
         }
         return result;
      }

      void excludeNonOverlappingInputs()
      {
         const std::vector<std::size_t> removed = isolatedInputIndexes();
         if(removed.empty())
            return;

         QStringList removedLabels;
         for(std::size_t index : removed)
            removedLabels.push_back(m_preflightLabels[index]);
         const QMessageBox::StandardButton response = QMessageBox::warning(
            this, "Exclude Non-overlapping Images",
            QString("Exclude %1 image(s) from this registration?\n\n%2\n\n"
                    "The images will remain in the Data Manager.")
               .arg(static_cast<int>(removed.size()))
               .arg(removedLabels.join("\n")),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel);
         if(response != QMessageBox::Yes)
            return;

         std::vector<ossim_uint32> anchors;
         std::vector<ossim_autoreg::BundleImagePair> pairs;
         const bool validAnchors = parseRegistrationInputIndexes(
            m_bundleAnchorInputIndexes->text().trimmed().toStdString(),
            anchors);
         const bool validPairs = parseRegistrationInputPairs(
            m_bundleInputPairs->text().trimmed().toStdString(), pairs);
         if(!validAnchors || !validPairs)
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "Correct the bundle anchor or explicit pair indexes before "
               "excluding images.");
            return;
         }

         std::vector<int> remappedIndexes(m_setupInputs.size(), -1);
         std::size_t nextIndex = 0;
         for(std::size_t index = 0; index < m_setupInputs.size(); ++index)
         {
            if(!m_preflightIsolated[index])
               remappedIndexes[index] = static_cast<int>(nextIndex++);
         }

         std::vector<ossim_uint32> remappedAnchors;
         for(ossim_uint32 anchor : anchors)
         {
            if(anchor < remappedIndexes.size() &&
               remappedIndexes[anchor] >= 0)
            {
               remappedAnchors.push_back(
                  static_cast<ossim_uint32>(remappedIndexes[anchor]));
            }
         }

         std::vector<ossim_autoreg::BundleImagePair> remappedPairs;
         for(const ossim_autoreg::BundleImagePair& pair : pairs)
         {
            if(pair.firstImageIndex() >= remappedIndexes.size() ||
               pair.secondImageIndex() >= remappedIndexes.size())
            {
               continue;
            }
            const int first = remappedIndexes[pair.firstImageIndex()];
            const int second = remappedIndexes[pair.secondImageIndex()];
            if(first >= 0 && second >= 0)
            {
               remappedPairs.push_back(
                  ossim_autoreg::BundleImagePair(first, second));
            }
         }

         {
            const QSignalBlocker anchorBlocker(m_bundleAnchorInputIndexes);
            const QSignalBlocker pairBlocker(m_bundleInputPairs);
            m_bundleAnchorInputIndexes->setText(
               formatRegistrationInputIndexes(remappedAnchors));
            m_bundleInputPairs->setText(
               formatRegistrationInputPairs(remappedPairs));
         }

         std::vector<RegistrationSetupInput> retainedInputs;
         retainedInputs.reserve(
            m_setupInputs.size() - removed.size());
         for(std::size_t index = 0; index < m_setupInputs.size(); ++index)
         {
            if(!m_preflightIsolated[index])
               retainedInputs.push_back(m_setupInputs[index]);
         }
         m_setupInputs.swap(retainedInputs);
         refreshPreflightInputs();
         updatePreflight();
         if(!m_setupInputs.empty())
         {
            const int selectedRow = std::min(
               static_cast<int>(removed.front()),
               static_cast<int>(m_setupInputs.size()) - 1);
            m_preflightTable->selectRow(selectedRow);
         }
         resizeForContent();
      }

      void moveSelectedInput(int offset)
      {
         const int row = m_preflightTable ?
            m_preflightTable->currentRow() : -1;
         const int target = row + offset;
         if(row < 0 || target < 0 ||
            target >= static_cast<int>(m_setupInputs.size()))
         {
            return;
         }

         const std::size_t first = static_cast<std::size_t>(row);
         const std::size_t second = static_cast<std::size_t>(target);
         std::vector<ossim_uint32> anchors;
         const bool validAnchors = parseRegistrationInputIndexes(
            m_bundleAnchorInputIndexes->text().trimmed().toStdString(),
            anchors);
         if(validAnchors)
         {
            for(ossim_uint32& anchor : anchors)
            {
               anchor = static_cast<ossim_uint32>(
                  remapInputIndex(anchor, first, second));
            }
            std::sort(anchors.begin(), anchors.end());
         }

         std::vector<ossim_autoreg::BundleImagePair> pairs;
         const bool validPairs = parseRegistrationInputPairs(
            m_bundleInputPairs->text().trimmed().toStdString(), pairs);
         std::vector<ossim_autoreg::BundleImagePair> remappedPairs;
         if(validPairs)
         {
            remappedPairs.reserve(pairs.size());
            for(const ossim_autoreg::BundleImagePair& pair : pairs)
            {
               std::size_t pairFirst =
                  remapInputIndex(pair.firstImageIndex(), first, second);
               std::size_t pairSecond =
                  remapInputIndex(pair.secondImageIndex(), first, second);
               if(pairSecond < pairFirst)
                  std::swap(pairFirst, pairSecond);
               remappedPairs.push_back(
                  ossim_autoreg::BundleImagePair(pairFirst, pairSecond));
            }
            std::sort(
               remappedPairs.begin(), remappedPairs.end(),
               [](const ossim_autoreg::BundleImagePair& lhs,
                  const ossim_autoreg::BundleImagePair& rhs) {
                  if(lhs.firstImageIndex() != rhs.firstImageIndex())
                     return lhs.firstImageIndex() < rhs.firstImageIndex();
                  return lhs.secondImageIndex() < rhs.secondImageIndex();
               });
         }

         {
            const QSignalBlocker anchorBlocker(m_bundleAnchorInputIndexes);
            const QSignalBlocker pairBlocker(m_bundleInputPairs);
            if(validAnchors)
            {
               m_bundleAnchorInputIndexes->setText(
                  formatRegistrationInputIndexes(anchors));
            }
            if(validPairs)
            {
               m_bundleInputPairs->setText(
                  formatRegistrationInputPairs(remappedPairs));
            }
         }
         std::swap(m_setupInputs[first], m_setupInputs[second]);
         refreshPreflightInputs();
         updatePreflight();
         m_preflightTable->selectRow(target);
      }

      void acceptLaunch(bool executeAfterCreate)
      {
         if(!validateBundleControls())
            return;
         if(executeAfterCreate && !m_executionEligible)
         {
            QMessageBox::warning(
               this, "Registration Setup",
               m_executionEligibilityMessage);
            return;
         }
         if(!applyObjectOptions())
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "The selected setup could not be applied through the "
               "registered source interface.");
            return;
         }
         m_executeAfterCreate = executeAfterCreate;
         accept();
      }

      void setOptions(const RegistrationSetupOptions& values)
      {
         const int approachIndex =
            m_approach->findData(values.approach);
         if(approachIndex >= 0)
            m_approach->setCurrentIndex(approachIndex);
         const int matcherIndex = m_matchMethod->findData(
            QString::fromStdString(values.matchMethod));
         if(matcherIndex >= 0)
            m_matchMethod->setCurrentIndex(matcherIndex);
         const int resamplerIndex = m_resampler->findData(
            QString::fromStdString(values.resamplerType));
         if(resamplerIndex >= 0)
            m_resampler->setCurrentIndex(resamplerIndex);
         const int supportIndex = m_supportPassResampler->findData(
            QString::fromStdString(values.supportPassMatcherResampler));
         if(supportIndex >= 0)
            m_supportPassResampler->setCurrentIndex(supportIndex);
         m_chipSize->setValue(values.chipSize);
         m_searchRadius->setValue(values.searchRadius);
         m_gridSpacing->setValue(values.gridSpacing);
         m_minScore->setValue(values.minScore);
         m_minScoreMargin->setValue(values.minScoreMargin);
         m_viewGsd->setValue(values.viewGsd);
         m_maxTiePoints->setValue(static_cast<int>(values.maxTiePoints));
         m_denseGridSeedBudget->setValue(
            static_cast<int>(values.denseGridSeedBudget));
         m_autoDenseGridSeedBudget->setChecked(
            values.autoDenseGridSeedBudget);
         m_tiePointTimingDiagnostics->setChecked(
            values.tiePointTimingDiagnostics);
         const int pairPolicyIndex =
            m_bundlePairPolicy->findData(values.bundlePairPolicy);
         if(pairPolicyIndex >= 0)
            m_bundlePairPolicy->setCurrentIndex(pairPolicyIndex);
         m_bundleNeighborSpan->setValue(static_cast<int>(
            values.bundleNeighborSpan ? values.bundleNeighborSpan : 1));
         m_bundleAnchorInputIndexes->setText(
            formatRegistrationInputIndexes(values.bundleAnchorInputIndexes));
         m_bundleInputPairs->setText(
            formatRegistrationInputPairs(values.bundleInputPairs));
         const int solverIndex = m_bundleLinearSolver->findData(
            QString::fromStdString(values.bundleLinearSolverType));
         if(solverIndex >= 0)
            m_bundleLinearSolver->setCurrentIndex(solverIndex);
         m_maxConcurrentRegistrations->setValue(
            static_cast<int>(values.maxConcurrentRegistrations));
         m_adaptiveBankThreadCount->setValue(
            static_cast<int>(values.adaptiveBankThreadCount));
         m_adaptiveFullPostBankRefinement->setChecked(
            values.adaptiveFullPostBankRefinement);
         m_baseOptions = values;
         updateBundlePairPolicyControls();
      }

      void removeApproach(RegistrationSetupApproach approach)
      {
         const int index = m_approach->findData(approach);
         if(index >= 0)
            m_approach->removeItem(index);
      }

      void loadObjectOptions()
      {
         if(!m_object.valid())
            return;

         if(ossimFixedRegistrationSource* fixed =
               dynamic_cast<ossimFixedRegistrationSource*>(m_object.get()))
         {
            removeApproach(REGISTRATION_SETUP_BUNDLE_ANCHORED);
            removeApproach(REGISTRATION_SETUP_BUNDLE_ALL_FLOATING);
            const RegistrationSetupApproach approach =
               fixed->autoRegistrationOptions().getAutoRegister()
                  ? REGISTRATION_SETUP_FIXED_AUTO
                  : REGISTRATION_SETUP_FIXED_MANUAL;
            setOptions(ossim_autoreg::registrationSetupOptions(
               fixed->autoRegistrationOptions(), approach));
            return;
         }

         if(ossimBundleAdjustmentRegistrationSource* bundle =
               dynamic_cast<ossimBundleAdjustmentRegistrationSource*>(
                  m_object.get()))
         {
            removeApproach(REGISTRATION_SETUP_FIXED_AUTO);
            removeApproach(REGISTRATION_SETUP_FIXED_MANUAL);
            const RegistrationSetupApproach approach =
               bundle->allInputsFloating()
                  ? REGISTRATION_SETUP_BUNDLE_ALL_FLOATING
                  : REGISTRATION_SETUP_BUNDLE_ANCHORED;
            RegistrationSetupOptions values =
               ossim_autoreg::registrationSetupOptions(
                  bundle->autoRegistrationOptions(), approach);
            values.bundleAnchorInputIndexes = bundle->anchorInputIndexes();
            values.bundleInputPairs = bundle->bundleInputPairs();
            if(!values.bundleInputPairs.empty())
               values.bundlePairPolicy = BUNDLE_PAIR_POLICY_EXPLICIT;
            setOptions(values);
         }
      }

      bool applyObjectOptions()
      {
         if(!m_object.valid())
            return true;
         const RegistrationSetupOptions values = options();

         if(ossimFixedRegistrationSource* fixed =
               dynamic_cast<ossimFixedRegistrationSource*>(m_object.get()))
         {
            ossim_autoreg::AutoRegistrationOptions current =
               fixed->autoRegistrationOptions();
            if(!ossim_autoreg::applyRegistrationSetupOptions(current, values))
               return false;
            fixed->setAutoRegistrationOptions(current);
            return true;
         }

         if(ossimBundleAdjustmentRegistrationSource* bundle =
               dynamic_cast<ossimBundleAdjustmentRegistrationSource*>(
                  m_object.get()))
         {
            ossim_autoreg::AutoRegistrationOptions current =
               bundle->autoRegistrationOptions();
            if(!ossim_autoreg::applyRegistrationSetupOptions(current, values))
               return false;
            bundle->setAutoRegistrationOptions(current);
            bundle->setAllInputsFloating(
               values.approach ==
                  REGISTRATION_SETUP_BUNDLE_ALL_FLOATING);
            bundle->setAnchorInputIndexes(values.bundleAnchorInputIndexes);
            bundle->setBundleInputPairs(values.bundleInputPairs);
            return true;
         }
         return false;
      }

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
         if(!m_preflightInputs.empty() &&
            std::any_of(
               anchors.begin(), anchors.end(),
               [this](ossim_uint32 index) {
                  return index >= m_preflightInputs.size();
               }))
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "Every anchor index must identify a selected image.");
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
         if(!m_preflightInputs.empty() &&
            std::any_of(
               pairs.begin(), pairs.end(),
               [this](const ossim_autoreg::BundleImagePair& pair) {
                  return pair.firstImageIndex() >= m_preflightInputs.size() ||
                         pair.secondImageIndex() >= m_preflightInputs.size();
               }))
         {
            QMessageBox::warning(
               this, "Registration Setup",
               "Every explicit pair index must identify a selected image.");
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
         m_baseOptions = defaults;

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
         updatePreflight();
      }

      void updatePreflight()
      {
         if(!m_preflightTable)
            return;
         m_preflightTable->setRowCount(
            static_cast<int>(m_preflightInputs.size()));

         const RegistrationSetupApproach approach =
            static_cast<RegistrationSetupApproach>(
               m_approach->itemData(m_approach->currentIndex()).toInt());
         std::vector<ossim_uint32> anchors;
         parseRegistrationInputIndexes(
            m_bundleAnchorInputIndexes->text().trimmed().toStdString(),
            anchors);
         const bool haveFixedEligible = std::any_of(
            m_preflightInputs.begin(), m_preflightInputs.end(),
            [](const ossim_autoreg::RegistrationSetupInputInfo& input) {
               return input.mobility ==
                  ossim_autoreg::REGISTRATION_SETUP_INPUT_FIXED_ELIGIBLE;
            });
         int floatingCount = 0;
         int fixedOrAnchorCount = 0;
         int availableGeometryCount = 0;
         int validAnchorCount = 0;
         const std::vector<std::size_t> isolated = isolatedInputIndexes();

         for(std::size_t index = 0;
             index < m_preflightInputs.size();
             ++index)
         {
            const ossim_autoreg::RegistrationSetupInputInfo& input =
               m_preflightInputs[index];
            if(input.mobility !=
               ossim_autoreg::REGISTRATION_SETUP_INPUT_GEOMETRY_UNAVAILABLE)
            {
               ++availableGeometryCount;
            }
            QString role;
            if(input.mobility ==
               ossim_autoreg::REGISTRATION_SETUP_INPUT_GEOMETRY_UNAVAILABLE)
            {
               role = "Unavailable";
            }
            else if(approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING)
            {
               if(input.mobility ==
                  ossim_autoreg::REGISTRATION_SETUP_INPUT_MOVABLE)
               {
                  role = "Floating";
                  ++floatingCount;
               }
               else
               {
                  role = "Fixed geometry";
                  ++fixedOrAnchorCount;
               }
            }
            else if(approach == REGISTRATION_SETUP_BUNDLE_ANCHORED)
            {
               const bool anchor =
                  std::find(anchors.begin(), anchors.end(),
                            static_cast<ossim_uint32>(index)) != anchors.end();
               if(anchor)
               {
                  role = "Anchor";
                  ++fixedOrAnchorCount;
                  ++validAnchorCount;
               }
               else if(input.mobility ==
                       ossim_autoreg::REGISTRATION_SETUP_INPUT_MOVABLE)
               {
                  role = "Floating";
                  ++floatingCount;
               }
               else
               {
                  role = "Fixed geometry";
                  ++fixedOrAnchorCount;
               }
            }
            else if(input.mobility ==
                    ossim_autoreg::REGISTRATION_SETUP_INPUT_FIXED_ELIGIBLE)
            {
               role = "Fixed / Control";
               ++fixedOrAnchorCount;
            }
            else if(!haveFixedEligible && index == 0)
            {
               role = "Fixed / Control (fallback)";
               ++fixedOrAnchorCount;
            }
            else
            {
               role = "Floating";
               ++floatingCount;
            }

            QString geometry;
            if(input.mobility ==
               ossim_autoreg::REGISTRATION_SETUP_INPUT_GEOMETRY_UNAVAILABLE)
            {
               geometry = "No usable projection";
            }
            else if(!input.adjustableParameterCount)
            {
               geometry = "Non-adjustable";
            }
            else if(!input.unlockedAdjustableParameterCount)
            {
               geometry = QString("%1 adjustable(s), all locked")
                  .arg(static_cast<int>(input.adjustableParameterCount));
            }
            else
            {
               geometry = QString("%1 of %2 adjustable(s) unlocked")
                  .arg(static_cast<int>(
                     input.unlockedAdjustableParameterCount))
                  .arg(static_cast<int>(input.adjustableParameterCount));
            }

            m_preflightTable->setItem(
               static_cast<int>(index), 0,
               new QTableWidgetItem(QString::number(index)));
            QTableWidgetItem* inputItem =
               new QTableWidgetItem(m_preflightLabels[index]);
            QString inputDetail = m_preflightLabels[index];
            if(!input.sourceIdentifier.empty())
            {
               const QString sourceIdentifier =
                  QString::fromStdString(input.sourceIdentifier);
               inputDetail =
                  sourceIdentifier == m_preflightLabels[index] ?
                     sourceIdentifier :
                     QString("Input: %1\nSource: %2")
                        .arg(m_preflightLabels[index], sourceIdentifier);
            }
            inputItem->setToolTip(inputDetail);
            inputItem->setData(Qt::AccessibleTextRole, inputDetail);
            m_preflightTable->setItem(
               static_cast<int>(index), 1, inputItem);
            m_preflightTable->setItem(
               static_cast<int>(index), 2,
               new QTableWidgetItem(QString::number(input.entryIndex)));
            m_preflightTable->setItem(
               static_cast<int>(index), 3,
               new QTableWidgetItem(role));
            m_preflightTable->setItem(
               static_cast<int>(index), 4,
               new QTableWidgetItem(geometry));
            QTableWidgetItem* overlapItem =
               new QTableWidgetItem(m_preflightOverlaps[index]);
            if(m_preflightIsolated[index])
            {
               overlapItem->setForeground(QBrush(QColor(180, 0, 0)));
               QFont warningFont = overlapItem->font();
               warningFont.setBold(true);
               overlapItem->setFont(warningFont);
               overlapItem->setToolTip(
                  "This image has determinable geometry but does not overlap "
                  "any other selected image.");
            }
            m_preflightTable->setItem(
               static_cast<int>(index), 5, overlapItem);
         }

         QString summary = QString("%1 fixed/anchor, %2 floating")
            .arg(fixedOrAnchorCount)
            .arg(floatingCount);
         if(approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING)
            summary += "; relative adjustment";
         if(approach == REGISTRATION_SETUP_BUNDLE_ALL_FLOATING ||
            approach == REGISTRATION_SETUP_BUNDLE_ANCHORED)
         {
            summary += QString("; pair policy: %1")
               .arg(m_bundlePairPolicy->currentText());
         }
         else if(!floatingCount)
            summary += "; no movable floating input is available";
         if(!isolated.empty())
         {
            summary += QString(
               "; <b>Warning: %1 non-overlapping image(s)</b>")
               .arg(static_cast<int>(isolated.size()));
            QStringList isolatedLabels;
            for(std::size_t index : isolated)
               isolatedLabels.push_back(m_preflightLabels[index]);
            m_preflightSummary->setToolTip(
               QString("Non-overlapping registration inputs:\n%1")
                  .arg(isolatedLabels.join("\n")));
         }
         else
         {
            m_preflightSummary->setToolTip(QString());
         }

         m_executionEligible =
            availableGeometryCount >= 2 && floatingCount > 0;
         if(approach == REGISTRATION_SETUP_BUNDLE_ANCHORED)
            m_executionEligible = m_executionEligible && validAnchorCount > 0;
         else if(approach != REGISTRATION_SETUP_BUNDLE_ALL_FLOATING)
            m_executionEligible =
               m_executionEligible && fixedOrAnchorCount > 0;

         if(!m_executionEligible)
         {
            if(availableGeometryCount < 2)
            {
               m_executionEligibilityMessage =
                  "At least two selected images need usable projections "
                  "before registration can run.";
            }
            else if(!floatingCount)
            {
               m_executionEligibilityMessage =
                  "No selected image has an unlocked adjustable geometry to "
                  "serve as a floating input.";
            }
            else
            {
               m_executionEligibilityMessage =
                  approach == REGISTRATION_SETUP_BUNDLE_ANCHORED ?
                     "Anchored registration needs at least one usable "
                     "selected anchor." :
                     "Fixed registration needs at least one usable selected "
                     "control image.";
            }
            summary += "; Create Setup only";
         }
         else
         {
            m_executionEligibilityMessage.clear();
         }
         if(m_runButton)
         {
            m_runButton->setEnabled(m_executionEligible);
            m_runButton->setToolTip(m_executionEligibilityMessage);
         }
         if(m_excludeNonOverlappingButton)
         {
            m_excludeNonOverlappingButton->setEnabled(!isolated.empty());
            m_excludeNonOverlappingButton->setText(
               isolated.empty() ?
                  "Exclude Non-overlapping" :
                  QString("Exclude Non-overlapping (%1)")
                     .arg(static_cast<int>(isolated.size())));
         }
         m_preflightSummary->setText(summary);
      }

      ossimRefPtr<ossimObject> m_object;
      bool m_executeAfterCreate;
      bool m_executionEligible;
      QString m_executionEligibilityMessage;
      RegistrationSetupOptions m_baseOptions;
      std::vector<RegistrationSetupInput> m_setupInputs;
      QTableWidget* m_preflightTable;
      QLabel* m_preflightSummary;
      QPushButton* m_excludeNonOverlappingButton;
      QPushButton* m_runButton;
      std::vector<QString> m_preflightLabels;
      std::vector<ossim_autoreg::RegistrationSetupInputInfo>
         m_preflightInputs;
      std::vector<QString> m_preflightOverlaps;
      std::vector<bool> m_preflightIsolated;
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

   bool promptForRegistrationLaunch(
      QWidget* parent,
      RegistrationSetupOptions& options,
      bool& executeAfterCreate,
      const RegistrationSetupOptions* initialOptions,
      std::vector<RegistrationSetupInput>& inputs)
   {
      RegistrationSetupDialog dialog(
         parent, 0, initialOptions, true, inputs);
      if(dialog.exec() != QDialog::Accepted)
         return false;
      options = dialog.options();
      executeAfterCreate = dialog.executeAfterCreate();
      inputs = dialog.setupInputs();
      return true;
   }

   QWidget* createRegistrationSetupEditor(
      ossimObject* object,
      QWidget* parent)
   {
      return object ? new RegistrationSetupDialog(parent, object) : 0;
   }

}

#endif
