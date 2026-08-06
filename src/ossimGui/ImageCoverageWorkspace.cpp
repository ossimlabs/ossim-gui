#include "ImageCoverageWorkspace.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QRubberBand>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace
{
   class CoverageGraphicsView : public QGraphicsView
   {
   public:
      using FootprintClicked =
         std::function<bool(const QPointF&, Qt::KeyboardModifiers, int)>;
      using FootprintsDragged =
         std::function<bool(const QPolygonF&, Qt::KeyboardModifiers)>;

      CoverageGraphicsView(
         QGraphicsScene* scene,
         QWidget* parent,
         FootprintClicked footprintClicked,
         FootprintsDragged footprintsDragged)
      : QGraphicsView(scene, parent),
        m_footprintClicked(std::move(footprintClicked)),
        m_footprintsDragged(std::move(footprintsDragged)),
        m_rubberBand(new QRubberBand(QRubberBand::Rectangle, viewport())),
        m_pressed(false),
        m_dragging(false),
        m_lastClickPosition(-10000, -10000),
        m_clickCycle(0)
      {
         setRenderHint(QPainter::Antialiasing, true);
         setDragMode(QGraphicsView::NoDrag);
         setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
         setResizeAnchor(QGraphicsView::AnchorViewCenter);
         setBackgroundBrush(QColor(247, 247, 247));
      }

      void fitCoverage()
      {
         if(!scene() || scene()->sceneRect().isEmpty())
            return;
         QRectF bounds = scene()->sceneRect();
         const double margin = std::max(bounds.width(), bounds.height()) * 0.08;
         if(margin > 0.0)
            bounds.adjust(-margin, -margin, margin, margin);
         fitInView(bounds, Qt::KeepAspectRatio);
      }

   protected:
      void mousePressEvent(QMouseEvent* event) override
      {
         if(event->button() == Qt::LeftButton)
         {
            m_pressed = true;
            m_dragging = false;
            m_pressPosition = event->pos();
            m_pressModifiers = event->modifiers();
            event->accept();
            return;
         }
         QGraphicsView::mousePressEvent(event);
      }

      void mouseMoveEvent(QMouseEvent* event) override
      {
         if(m_pressed)
         {
            if(!m_dragging &&
               (event->pos() - m_pressPosition).manhattanLength() > 4)
               m_dragging = true;
            if(m_dragging)
            {
               m_rubberBand->setGeometry(
                  QRect(m_pressPosition, event->pos()).normalized());
               m_rubberBand->show();
            }
            event->accept();
            return;
         }
         QGraphicsView::mouseMoveEvent(event);
      }

      void mouseReleaseEvent(QMouseEvent* event) override
      {
         if(event->button() == Qt::LeftButton && m_pressed)
         {
            m_pressed = false;
            m_rubberBand->hide();
            if(m_dragging && m_footprintsDragged)
            {
               const QRect selectionRect =
                  QRect(m_pressPosition, event->pos()).normalized();
               m_footprintsDragged(
                  mapToScene(selectionRect), m_pressModifiers);
            }
            else if(m_footprintClicked)
            {
               const bool repeated = m_lastClickTimer.isValid() &&
                  m_lastClickTimer.elapsed() < 1000 &&
                  (event->pos() - m_lastClickPosition).manhattanLength() <= 4;
               m_clickCycle = repeated ? m_clickCycle + 1 : 0;
               m_lastClickPosition = event->pos();
               m_lastClickTimer.restart();
               m_footprintClicked(
                  mapToScene(event->pos()),
                  m_pressModifiers,
                  m_clickCycle);
            }
            m_dragging = false;
            event->accept();
            return;
         }
         QGraphicsView::mouseReleaseEvent(event);
      }

      void wheelEvent(QWheelEvent* event) override
      {
         if(event->modifiers().testFlag(Qt::ControlModifier) ||
            event->modifiers().testFlag(Qt::MetaModifier))
         {
            const double factor =
               event->angleDelta().y() > 0 ? 1.2 : 1.0 / 1.2;
            scale(factor, factor);
            event->accept();
            return;
         }
         QGraphicsView::wheelEvent(event);
      }

      void mouseDoubleClickEvent(QMouseEvent* event) override
      {
         if(event->button() == Qt::LeftButton)
         {
            m_pressed = false;
            m_dragging = false;
            m_rubberBand->hide();
            fitCoverage();
            event->accept();
            return;
         }
         QGraphicsView::mouseDoubleClickEvent(event);
      }

   private:
      FootprintClicked m_footprintClicked;
      FootprintsDragged m_footprintsDragged;
      QRubberBand* m_rubberBand;
      bool m_pressed;
      bool m_dragging;
      QPoint m_pressPosition;
      Qt::KeyboardModifiers m_pressModifiers;
      QPoint m_lastClickPosition;
      QElapsedTimer m_lastClickTimer;
      int m_clickCycle;
   };

   class CoverageListWidget : public QListWidget
   {
   public:
      using Hovered = std::function<void(int)>;

      CoverageListWidget(QWidget* parent, Hovered hovered)
      : QListWidget(parent),
        m_hovered(std::move(hovered)),
        m_hoveredRow(-1)
      {
         setMouseTracking(true);
      }

   protected:
      void mouseMoveEvent(QMouseEvent* event) override
      {
         const QModelIndex index = indexAt(event->pos());
         const int row = index.isValid() ? index.row() : -1;
         if(row != m_hoveredRow)
         {
            m_hoveredRow = row;
            if(m_hovered)
               m_hovered(row);
         }
         QListWidget::mouseMoveEvent(event);
      }

      void leaveEvent(QEvent* event) override
      {
         m_hoveredRow = -1;
         if(m_hovered)
            m_hovered(-1);
         QListWidget::leaveEvent(event);
      }

   private:
      Hovered m_hovered;
      int m_hoveredRow;
   };

   class CoveragePolygonItem : public QGraphicsPolygonItem
   {
   public:
      using Hovered = std::function<void(std::size_t, bool)>;

      CoveragePolygonItem(
         std::size_t index,
         const QPolygonF& polygon,
         const QColor& color,
         double stackingOrder,
         Hovered hovered)
      : QGraphicsPolygonItem(polygon),
        m_index(index),
        m_color(color),
        m_stackingOrder(stackingOrder),
        m_hovered(std::move(hovered))
      {
         setFlag(QGraphicsItem::ItemIsSelectable, true);
         setAcceptHoverEvents(true);
         setBrush(QBrush(QColor(color.red(), color.green(), color.blue(), 74)));
         QPen outline(color.darker(145), 2.0);
         outline.setCosmetic(true);
         setPen(outline);
         setZValue(m_stackingOrder);
      }

      std::size_t index() const
      {
         return m_index;
      }

      void applyPresentation(bool emphasized, bool related, bool selected)
      {
         const bool strong = emphasized || selected;
         setOpacity(strong ? 1.0 : (related ? 0.78 : 0.18));
         QPen outline(strong ? QColor(255, 196, 0) : m_color.darker(145),
                      strong ? 4.0 : 2.0);
         outline.setCosmetic(true);
         setPen(outline);
         setZValue(emphasized ? 1000.0 : m_stackingOrder);
      }

   protected:
      void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
      {
         if(m_hovered)
            m_hovered(m_index, true);
         QGraphicsPolygonItem::hoverEnterEvent(event);
      }

      void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override
      {
         if(m_hovered)
            m_hovered(m_index, false);
         QGraphicsPolygonItem::hoverLeaveEvent(event);
      }

   private:
      std::size_t m_index;
      QColor m_color;
      double m_stackingOrder;
      Hovered m_hovered;
   };

   double polygonArea(const QPolygonF& polygon)
   {
      if(polygon.size() < 3)
         return 0.0;
      double twiceArea = 0.0;
      for(int index = 0; index < polygon.size(); ++index)
      {
         const QPointF& first = polygon[index];
         const QPointF& second = polygon[(index + 1) % polygon.size()];
         twiceArea += first.x() * second.y() - second.x() * first.y();
      }
      return std::abs(twiceArea) * 0.5;
   }

   QPainterPath polygonPath(const QPolygonF& polygon)
   {
      QPainterPath result;
      if(!polygon.isEmpty())
      {
         result.addPolygon(polygon);
         result.closeSubpath();
      }
      return result;
   }

   bool polygonsOverlap(const QPolygonF& first, const QPolygonF& second)
   {
      if(first.isEmpty() || second.isEmpty())
         return false;
      const QPainterPath firstPath = polygonPath(first);
      const QPainterPath secondPath = polygonPath(second);
      return firstPath.intersects(secondPath) ||
             firstPath.contains(second.first()) ||
             secondPath.contains(first.first());
   }

   bool polygonContains(const QPolygonF& outer, const QPolygonF& inner)
   {
      if(outer.isEmpty() || inner.isEmpty())
         return false;
      return polygonPath(outer).contains(polygonPath(inner));
   }
}

