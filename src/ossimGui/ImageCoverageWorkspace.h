#ifndef ossimGui_ImageCoverageWorkspace_HEADER
#define ossimGui_ImageCoverageWorkspace_HEADER

#include <QColor>
#include <QDialog>
#include <QPolygonF>

#include <cstddef>
#include <functional>
#include <vector>

class QShowEvent;

namespace ossimGui
{
   struct ImageCoverageInput
   {
      QString label;
      QString details;
      QString footprintUnavailableReason;
      QPolygonF footprint;
      QColor color;
      std::size_t originalIndex;
      bool selected;

      ImageCoverageInput()
      : originalIndex(0), selected(false)
      {
      }
   };

   class ImageCoverageWorkspace : public QDialog
   {
   public:
      using SelectionChanged =
         std::function<void(const std::vector<std::size_t>&)>;

      ImageCoverageWorkspace(
         QWidget* parent,
         const std::vector<ImageCoverageInput>& inputs,
         SelectionChanged selectionChanged);
      ~ImageCoverageWorkspace() override;

      void setSelectedIndexes(const std::vector<std::size_t>& indexes);
      std::vector<std::size_t> selectedIndexes() const;

   protected:
      void showEvent(QShowEvent* event) override;

   private:
      class Implementation;
      Implementation* m_implementation;
   };
}

#endif
