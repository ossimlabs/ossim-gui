#include "RegistrationTiePointWorkbench.h"

#ifdef OSSIM_AUTOREGISTRATION_ENABLED

#include <ossimGui/ImageScrollView.h>
#include <ossimGui/GatherImageViewProjTransVisitor.h>
#include <ossimGui/IvtGeomTransform.h>

#include <ossim/base/ossimGpt.h>
#include <ossim/imaging/ossimImageGeometry.h>

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QSignalBlocker>
#include <QStyleOptionGraphicsItem>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
   struct RenderedTie
   {
      QPointF fixed;
      QPointF moving;
      ossimDpt fixedImage;
      ossimDpt movingImage;
      ossimGpt fixedGround;
      ossimGpt movingGround;
      QPointF displacementStart;
      QPointF displacementEnd;
      ossimGpt displacementStartGround;
      ossimGpt displacementEndGround;
      double displacementPixels = std::numeric_limits<double>::quiet_NaN();
      bool hasDisplacement = false;
      double score = std::numeric_limits<double>::quiet_NaN();
   };

   bool sameTie(const RenderedTie& lhs, const RenderedTie& rhs)
   {
      // Fixed-image coordinates provide the most stable identity across
      // optimization updates; the moving observation is expected to evolve.
      const double tolerance = 0.5;
      return std::abs(lhs.fixedImage.x - rhs.fixedImage.x) <= tolerance &&
             std::abs(lhs.fixedImage.y - rhs.fixedImage.y) <= tolerance;
   }

   class TiePointBatchItem : public QGraphicsItem
   {
   public:
      TiePointBatchItem()
         : m_selected(-1), m_markerPixels(9), m_showLinks(true)
      {
         setZValue(10000.0);
         setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
      }

      QRectF boundingRect() const override
      {
         return m_bounds;
      }

      void setTies(const std::vector<RenderedTie>& ties,
                   const std::vector<RenderedTie>& removed)
      {
         prepareGeometryChange();
         m_ties = ties;
         m_removed = removed;
         m_bounds = QRectF();
         const auto includePoint = [this](const QPointF& point) {
            const QRectF pointRect(point.x() - 2.0, point.y() - 2.0,
                                   4.0, 4.0);
            m_bounds = m_bounds.isNull() ? pointRect :
               m_bounds.united(pointRect);
         };
         for(const RenderedTie& tie : m_ties)
         {
            includePoint(tie.fixed);
            includePoint(tie.moving);
            if(tie.hasDisplacement)
            {
               includePoint(tie.displacementStart);
               includePoint(tie.displacementEnd);
            }
         }
         for(const RenderedTie& tie : m_removed)
            includePoint(tie.fixed);
         m_bounds.adjust(-64.0, -64.0, 64.0, 64.0);
         if(m_selected >= static_cast<int>(m_ties.size()))
            m_selected = -1;
         update();
      }

      void setSelected(int index)
      {
         m_selected = index;
         update();
      }

      void setMarkerPixels(int value)
      {
         m_markerPixels = value;
         update();
      }

      void setShowLinks(bool value)
      {
         m_showLinks = value;
         update();
      }

      void paint(QPainter* painter,
                 const QStyleOptionGraphicsItem* option,
                 QWidget*) override
      {
         if(!painter || !option)
            return;

         const qreal lod =
            std::max<qreal>(0.0001,
               QStyleOptionGraphicsItem::levelOfDetailFromTransform(
                  painter->worldTransform()));
         const qreal radius = static_cast<qreal>(m_markerPixels) /
                              (2.0 * lod);
         const QRectF exposed = option->exposedRect.adjusted(
            -radius * 3.0, -radius * 3.0, radius * 3.0, radius * 3.0);

         painter->setRenderHint(QPainter::Antialiasing, true);
         QPen haloPen(QColor(0, 0, 0, 220), 4.0);
         haloPen.setCosmetic(true);
         QPen linkPen(QColor(255, 225, 0, 230), 2.0);
         linkPen.setCosmetic(true);
         QPen fixedPen(QColor(0, 255, 255), 3.0);
         fixedPen.setCosmetic(true);
         QPen movingPen(QColor(255, 40, 220), 3.0);
         movingPen.setCosmetic(true);

         for(std::size_t index = 0; index < m_ties.size(); ++index)
         {
            const RenderedTie& tie = m_ties[index];
            if(!exposed.contains(tie.fixed) &&
               !exposed.contains(tie.moving) &&
               !(tie.hasDisplacement &&
                 exposed.intersects(
                    QRectF(tie.displacementStart,
                           tie.displacementEnd).normalized())))
            {
               continue;
            }
            if(m_showLinks && tie.hasDisplacement)
            {
               painter->setPen(haloPen);
               painter->drawLine(tie.displacementStart,
                                 tie.displacementEnd);
               painter->setPen(linkPen);
               painter->drawLine(tie.displacementStart,
                                 tie.displacementEnd);
               painter->drawEllipse(tie.displacementStart,
                                    radius * 0.7, radius * 0.7);

               const QLineF displacement(tie.displacementStart,
                                         tie.displacementEnd);
               if(displacement.length() > radius)
               {
                  const double pi = 3.14159265358979323846;
                  const double angle = std::atan2(
                     -displacement.dy(), displacement.dx());
                  const double arrowSize = radius * 1.5;
                  const QPointF first =
                     tie.displacementEnd -
                     QPointF(std::sin(angle + pi / 3.0) * arrowSize,
                             std::cos(angle + pi / 3.0) * arrowSize);
                  const QPointF second =
                     tie.displacementEnd -
                     QPointF(std::sin(angle + pi - pi / 3.0) *
                                arrowSize,
                             std::cos(angle + pi - pi / 3.0) *
                                arrowSize);
                  painter->drawLine(tie.displacementEnd, first);
                  painter->drawLine(tie.displacementEnd, second);
               }
            }

            painter->setBrush(Qt::NoBrush);
            painter->setPen(haloPen);
            painter->drawEllipse(tie.fixed, radius, radius);
            painter->setPen(fixedPen);
            painter->drawEllipse(tie.fixed, radius, radius);

            painter->setPen(haloPen);
            painter->drawLine(tie.moving + QPointF(-radius, -radius),
                              tie.moving + QPointF(radius, radius));
            painter->drawLine(tie.moving + QPointF(-radius, radius),
                              tie.moving + QPointF(radius, -radius));
            painter->setPen(movingPen);
            painter->drawLine(tie.moving + QPointF(-radius, -radius),
                              tie.moving + QPointF(radius, radius));
            painter->drawLine(tie.moving + QPointF(-radius, radius),
                              tie.moving + QPointF(radius, -radius));

            if(static_cast<int>(index) == m_selected)
            {
               QPen selectedPen(Qt::white, 5.0);
               selectedPen.setCosmetic(true);
               painter->setPen(selectedPen);
               painter->drawEllipse(tie.fixed, radius * 2.0, radius * 2.0);
            }
         }

         QPen removedHalo(QColor(0, 0, 0, 220), 5.0);
         removedHalo.setCosmetic(true);
         QPen removedPen(QColor(255, 55, 40), 3.0);
         removedPen.setCosmetic(true);
         for(const RenderedTie& tie : m_removed)
         {
            if(!exposed.contains(tie.fixed))
               continue;
            const qreal removedRadius = radius * 1.4;
            painter->setPen(removedHalo);
            painter->drawLine(
               tie.fixed + QPointF(-removedRadius, -removedRadius),
               tie.fixed + QPointF(removedRadius, removedRadius));
            painter->drawLine(
               tie.fixed + QPointF(-removedRadius, removedRadius),
               tie.fixed + QPointF(removedRadius, -removedRadius));
            painter->setPen(removedPen);
            painter->drawLine(
               tie.fixed + QPointF(-removedRadius, -removedRadius),
               tie.fixed + QPointF(removedRadius, removedRadius));
            painter->drawLine(
               tie.fixed + QPointF(-removedRadius, removedRadius),
               tie.fixed + QPointF(removedRadius, -removedRadius));
         }
      }

   private:
      std::vector<RenderedTie> m_ties;
      std::vector<RenderedTie> m_removed;
      QRectF m_bounds;
      int m_selected;
      int m_markerPixels;
      bool m_showLinks;
   };

   class TiePointWorkbenchDialog : public QDialog
   {
   public:
      TiePointWorkbenchDialog(
         QWidget* parent,
         ossimFixedRegistrationSource* fixedSource,
         ossimBundleAdjustmentRegistrationSource* bundleSource,
         ossimGui::ImageScrollView* view,
         std::shared_ptr<ossimGui::RegistrationTiePointSnapshotMailbox> mailbox)
         : QDialog(parent),
           m_source(fixedSource),
           m_bundleSource(bundleSource),
           m_view(view),
           m_mailbox(std::move(mailbox)),
           m_overlay(new TiePointBatchItem()),
           m_status(new QLabel("Waiting for registration evidence...", this)),
           m_table(new QTableWidget(this)),
           m_pairSelection(new QComboBox(this)),
           m_autoCenter(new QCheckBox("Auto-center selection", this)),
           m_revision(0),
           m_selected(-1),
           m_requestedSelection(-1),
           m_haveSnapshot(false),
           m_completionShown(false)
      {
         setWindowTitle("Experimental Tie Point Workbench");
         setAttribute(Qt::WA_DeleteOnClose);
         setWindowFlag(Qt::Tool, true);
         resize(720, 520);

         QVBoxLayout* layout = new QVBoxLayout(this);
         const bool bundleMode = m_bundleSource.valid();
         QLabel* legend = new QLabel(
            bundleMode ?
               "<b style='color:#00ffff'>Cyan circle</b>: first image "
               "observation &nbsp; "
               "<b style='color:#ff28dc'>Magenta X</b>: second image "
               "observation" :
               "<b style='color:#00ffff'>Cyan circle</b>: fixed observation "
               "&nbsp; "
               "<b style='color:#ff28dc'>Magenta X</b>: moving observation "
               "&nbsp; "
               "<b style='color:#ffe100'>Yellow arrow</b>: applied correction "
               "(before to after) &nbsp; "
               "<b style='color:#ff3728'>Red X</b>: removed in latest update",
            this);
         legend->setTextFormat(Qt::RichText);
         layout->addWidget(legend);
         m_status->setWordWrap(true);
         layout->addWidget(m_status);

         QHBoxLayout* pairLayout = new QHBoxLayout();
         pairLayout->addWidget(
            new QLabel(bundleMode ? "Bundle edge" : "Registration pair",
                       this));
         pairLayout->addWidget(m_pairSelection, 1);
         layout->addLayout(pairLayout);

         QHBoxLayout* controls = new QHBoxLayout();
         QPushButton* previous = new QPushButton("Previous", this);
         QPushButton* next = new QPushButton("Next", this);
         QPushButton* center = new QPushButton("Center Selected", this);
         m_autoCenter->setChecked(true);
         QCheckBox* links = new QCheckBox("Show applied correction", this);
         links->setChecked(true);
         links->setVisible(!bundleMode);
         links->setToolTip(
            "Draw the moving model's predicted location before registration "
            "to its predicted location after registration.");
         QLabel* sizeLabel = new QLabel("Marker size", this);
         QSlider* size = new QSlider(Qt::Horizontal, this);
         size->setRange(5, 25);
         size->setValue(9);
         size->setMaximumWidth(130);
         controls->addWidget(previous);
         controls->addWidget(next);
         controls->addWidget(center);
         controls->addWidget(m_autoCenter);
         controls->addWidget(links);
         controls->addStretch();
         controls->addWidget(sizeLabel);
         controls->addWidget(size);
         layout->addLayout(controls);

         m_table->setColumnCount(7);
         m_table->setHorizontalHeaderLabels(
            bundleMode ?
               (QStringList() << "#" << "Score" << "Applied px"
                              << "First X" << "First Y"
                              << "Second X" << "Second Y") :
               (QStringList() << "#" << "Score" << "Applied px"
                              << "Fixed X" << "Fixed Y"
                              << "Moving X" << "Moving Y"));
         m_table->setColumnHidden(2, bundleMode);
         m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
         m_table->setSelectionMode(QAbstractItemView::SingleSelection);
         m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
         m_table->verticalHeader()->setVisible(false);
         m_table->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
         m_table->horizontalHeader()->setStretchLastSection(true);
         layout->addWidget(m_table, 1);

         QDialogButtonBox* buttons =
            new QDialogButtonBox(QDialogButtonBox::Close, this);
         connect(buttons, &QDialogButtonBox::rejected,
                 this, &QDialog::close);
         layout->addWidget(buttons);

         if(m_view && m_view->scene())
            m_view->scene()->addItem(m_overlay);
         if(m_view)
         {
            connect(m_view,
                    &ossimGui::ImageScrollView::viewChanged,
                    this,
                    [this]() {
                       if(m_haveSnapshot)
                          remapTiesToCurrentView();
                    });
            connect(m_view,
                    &QObject::destroyed,
                    this,
                    [this]() {
                       m_view = 0;
                       m_overlay = 0;
                       close();
                    });
         }

         connect(m_table, &QTableWidget::currentCellChanged,
            [this](int row, int, int, int) {
               selectTie(row, m_autoCenter->isChecked());
            });
         connect(m_pairSelection,
                 QOverload<int>::of(&QComboBox::currentIndexChanged),
                 [this](int index) { activatePair(index); });
         connect(previous, &QPushButton::clicked, [this]() {
            if(m_ties.empty())
               return;
            const int nextIndex =
               (m_selected <= 0) ?
                  static_cast<int>(m_ties.size()) - 1 : m_selected - 1;
            m_table->selectRow(nextIndex);
         });
         connect(next, &QPushButton::clicked, [this]() {
            if(m_ties.empty())
               return;
            const int nextIndex =
               (m_selected + 1) % static_cast<int>(m_ties.size());
            m_table->selectRow(nextIndex);
         });
         connect(center, &QPushButton::clicked,
                 [this]() { centerSelected(); });
         connect(links, &QCheckBox::toggled,
                 [this](bool checked) {
                    if(m_overlay)
                       m_overlay->setShowLinks(checked);
                 });
         connect(size, &QSlider::valueChanged,
                 [this](int value) {
                    if(m_overlay)
                       m_overlay->setMarkerPixels(value);
                 });

         QTimer* timer = new QTimer(this);
         connect(timer, &QTimer::timeout, [this]() { pollMailbox(); });
         timer->start(100);
      }

      ~TiePointWorkbenchDialog() override
      {
         if(m_overlay)
         {
            if(m_overlay->scene())
               m_overlay->scene()->removeItem(m_overlay);
            delete m_overlay;
         }
      }

   private:
      ossimRefPtr<ossimGui::IvtGeomTransform>
      commonViewTransform() const
      {
         if(!m_view || !m_view->layers())
            return 0;
         const ossim_uint32 layerCount =
            m_view->layers()->numberOfLayers();
         for(ossim_uint32 index = 0; index < layerCount; ++index)
         {
            ossimGui::ImageScrollView::Layer* layer =
               m_view->layers()->layer(index);
            ossimImageSource* layerSource = layer ? layer->chain() : 0;
            if(!layerSource)
               continue;
            ossimGui::GatherImageViewProjTransVisitor visitor;
            layerSource->accept(visitor);
            if(visitor.getTransformList().size() == 1 &&
               visitor.getTransformList().front().valid())
            {
               return visitor.getTransformList().front();
            }
         }
         return 0;
      }

      ossimImageGeometry* inputGeometry(ossim_uint32 inputIndex) const
      {
         if(m_source.valid())
         {
            const ossimFixedRegistrationSource::InputWrapper* input =
               m_source->inputWrapper(inputIndex);
            return input ? input->geometry() : 0;
         }
         if(m_bundleSource.valid())
         {
            const ossimBundleAdjustmentRegistrationSource::InputWrapper*
               input = m_bundleSource->inputWrapper(inputIndex);
            return input ? input->registrationImage().geometry() : 0;
         }
         return 0;
      }

      ossimImageSource* inputSource(ossim_uint32 inputIndex) const
      {
         if(m_source.valid())
         {
            const ossimFixedRegistrationSource::InputWrapper* input =
               m_source->inputWrapper(inputIndex);
            return input ? input->source() : 0;
         }
         if(m_bundleSource.valid())
         {
            const ossimBundleAdjustmentRegistrationSource::InputWrapper*
               input = m_bundleSource->inputWrapper(inputIndex);
            return input ? input->source() : 0;
         }
         return 0;
      }

      bool mapImagePoint(ossim_uint32 inputIndex,
                         const ossimDpt& imagePoint,
                         ossimGui::IvtGeomTransform* commonView,
                         QPointF& viewPoint,
                         ossimGpt* resolvedGround = 0) const
      {
         if((!m_source.valid() && !m_bundleSource.valid()) || !m_view)
            return false;
         ossimImageGeometry* geometry = inputGeometry(inputIndex);
         if(geometry)
         {
            ossimGpt ground;
            geometry->localToWorld(imagePoint, ground);
            if(resolvedGround)
               *resolvedGround = ground;
            if(commonView && !ground.isLatLonNan())
            {
               ossimDpt mapped;
               commonView->groundToView(ground, mapped);
               if(!mapped.hasNans())
               {
                  viewPoint = QPointF(mapped.x, mapped.y);
                  return true;
               }
            }
         }

         // Direct layer mapping is a fallback when a common ground/view
         // conversion is unavailable.
         ossimImageSource* source = inputSource(inputIndex);
         ossimGui::ImageScrollView::Layer* layer =
            source && m_view->layers() ?
               m_view->layers()->layer(source) : 0;
         ossimImageSource* layerSource = layer ? layer->chain() : 0;
         if(layerSource)
         {
            ossimGui::GatherImageViewProjTransVisitor visitor;
            layerSource->accept(visitor);
            if(visitor.getTransformList().size() == 1 &&
               visitor.getTransformList().front().valid())
            {
               ossimDpt mapped;
               visitor.getTransformList().front()->imageToView(
                  imagePoint, mapped);
               if(!mapped.hasNans())
               {
                  viewPoint = QPointF(mapped.x, mapped.y);
                  return true;
               }
            }
         }

         // Geometry mapping is a fallback for chains without an explicit
         // image/view transform.
         ossimImageGeometry* viewGeometry = m_view->getGeometry();
         if(!geometry || !viewGeometry)
            return false;
         ossimGpt ground;
         geometry->localToWorld(imagePoint, ground);
         if(ground.isLatLonNan())
            return false;
         ossimDpt mapped;
         viewGeometry->worldToLocal(ground, mapped);
         if(mapped.hasNans())
            return false;
         viewPoint = QPointF(mapped.x, mapped.y);
         return true;
      }

      bool mapGroundPoint(const ossimGpt& ground,
                          ossimGui::IvtGeomTransform* commonView,
                          QPointF& viewPoint) const
      {
         if(ground.isLatLonNan() || !commonView)
            return false;
         ossimDpt mapped;
         commonView->groundToView(ground, mapped);
         if(mapped.hasNans())
            return false;
         viewPoint = QPointF(mapped.x, mapped.y);
         return true;
      }

      void remapTiesToCurrentView()
      {
         ossimRefPtr<ossimGui::IvtGeomTransform> commonView =
            commonViewTransform();
         for(RenderedTie& tie : m_ties)
         {
            mapGroundPoint(tie.fixedGround, commonView.get(), tie.fixed);
            mapGroundPoint(tie.movingGround, commonView.get(), tie.moving);
            if(tie.hasDisplacement)
            {
               mapGroundPoint(tie.displacementStartGround,
                              commonView.get(),
                              tie.displacementStart);
               mapGroundPoint(tie.displacementEndGround,
                              commonView.get(),
                              tie.displacementEnd);
            }
         }
         for(RenderedTie& tie : m_removed)
         {
            mapGroundPoint(tie.fixedGround, commonView.get(), tie.fixed);
            mapGroundPoint(tie.movingGround, commonView.get(), tie.moving);
            if(tie.hasDisplacement)
            {
               mapGroundPoint(tie.displacementStartGround,
                              commonView.get(),
                              tie.displacementStart);
               mapGroundPoint(tie.displacementEndGround,
                              commonView.get(),
                              tie.displacementEnd);
            }
         }
         if(m_overlay)
         {
            m_overlay->setTies(m_ties, m_removed);
            m_overlay->setSelected(m_selected);
            updateSceneBoundsForTies();
         }
      }

      void updateSceneBoundsForTies()
      {
         if(!m_view || !m_overlay || m_ties.empty())
            return;

         const QRectF overlayBounds = m_overlay->sceneBoundingRect();
         ossimDrect tieBounds(overlayBounds.left(),
                              overlayBounds.top(),
                              overlayBounds.right(),
                              overlayBounds.bottom());
         const ossimDrect visible = m_view->viewportBoundsInSceneSpace();
         if(!visible.hasNans())
         {
            tieBounds = ossimDrect(
               tieBounds.ul().x - visible.width() * 0.5,
               tieBounds.ul().y - visible.height() * 0.5,
               tieBounds.lr().x + visible.width() * 0.5,
               tieBounds.lr().y + visible.height() * 0.5);
         }

         const ossimDrect& input = m_view->getInputBounds();
         const ossimDrect sceneBounds =
            input.hasNans() ? tieBounds : input.combine(tieBounds);
         m_view->setSceneRect(sceneBounds.ul().x,
                              sceneBounds.ul().y,
                              sceneBounds.width(),
                              sceneBounds.height());
      }

      void pollMailbox()
      {
         if(!m_mailbox)
            return;
         std::vector<ossimGui::RegistrationTiePointSnapshot> snapshots;
         if(m_mailbox->read(m_revision, snapshots))
            updateAvailablePairs(snapshots);

         bool success = false;
         std::string message;
         if(!m_completionShown &&
            m_mailbox->completion(success, message))
         {
            m_completionShown = true;
            m_status->setText(
               QString("%1 — %2")
                  .arg(success ? "Registration complete" :
                                 "Registration stopped")
                  .arg(QString::fromStdString(message)));
         }
      }

      void updateAvailablePairs(
         const std::vector<ossimGui::RegistrationTiePointSnapshot>& snapshots)
      {
         ossim_uint32 selectedFirst = 0;
         ossim_uint32 selectedSecond = 0;
         const bool preserveSelection = m_haveSnapshot;
         if(preserveSelection)
         {
            selectedFirst = m_latestSnapshot.firstInputIndex();
            selectedSecond = m_latestSnapshot.secondInputIndex();
         }

         m_snapshots = snapshots;
         int selectedIndex = m_snapshots.empty() ? -1 : 0;
         QSignalBlocker blocker(m_pairSelection);
         m_pairSelection->clear();
         for(std::size_t index = 0; index < m_snapshots.size(); ++index)
         {
            const ossimGui::RegistrationTiePointSnapshot& snapshot =
               m_snapshots[index];
            m_pairSelection->addItem(
               snapshot.bundleEdge() ?
                  QString("Input %1  \u2194  Input %2")
                     .arg(snapshot.firstInputIndex())
                     .arg(snapshot.secondInputIndex()) :
                  QString("Fixed input %1  \u2194  Floating input %2")
                     .arg(snapshot.firstInputIndex())
                     .arg(snapshot.secondInputIndex()));
            if(preserveSelection &&
               snapshot.firstInputIndex() == selectedFirst &&
               snapshot.secondInputIndex() == selectedSecond)
            {
               selectedIndex = static_cast<int>(index);
            }
         }
         m_pairSelection->setEnabled(m_snapshots.size() > 1);
         m_pairSelection->setCurrentIndex(selectedIndex);
         if(selectedIndex >= 0)
            activatePair(selectedIndex);
      }

      void activatePair(int index)
      {
         if(index < 0 || index >= static_cast<int>(m_snapshots.size()))
            return;
         const ossimGui::RegistrationTiePointSnapshot& snapshot =
            m_snapshots[static_cast<std::size_t>(index)];
         const bool pairChanged =
            m_haveSnapshot &&
            (m_latestSnapshot.firstInputIndex() !=
                snapshot.firstInputIndex() ||
             m_latestSnapshot.secondInputIndex() !=
                snapshot.secondInputIndex());
         if(pairChanged)
         {
            m_requestedSelection = m_selected >= 0 ? m_selected : 0;
            m_ties.clear();
            m_removed.clear();
            m_selected = -1;
         }
         if(m_view)
         {
            m_view->setMultiLayerPair(snapshot.firstInputIndex(),
                                      snapshot.secondInputIndex());
         }
         applySnapshot(snapshot);
      }

      void applySnapshot(
         const ossimGui::RegistrationTiePointSnapshot& snapshot)
      {
         m_latestSnapshot = snapshot;
         m_haveSnapshot = true;
         std::vector<RenderedTie> next;
         next.reserve(snapshot.tiePoints().size());
         ossimRefPtr<ossimGui::IvtGeomTransform> commonView =
            commonViewTransform();
         const std::vector<ossim_autoreg::TiePointResidual>& initialResiduals =
            snapshot.initialTiePointResiduals();
         const std::vector<ossim_autoreg::TiePointResidual>& finalResiduals =
            snapshot.tiePointResiduals();
         for(std::size_t observationIndex = 0;
             observationIndex < snapshot.tiePoints().size();
             ++observationIndex)
         {
            const ossim_autoreg::TiePointObservation& observation =
               snapshot.tiePoints()[observationIndex];
            RenderedTie tie;
            tie.fixedImage = observation.fixedPoint();
            tie.movingImage = observation.movingPoint();
            tie.score = observation.score();
            if(mapImagePoint(snapshot.firstInputIndex(),
                             tie.fixedImage, commonView.get(), tie.fixed,
                             &tie.fixedGround) &&
               mapImagePoint(snapshot.secondInputIndex(),
                             tie.movingImage, commonView.get(), tie.moving,
                             &tie.movingGround))
            {
               if(observationIndex < initialResiduals.size() &&
                  observationIndex < finalResiduals.size() &&
                  initialResiduals[observationIndex].valid() &&
                  finalResiduals[observationIndex].valid())
               {
                  const ossimDpt& before =
                     initialResiduals[observationIndex].
                        predictedMovingPoint();
                  const ossimDpt& after =
                     finalResiduals[observationIndex].
                        predictedMovingPoint();
                  if(mapImagePoint(snapshot.secondInputIndex(),
                                   before,
                                   commonView.get(),
                                   tie.displacementStart,
                                   &tie.displacementStartGround) &&
                     mapImagePoint(snapshot.secondInputIndex(),
                                   after,
                                   commonView.get(),
                                   tie.displacementEnd,
                                   &tie.displacementEndGround))
                  {
                     const double dx = after.x - before.x;
                     const double dy = after.y - before.y;
                     tie.displacementPixels =
                        std::sqrt((dx * dx) + (dy * dy));
                     tie.hasDisplacement = true;
                  }
               }
               next.push_back(tie);
            }
         }

         m_removed.clear();
         for(const RenderedTie& oldTie : m_ties)
         {
            if(std::find_if(next.begin(), next.end(),
                  [&oldTie](const RenderedTie& newTie) {
                     return sameTie(oldTie, newTie);
                  }) == next.end())
            {
               m_removed.push_back(oldTie);
            }
         }

         RenderedTie selectedTie;
         const int previousSelection = m_selected;
         const bool hadSelection =
            m_selected >= 0 &&
            m_selected < static_cast<int>(m_ties.size());
         if(hadSelection)
            selectedTie = m_ties[static_cast<std::size_t>(m_selected)];

         m_ties = std::move(next);
         m_selected = -1;
         if(hadSelection)
         {
            const auto selected =
               std::find_if(m_ties.begin(), m_ties.end(),
                  [&selectedTie](const RenderedTie& tie) {
                     return sameTie(selectedTie, tie);
                  });
            if(selected != m_ties.end())
               m_selected = static_cast<int>(
                  std::distance(m_ties.begin(), selected));
         }
         if(!m_ties.empty() && m_selected < 0)
         {
            const int preferred =
               m_requestedSelection >= 0 ?
                  m_requestedSelection :
                  (previousSelection >= 0 ? previousSelection : 0);
            m_selected = std::min(
               preferred, static_cast<int>(m_ties.size()) - 1);
         }
         m_requestedSelection = -1;

         if(m_overlay)
         {
            m_overlay->setTies(m_ties, m_removed);
            m_overlay->setSelected(m_selected);
            updateSceneBoundsForTies();
         }
         rebuildTable();
         if(m_selected >= 0 && m_autoCenter->isChecked())
            centerSelected();
         const QString pass =
            snapshot.passIndex() >= 0 && snapshot.passCount() > 0 ?
               QString("pass %1/%2")
                  .arg(snapshot.passIndex() + 1)
                  .arg(snapshot.passCount()) :
               QString("iteration");
         m_status->setText(
            QString("%1 — %2 retained, %3 removed — %4")
               .arg(pass)
               .arg(m_ties.size())
               .arg(m_removed.size())
               .arg(QString::fromStdString(snapshot.message())));
      }

      void rebuildTable()
      {
         QSignalBlocker blocker(m_table);
         m_table->setUpdatesEnabled(false);
         m_table->setRowCount(static_cast<int>(m_ties.size()));
         for(int row = 0; row < static_cast<int>(m_ties.size()); ++row)
         {
            const RenderedTie& tie = m_ties[static_cast<std::size_t>(row)];
            const QString values[] = {
               QString::number(row + 1),
               std::isfinite(tie.score) ?
                  QString::number(tie.score, 'f', 3) : QString("-"),
               std::isfinite(tie.displacementPixels) ?
                  QString::number(tie.displacementPixels, 'f', 2) :
                  QString("-"),
               QString::number(tie.fixedImage.x, 'f', 2),
               QString::number(tie.fixedImage.y, 'f', 2),
               QString::number(tie.movingImage.x, 'f', 2),
               QString::number(tie.movingImage.y, 'f', 2)
            };
            for(int column = 0; column < 7; ++column)
               m_table->setItem(row, column,
                                new QTableWidgetItem(values[column]));
         }
         if(m_selected >= 0)
            m_table->selectRow(m_selected);
         m_table->setUpdatesEnabled(true);
      }

      void selectTie(int row, bool center)
      {
         if(row < 0 || row >= static_cast<int>(m_ties.size()))
            return;
         m_selected = row;
         if(m_overlay)
            m_overlay->setSelected(row);
         if(center)
            centerSelected();
      }

      void centerSelected()
      {
         if(!m_view || m_selected < 0 ||
            m_selected >= static_cast<int>(m_ties.size()))
         {
            return;
         }
         const RenderedTie& tie =
            m_ties[static_cast<std::size_t>(m_selected)];
         const QPointF center = (tie.fixed + tie.moving) * 0.5;
         const ossimDpt viewCenter(center.x(), center.y());
         m_view->setPositionGivenView(viewCenter);
         m_view->setLastClickedPoint(viewCenter);
         m_view->emitTracking(viewCenter);
      }

      ossimRefPtr<ossimFixedRegistrationSource> m_source;
      ossimRefPtr<ossimBundleAdjustmentRegistrationSource> m_bundleSource;
      QPointer<ossimGui::ImageScrollView> m_view;
      std::shared_ptr<ossimGui::RegistrationTiePointSnapshotMailbox> m_mailbox;
      TiePointBatchItem* m_overlay;
      QLabel* m_status;
      QTableWidget* m_table;
      QComboBox* m_pairSelection;
      QCheckBox* m_autoCenter;
      std::vector<RenderedTie> m_ties;
      std::vector<RenderedTie> m_removed;
      std::vector<ossimGui::RegistrationTiePointSnapshot> m_snapshots;
      ossimGui::RegistrationTiePointSnapshot m_latestSnapshot;
      std::uint64_t m_revision;
      int m_selected;
      int m_requestedSelection;
      bool m_haveSnapshot;
      bool m_completionShown;
   };
}

