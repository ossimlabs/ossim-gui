#ifndef ossimGui_ObjectEditorFactory_HEADER
#define ossimGui_ObjectEditorFactory_HEADER

#include <ossimGui/Export.h>

#include <functional>
#include <mutex>
#include <string>
#include <vector>

class QWidget;
class ossimObject;

namespace ossimGui
{
   class OSSIMGUI_DLL ObjectEditorDescriptor
   {
   public:
      typedef std::function<bool(ossimObject*)> Applicability;
      typedef std::function<QWidget*(ossimObject*, QWidget*)> Creator;

      ObjectEditorDescriptor();
      ObjectEditorDescriptor(const std::string& name,
                             const std::string& displayName,
                             const std::string& description,
                             int priority,
                             const std::string& objectType,
                             const Creator& creator);
      ObjectEditorDescriptor(const std::string& name,
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
      bool appliesTo(ossimObject* object) const;
      QWidget* create(ossimObject* object, QWidget* parent) const;
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
    * Registry for optional, object-specific GUI editors.
    *
    * The registry only selects tailored editors. Callers must keep the generic
    * ossimPropertyInterface editor available as the universal fallback.
    */
   class OSSIMGUI_DLL ObjectEditorFactory
   {
   public:
      static ObjectEditorFactory* instance();

      bool registerEditor(const ObjectEditorDescriptor& descriptor);
      bool unregisterEditor(const std::string& name);
      bool getEditor(const std::string& name,
                     ObjectEditorDescriptor& descriptor) const;
      std::vector<ObjectEditorDescriptor> editorsFor(
         ossimObject* object) const;
      QWidget* create(const std::string& name,
                      ossimObject* object,
                      QWidget* parent = 0) const;
      QWidget* createBest(ossimObject* object, QWidget* parent = 0) const;

   private:
      ObjectEditorFactory();
      ObjectEditorFactory(const ObjectEditorFactory&) = delete;
      ObjectEditorFactory& operator=(const ObjectEditorFactory&) = delete;

      mutable std::mutex m_mutex;
      std::vector<ObjectEditorDescriptor> m_editors;
   };
}

#endif