class ossimGui::ImageCoverageWorkspace::Implementation
{
public:
   Implementation(
      ImageCoverageWorkspace* owner,
      const std::vector<ImageCoverageInput>& inputs,
      SelectionChanged selectionChanged)
   : m_owner(owner),
     m_inputs(inputs),
     m_selectionChanged(std::move(selectionChanged)),
     m_scene(new QGraphicsScene(owner)),
     m_view(new CoverageGraphicsView(
        m_scene, owner,
        [this](const QPointF& scenePoint,
               Qt::KeyboardModifiers modifiers,
               int cycle) {
           return selectFootprintAt(scenePoint, modifiers, cycle);
        },
        [this](const QPolygonF& sceneArea,
               Qt::KeyboardModifiers modifiers) {
           return selectFootprintsIn(sceneArea, modifiers);
        })),
     m_list(new CoverageListWidget(
        owner,
        [this](int row) {
           hoverChanged(
              row >= 0 ? static_cast<std::size_t>(row) : m_inputs.size(),
              row >= 0);
        })),
     m_palette(new QWidget(owner)),
     m_status(new QLabel(owner)),
     m_clearButton(new QToolButton(owner)),
     m_selectOverlappingButton(new QToolButton(owner)),
     m_selectContainedButton(new QToolButton(owner)),
     m_initialFitScheduled(false),
     m_syncing(false),
     m_hoveredIndex(inputs.size())
   {
      m_owner->setWindowTitle("Image Coverage Workspace");
      m_owner->setWindowFlag(Qt::Tool, true);
      m_owner->setAttribute(Qt::WA_DeleteOnClose);
      m_owner->setMinimumSize(760, 500);
      m_owner->resize(1050, 680);

      QVBoxLayout* layout = new QVBoxLayout(m_owner);
      QHBoxLayout* controls = new QHBoxLayout();
      QLabel* title = new QLabel(
         "Image footprints  |  geographic plane", m_owner);
      title->setToolTip(
         "All footprints share one equidistant-cylindrical map plane. "
         "The longitude seam is placed in the largest empty gap.");
      QFont titleFont = title->font();
      titleFont.setBold(true);
      title->setFont(titleFont);
      controls->addWidget(title);
      controls->addStretch(1);

      QPushButton* fitButton = new QPushButton("Fit", m_owner);
      fitButton->setToolTip("Fit all available footprints in the canvas.");
      controls->addWidget(fitButton);
      m_clearButton->setText("Clear");
      m_clearButton->setToolTip("Clear the synchronized image selection.");
      controls->addWidget(m_clearButton);
      m_selectOverlappingButton->setText("Select Overlapping");
      m_selectOverlappingButton->setToolTip(
         "Add every footprint overlapping the current selection.");
      controls->addWidget(m_selectOverlappingButton);
      m_selectContainedButton->setText("Select Contained");
      m_selectContainedButton->setToolTip(
         "Add footprints contained by the current selection.");
      controls->addWidget(m_selectContainedButton);
      QToolButton* layersButton = new QToolButton(m_owner);
      layersButton->setText("Layers");
      layersButton->setCheckable(true);
      layersButton->setChecked(false);
      layersButton->setToolTip(
         "Show the compact layer palette for stacked-footprint selection.");
      controls->addWidget(layersButton);
      layout->addLayout(controls);

      QSplitter* splitter = new QSplitter(Qt::Horizontal, m_owner);
      splitter->addWidget(m_view);
      m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
      m_list->setUniformItemSizes(true);
      m_list->setSpacing(1);
      m_list->setTextElideMode(Qt::ElideMiddle);
      m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      QVBoxLayout* paletteLayout = new QVBoxLayout(m_palette);
      paletteLayout->setContentsMargins(4, 4, 4, 4);
      QLabel* paletteTitle = new QLabel("Layers", m_palette);
      paletteLayout->addWidget(paletteTitle);
      paletteLayout->addWidget(m_list, 1);
      m_palette->setFixedWidth(230);
      m_palette->setVisible(false);
      splitter->addWidget(m_palette);
      splitter->setStretchFactor(0, 6);
      splitter->setStretchFactor(1, 1);
      layout->addWidget(splitter, 1);

      QHBoxLayout* footer = new QHBoxLayout();
      footer->addWidget(m_status, 1);
      QDialogButtonBox* closeButtons =
         new QDialogButtonBox(QDialogButtonBox::Close, m_owner);
      footer->addWidget(closeButtons);
      layout->addLayout(footer);

      buildOverlapModel();
      buildSceneAndList();

      QObject::connect(
         m_scene, &QGraphicsScene::selectionChanged,
         [this]() { sceneSelectionChanged(); });
      QObject::connect(
         m_list, &QListWidget::itemSelectionChanged,
         [this]() { listSelectionChanged(); });
      QObject::connect(
         fitButton, &QPushButton::clicked,
         [this]() { m_view->fitCoverage(); });
      QObject::connect(
         m_clearButton, &QToolButton::clicked,
         [this]() { replaceSelection(std::vector<std::size_t>()); });
      QObject::connect(
         m_selectOverlappingButton, &QToolButton::clicked,
         [this]() { selectRelated(false); });
      QObject::connect(
         m_selectContainedButton, &QToolButton::clicked,
         [this]() { selectRelated(true); });
      QObject::connect(
         layersButton, &QToolButton::toggled,
         m_palette, &QWidget::setVisible);
      QObject::connect(
         closeButtons, &QDialogButtonBox::rejected,
         m_owner, &QDialog::close);

      std::vector<std::size_t> initialSelection;
      for(const ImageCoverageInput& input : m_inputs)
      {
         if(input.selected)
            initialSelection.push_back(input.originalIndex);
      }
      setSelectedIndexes(initialSelection);
   }

