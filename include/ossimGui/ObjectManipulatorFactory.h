#ifndef ossimGui_ObjectManipulatorFactory_HEADER
#define ossimGui_ObjectManipulatorFactory_HEADER

#include <ossimGui/Export.h>

#include <functional>
#include <mutex>
#include <string>
#include <vector>

class ossimObject;

namespace ossimGui
{
   class ImageScrollView;
   class ImageViewManipulator;

   class OSSIMGUI_DLL ObjectManipulatorDescriptor
   {
   public:
      typedef std::function<bool(ossimObject*, ImageScrollView*)>
         Applicability;
      typedef std::function<ImageViewManipulator*(
         ossimObject*, ImageScrollView*)> Creator;

      ObjectManipulatorDescriptor();
      ObjectManipulatorDescriptor(const std::string& name,
                                  const std::string& displayName,
                                  const std::string& description,
                                  int priority,
                                  const std::string& objectType,
                                  const Creator& creator);
      ObjectManipulatorDescriptor(const std::string& name,
                                  const std::string& displayName,
                                  const std::string& description,
                                  int priority,
                                  const Applicability& applicability,
                                  const Creator& creator);

      const std::string& name() const;
      const std::string& displayName() const;
      const std::string& description() const;
      const std::string& objectType() const;
      int priority() const;
      bool appliesTo(ossimObject* object, ImageScrollView* view) const;
      ImageViewManipulator* create(
         ossimObject* object, ImageScrollView* view) const;
      bool valid() const;

   private:
      std::string m_name;
      std::string m_displayName;
      std::string m_description;
      std::string m_objectType;
      int m_priority;
      Applicability m_applicability;
      Creator m_creator;
   };

   /**
    * Registry for image-view interaction modes selected by displayed object.
    *
    * The standard navigation manipulator is always registered. Additional
    * providers can register object-specific modes without changing
    * ImageScrollView or ImageMdiSubWindow.
    */
   class OSSIMGUI_DLL ObjectManipulatorFactory
   {
   public:
      static ObjectManipulatorFactory* instance();

      bool registerManipulator(
         const ObjectManipulatorDescriptor& descriptor);
      bool unregisterManipulator(const std::string& name);
      bool getManipulator(
         const std::string& name,
         ObjectManipulatorDescriptor& descriptor) const;
      std::vector<ObjectManipulatorDescriptor> manipulatorsFor(
         ossimObject* object,
         ImageScrollView* view) const;
      ImageViewManipulator* create(
         const std::string& name,
         ossimObject* object,
         ImageScrollView* view) const;
      ImageViewManipulator* createBest(
         ossimObject* object,
         ImageScrollView* view,
         std::string* selectedName = 0) const;

   private:
      ObjectManipulatorFactory();
      ObjectManipulatorFactory(const ObjectManipulatorFactory&) = delete;
      ObjectManipulatorFactory& operator=(
         const ObjectManipulatorFactory&) = delete;

      mutable std::mutex m_mutex;
      std::vector<ObjectManipulatorDescriptor> m_manipulators;
   };
}

#endif