void ossimGui::RegistrationTiePointSnapshotMailbox::publish(
   const RegistrationTiePointSnapshot& snapshot)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   const auto existing =
      std::find_if(m_snapshots.begin(), m_snapshots.end(),
         [&snapshot](const RegistrationTiePointSnapshot& current) {
            return current.firstInputIndex() == snapshot.firstInputIndex() &&
                   current.secondInputIndex() == snapshot.secondInputIndex();
         });
   if(existing == m_snapshots.end())
      m_snapshots.push_back(snapshot);
   else
      *existing = snapshot;
   ++m_revision;
}

void ossimGui::RegistrationTiePointSnapshotMailbox::publish(
   const ossimFixedRegistrationSource::TiePointSnapshot& sourceSnapshot)
{
   RegistrationTiePointSnapshot snapshot;
   snapshot.setFirstInputIndex(sourceSnapshot.fixedInputIndex());
   snapshot.setSecondInputIndex(sourceSnapshot.movingInputIndex());
   snapshot.setPassIndex(sourceSnapshot.passIndex());
   snapshot.setPassCount(sourceSnapshot.passCount());
   snapshot.setMessage(sourceSnapshot.message());
   snapshot.setTiePoints(sourceSnapshot.tiePoints());
   snapshot.setInitialTiePointResiduals(
      sourceSnapshot.initialTiePointResiduals());
   snapshot.setTiePointResiduals(sourceSnapshot.tiePointResiduals());
   publish(snapshot);
}