   ~Implementation()
   {
      QObject::disconnect(m_scene, nullptr, nullptr, nullptr);
      QObject::disconnect(m_list, nullptr, nullptr, nullptr);
   }

   void setSelectedIndexes(const std::vector<std::size_t>& indexes)
   {
      const std::set<std::size_t> selected(indexes.begin(), indexes.end());
      m_syncing = true;
      m_scene->clearSelection();
      m_list->clearSelection();
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         const bool isSelected =
            !m_inputs[index].footprint.isEmpty() &&
            selected.find(m_inputs[index].originalIndex) != selected.end();
         if(index < m_polygonItems.size() && m_polygonItems[index])
            m_polygonItems[index]->setSelected(isSelected);
         if(QListWidgetItem* item = m_list->item(static_cast<int>(index)))
            item->setSelected(isSelected);
      }
      m_syncing = false;
      updatePresentation();
      updateStatus();
   }

   std::vector<std::size_t> selectedIndexes() const
   {
      std::vector<std::size_t> result;
      for(int row = 0; row < m_list->count(); ++row)
      {
         const QListWidgetItem* item = m_list->item(row);
         if(item && item->isSelected())
            result.push_back(m_inputs[static_cast<std::size_t>(row)].originalIndex);
      }
      return result;
   }

   void scheduleInitialFit()
   {
      if(m_initialFitScheduled)
         return;
      m_initialFitScheduled = true;
      QTimer::singleShot(
         0, m_owner, [this]() { m_view->fitCoverage(); });
   }