bool ossimGui::RegistrationTiePointSnapshotMailbox::read(
   std::uint64_t& revision,
   std::vector<RegistrationTiePointSnapshot>& snapshots) const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if(revision == m_revision)
      return false;
   revision = m_revision;
   snapshots = m_snapshots;
   return true;
}

void ossimGui::RegistrationTiePointSnapshotMailbox::finish(
   bool success,
   const std::string& message)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_finished = true;
   m_success = success;
   m_completionMessage = message;
}

bool ossimGui::RegistrationTiePointSnapshotMailbox::completion(
   bool& success,
   std::string& message) const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   if(!m_finished)
      return false;
   success = m_success;
   message = m_completionMessage;
   return true;
}

QWidget* ossimGui::createRegistrationTiePointWorkbench(
   QWidget* parent,
   ossimFixedRegistrationSource* source,
   ImageScrollView* view,
   const std::shared_ptr<RegistrationTiePointSnapshotMailbox>& mailbox)
{
   return new TiePointWorkbenchDialog(
      parent, source, 0, view, mailbox);
}

QWidget* ossimGui::createRegistrationTiePointWorkbench(
   QWidget* parent,
   ossimBundleAdjustmentRegistrationSource* source,
   ImageScrollView* view,
   const std::shared_ptr<RegistrationTiePointSnapshotMailbox>& mailbox)
{
   return new TiePointWorkbenchDialog(
      parent, 0, source, view, mailbox);
}

#endif