private:
   void buildOverlapModel()
   {
      m_overlaps.assign(
         m_inputs.size(), std::vector<bool>(m_inputs.size(), false));
      m_contains.assign(
         m_inputs.size(), std::vector<bool>(m_inputs.size(), false));
      for(std::size_t first = 0; first < m_inputs.size(); ++first)
      {
         for(std::size_t second = first + 1;
             second < m_inputs.size(); ++second)
         {
            const bool overlaps = polygonsOverlap(
               m_inputs[first].footprint, m_inputs[second].footprint);
            m_overlaps[first][second] = overlaps;
            m_overlaps[second][first] = overlaps;
            m_contains[first][second] = polygonContains(
               m_inputs[first].footprint, m_inputs[second].footprint);
            m_contains[second][first] = polygonContains(
               m_inputs[second].footprint, m_inputs[first].footprint);
         }
      }
   }

   void buildSceneAndList()
   {
      m_polygonItems.assign(m_inputs.size(), nullptr);
      std::vector<std::size_t> stackingOrder(m_inputs.size());
      for(std::size_t index = 0; index < stackingOrder.size(); ++index)
         stackingOrder[index] = index;
      std::stable_sort(
         stackingOrder.begin(), stackingOrder.end(),
         [this](std::size_t first, std::size_t second) {
            const double firstArea = polygonArea(m_inputs[first].footprint);
            const double secondArea = polygonArea(m_inputs[second].footprint);
            if(firstArea != secondArea)
               return firstArea > secondArea;
            return std::count(
                      m_overlaps[first].begin(),
                      m_overlaps[first].end(), true) >
                   std::count(
                      m_overlaps[second].begin(),
                      m_overlaps[second].end(), true);
         });
      std::vector<double> zValues(m_inputs.size(), 0.0);
      for(std::size_t rank = 0; rank < stackingOrder.size(); ++rank)
         zValues[stackingOrder[rank]] = static_cast<double>(rank);

      QRectF footprintBounds;
      bool haveFootprintBounds = false;
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         const ImageCoverageInput& input = m_inputs[index];
         QListWidgetItem* listItem = new QListWidgetItem(m_list);
         listItem->setToolTip(input.details);
         int overlapCount = 0;
         int containsCount = 0;
         int containedByCount = 0;
         for(bool overlaps : m_overlaps[index])
         {
            if(overlaps)
               ++overlapCount;
         }
         for(std::size_t other = 0; other < m_inputs.size(); ++other)
         {
            if(m_contains[index][other])
               ++containsCount;
            if(m_contains[other][index])
               ++containedByCount;
         }
         QString relationship = input.footprint.isEmpty()
            ? (input.footprintUnavailableReason.isEmpty()
                 ? QString("Footprint unavailable")
                 : input.footprintUnavailableReason)
            : QString("Overlaps %1 image(s)").arg(overlapCount);
         if(containsCount > 0)
            relationship += QString("; contains %1").arg(containsCount);
         if(containedByCount > 0)
            relationship += QString("; inside %1").arg(containedByCount);
         listItem->setText(
            QString("[%1] %2")
               .arg(index + 1)
               .arg(input.label));
         listItem->setToolTip(
            QString("%1\n%2").arg(input.details).arg(relationship));
         listItem->setForeground(QBrush(input.color.darker(150)));
         QPixmap colorMarker(14, 14);
         colorMarker.fill(input.color);
         listItem->setIcon(QIcon(colorMarker));

         if(input.footprint.isEmpty())
         {
            listItem->setFlags(
               listItem->flags() &
               ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            listItem->setForeground(QBrush(QColor(130, 130, 130)));
            continue;
         }
         const QRectF inputBounds = input.footprint.boundingRect();
         footprintBounds = haveFootprintBounds
            ? footprintBounds.united(inputBounds) : inputBounds;
         haveFootprintBounds = true;
         CoveragePolygonItem* polygonItem = new CoveragePolygonItem(
            index, input.footprint, input.color, zValues[index],
            [this](std::size_t hoveredIndex, bool entered) {
               hoverChanged(hoveredIndex, entered);
            });
         polygonItem->setToolTip(input.details);
         polygonItem->setData(0, static_cast<qulonglong>(index));
         m_scene->addItem(polygonItem);
         m_polygonItems[index] = polygonItem;

         QGraphicsSimpleTextItem* label =
            new QGraphicsSimpleTextItem(
               QString::number(index + 1), polygonItem);
         label->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
         label->setAcceptedMouseButtons(Qt::NoButton);
         label->setBrush(QBrush(QColor(30, 30, 30)));
         label->setPos(input.footprint.boundingRect().center());
      }
      if(haveFootprintBounds)
         m_scene->setSceneRect(footprintBounds);
   }

   void sceneSelectionChanged()
   {
      if(m_syncing)
         return;
      m_syncing = true;
      m_list->clearSelection();
      for(std::size_t index = 0; index < m_polygonItems.size(); ++index)
      {
         if(m_polygonItems[index] && m_polygonItems[index]->isSelected())
         {
            if(QListWidgetItem* item =
                  m_list->item(static_cast<int>(index)))
               item->setSelected(true);
         }
      }
      m_syncing = false;
      selectionChanged();
   }

   bool selectFootprintAt(
      const QPointF& scenePoint,
      Qt::KeyboardModifiers modifiers,
      int cycle)
   {
      std::vector<std::size_t> candidates;
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         if(!m_inputs[index].footprint.isEmpty() &&
            m_inputs[index].footprint.containsPoint(
               scenePoint, Qt::OddEvenFill))
         {
            candidates.push_back(index);
         }
      }
      if(candidates.empty())
      {
         const bool preserveSelection =
            modifiers.testFlag(Qt::ControlModifier) ||
            modifiers.testFlag(Qt::MetaModifier) ||
            modifiers.testFlag(Qt::ShiftModifier);
         if(!preserveSelection)
         {
            {
               QSignalBlocker blocker(m_scene);
               m_scene->clearSelection();
            }
            sceneSelectionChanged();
         }
         return false;
      }

      std::stable_sort(
         candidates.begin(), candidates.end(),
         [this](std::size_t first, std::size_t second) {
            return m_polygonItems[first]->zValue() >
                   m_polygonItems[second]->zValue();
         });
      const std::size_t selectedIndex = candidates[
         static_cast<std::size_t>(cycle) % candidates.size()];
      const bool extendSelection =
         modifiers.testFlag(Qt::ControlModifier) ||
         modifiers.testFlag(Qt::MetaModifier) ||
         modifiers.testFlag(Qt::ShiftModifier);

      {
         QSignalBlocker blocker(m_scene);
         if(!extendSelection)
            m_scene->clearSelection();
         CoveragePolygonItem* selectedItem =
            m_polygonItems[selectedIndex];
         selectedItem->setSelected(
            extendSelection ? !selectedItem->isSelected() : true);
      }
      sceneSelectionChanged();
      m_status->setText(
         candidates.size() > 1
            ? QString("Selected [%1] %2; click again to cycle through %3 "
                      "stacked footprints")
                 .arg(selectedIndex + 1)
                 .arg(m_inputs[selectedIndex].label)
                 .arg(candidates.size())
            : QString("Selected [%1] %2")
                 .arg(selectedIndex + 1)
                 .arg(m_inputs[selectedIndex].label));
      return true;
   }

   bool selectFootprintsIn(
      const QPolygonF& sceneArea,
      Qt::KeyboardModifiers modifiers)
   {
      const QPainterPath areaPath = polygonPath(sceneArea);
      std::vector<std::size_t> candidates;
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         if(m_inputs[index].footprint.isEmpty())
            continue;
         const QPainterPath footprintPath =
            polygonPath(m_inputs[index].footprint);
         if(areaPath.intersects(footprintPath) ||
            areaPath.contains(m_inputs[index].footprint.first()) ||
            footprintPath.contains(sceneArea.first()))
         {
            candidates.push_back(index);
         }
      }

      const bool extendSelection =
         modifiers.testFlag(Qt::ControlModifier) ||
         modifiers.testFlag(Qt::MetaModifier) ||
         modifiers.testFlag(Qt::ShiftModifier);
      {
         QSignalBlocker blocker(m_scene);
         if(!extendSelection)
            m_scene->clearSelection();
         for(std::size_t index : candidates)
            m_polygonItems[index]->setSelected(true);
      }
      sceneSelectionChanged();
      m_status->setText(
         QString("Selected %1 footprint(s) intersecting the drag area%2")
            .arg(candidates.size())
            .arg(extendSelection ? " and retained the prior selection" : ""));
      return !candidates.empty();
   }

   void listSelectionChanged()
   {
      if(m_syncing)
         return;
      m_syncing = true;
      for(std::size_t index = 0; index < m_polygonItems.size(); ++index)
      {
         if(m_polygonItems[index])
         {
            const QListWidgetItem* item =
               m_list->item(static_cast<int>(index));
            m_polygonItems[index]->setSelected(item && item->isSelected());
         }
      }
      m_syncing = false;
      selectionChanged();
   }

   void selectionChanged()
   {
      updatePresentation();
      updateStatus();
      if(m_selectionChanged)
         m_selectionChanged(selectedIndexes());
   }

   void replaceSelection(const std::vector<std::size_t>& localIndexes)
   {
      std::vector<std::size_t> originalIndexes;
      originalIndexes.reserve(localIndexes.size());
      for(std::size_t index : localIndexes)
      {
         if(index < m_inputs.size())
            originalIndexes.push_back(m_inputs[index].originalIndex);
      }
      setSelectedIndexes(originalIndexes);
      selectionChanged();
   }

   void selectRelated(bool containedOnly)
   {
      std::set<std::size_t> result;
      std::vector<std::size_t> selectedLocalIndexes;
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         const QListWidgetItem* item =
            m_list->item(static_cast<int>(index));
         if(item && item->isSelected())
         {
            selectedLocalIndexes.push_back(index);
            result.insert(index);
         }
      }
      for(std::size_t selectedIndex : selectedLocalIndexes)
      {
         for(std::size_t candidate = 0;
             candidate < m_inputs.size(); ++candidate)
         {
            if(containedOnly
                  ? m_contains[selectedIndex][candidate]
                  : m_overlaps[selectedIndex][candidate])
               result.insert(candidate);
         }
      }
      replaceSelection(
         std::vector<std::size_t>(result.begin(), result.end()));
   }

   void hoverChanged(std::size_t index, bool entered)
   {
      m_hoveredIndex = entered ? index : m_inputs.size();
      updatePresentation();
      updateStatus();
   }

   void updatePresentation()
   {
      const bool hasHover = m_hoveredIndex < m_inputs.size();
      for(std::size_t index = 0; index < m_polygonItems.size(); ++index)
      {
         CoveragePolygonItem* item = m_polygonItems[index];
         if(!item)
            continue;
         const bool emphasized = hasHover && index == m_hoveredIndex;
         const bool related = !hasHover ||
            (m_hoveredIndex < m_overlaps.size() &&
             m_overlaps[m_hoveredIndex][index]);
         item->applyPresentation(
            emphasized, related, item->isSelected());
      }
      for(std::size_t index = 0; index < m_inputs.size(); ++index)
      {
         QListWidgetItem* listItem =
            m_list->item(static_cast<int>(index));
         if(!listItem)
            continue;
         if(hasHover && index == m_hoveredIndex)
            listItem->setBackground(QColor(255, 222, 112));
         else if(hasHover && m_overlaps[m_hoveredIndex][index])
            listItem->setBackground(QColor(
               m_inputs[index].color.red(),
               m_inputs[index].color.green(),
               m_inputs[index].color.blue(), 55));
         else
            listItem->setBackground(QBrush());
      }
   }

   void updateStatus()
   {
      const std::size_t selectionCount = selectedIndexes().size();
      const bool hasSelection = selectionCount > 0;
      m_clearButton->setEnabled(hasSelection);
      m_selectOverlappingButton->setEnabled(hasSelection);
      m_selectContainedButton->setEnabled(hasSelection);
      if(m_hoveredIndex < m_inputs.size())
      {
         int overlapCount = 0;
         for(bool overlaps : m_overlaps[m_hoveredIndex])
         {
            if(overlaps)
               ++overlapCount;
         }
         m_status->setText(
            QString("%1 overlaps %2 image(s)")
               .arg(m_inputs[m_hoveredIndex].label)
               .arg(overlapCount));
         return;
      }
      m_status->setText(
         QString("Selected %1 of %2 image chain(s)")
            .arg(selectionCount)
            .arg(m_inputs.size()));
   }

   ImageCoverageWorkspace* m_owner;
   std::vector<ImageCoverageInput> m_inputs;
   SelectionChanged m_selectionChanged;
   QGraphicsScene* m_scene;
   CoverageGraphicsView* m_view;
   CoverageListWidget* m_list;
   QWidget* m_palette;
   QLabel* m_status;
   QToolButton* m_clearButton;
   QToolButton* m_selectOverlappingButton;
   QToolButton* m_selectContainedButton;
   std::vector<CoveragePolygonItem*> m_polygonItems;
   std::vector<std::vector<bool>> m_overlaps;
   std::vector<std::vector<bool>> m_contains;
   bool m_initialFitScheduled;
   bool m_syncing;
   std::size_t m_hoveredIndex;
};

ossimGui::ImageCoverageWorkspace::ImageCoverageWorkspace(
   QWidget* parent,
   const std::vector<ImageCoverageInput>& inputs,
   SelectionChanged selectionChanged)
: QDialog(parent),
  m_implementation(new Implementation(
     this, inputs, std::move(selectionChanged)))
{
}

ossimGui::ImageCoverageWorkspace::~ImageCoverageWorkspace()
{
   delete m_implementation;
}

void ossimGui::ImageCoverageWorkspace::setSelectedIndexes(
   const std::vector<std::size_t>& indexes)
{
   m_implementation->setSelectedIndexes(indexes);
}

std::vector<std::size_t>
ossimGui::ImageCoverageWorkspace::selectedIndexes() const
{
   return m_implementation->selectedIndexes();
}

void ossimGui::ImageCoverageWorkspace::showEvent(QShowEvent* event)
{
   QDialog::showEvent(event);
   m_implementation->scheduleInitialFit();
}
