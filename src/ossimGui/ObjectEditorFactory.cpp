#include <ossimGui/ObjectEditorFactory.h>

#include <ossimGui/BandSelectorEditor.h>
#include <ossimGui/BrightnessContrastEditor.h>
#include <ossimGui/HistogramRemapperEditor.h>
#include <ossimGui/HsiRemapperEditor.h>

#include <ossim/base/ossimObject.h>

#ifdef OSSIM_AUTOREGISTRATION_ENABLED
#include "RegistrationSetupDialog.h"
#include <ossim/registration/ossimRegistrationSourceFactory.h>
#endif

#include <algorithm>

namespace
{
   template <class Editor>
   QWidget* createObjectEditor(ossimObject* object, QWidget* parent)
   {
      Editor* editor = new Editor(parent);
      editor->setObject(object);
      return editor;
   }

   bool editorOrder(const ossimGui::ObjectEditorDescriptor& lhs,
                    const ossimGui::ObjectEditorDescriptor& rhs)
   {
      if(lhs.priority() != rhs.priority())
         return lhs.priority() > rhs.priority();
      if(lhs.displayName() != rhs.displayName())
         return lhs.displayName() < rhs.displayName();
      return lhs.name() < rhs.name();
   }
}

ossimGui::ObjectEditorDescriptor::ObjectEditorDescriptor()
   : m_name(), m_displayName(), m_description(), m_objectType(), m_priority(0),
     m_applicability(), m_creator()
{
}

ossimGui::ObjectEditorDescriptor::ObjectEditorDescriptor(
   const std::string& name,
   const std::string& displayName,
   const std::string& description,
   int priority,
   const std::string& objectType,
   const Creator& creator)
   : m_name(name), m_displayName(displayName), m_description(description),
     m_objectType(objectType), m_priority(priority), m_applicability(),
     m_creator(creator)
{
}

ossimGui::ObjectEditorDescriptor::ObjectEditorDescriptor(
   const std::string& name,
   const std::string& displayName,
   const std::string& description,
   int priority,
   const Applicability& applicability,
   const Creator& creator)
   : m_name(name), m_displayName(displayName), m_description(description),
     m_objectType(), m_priority(priority), m_applicability(applicability),
     m_creator(creator)
{
}

const std::string& ossimGui::ObjectEditorDescriptor::name() const
{
   return m_name;
}

const std::string& ossimGui::ObjectEditorDescriptor::displayName() const
{
   return m_displayName;
}

const std::string& ossimGui::ObjectEditorDescriptor::description() const
{
   return m_description;
}

const std::string& ossimGui::ObjectEditorDescriptor::objectType() const
{
   return m_objectType;
}

int ossimGui::ObjectEditorDescriptor::priority() const
{
   return m_priority;
}

bool ossimGui::ObjectEditorDescriptor::appliesTo(ossimObject* object) const
{
   if(!object)
      return false;
   if(!m_objectType.empty())
      return object->canCastTo(m_objectType.c_str());
   return m_applicability && m_applicability(object);
}

QWidget* ossimGui::ObjectEditorDescriptor::create(
   ossimObject* object,
   QWidget* parent) const
{
   return appliesTo(object) && m_creator ? m_creator(object, parent) : 0;
}

bool ossimGui::ObjectEditorDescriptor::valid() const
{
   return !m_name.empty() && !m_displayName.empty() &&
          (!m_objectType.empty() || static_cast<bool>(m_applicability)) &&
          static_cast<bool>(m_creator);
}

ossimGui::ObjectEditorFactory* ossimGui::ObjectEditorFactory::instance()
{
   static ObjectEditorFactory factory;
   return &factory;
}

ossimGui::ObjectEditorFactory::ObjectEditorFactory()
   : m_mutex(), m_editors()
{
   registerEditor(ObjectEditorDescriptor(
      "band-selector", "Band Selector",
      "Choose and order the output image bands.", 100,
      "ossimBandSelector",
      &createObjectEditor<BandSelectorEditor>));
   registerEditor(ObjectEditorDescriptor(
      "histogram-remapper", "Histogram",
      "Adjust histogram stretch and clipping.", 90,
      "ossimHistogramRemapper",
      &createObjectEditor<HistogramRemapperEditor>));
   registerEditor(ObjectEditorDescriptor(
      "hsi-remapper", "Hue / Saturation / Intensity",
      "Adjust hue, saturation, and intensity ranges.", 80,
      "ossimHsiRemapper",
      &createObjectEditor<HsiRemapperEditor>));
   registerEditor(ObjectEditorDescriptor(
      "brightness-contrast", "Brightness / Contrast",
      "Adjust display brightness and contrast.", 70,
      "ossimBrightnessContrastSource",
      &createObjectEditor<BrightnessContrastEditor>));

#ifdef OSSIM_AUTOREGISTRATION_ENABLED
   registerEditor(ObjectEditorDescriptor(
      "registration-setup", "Registration Setup",
      "Configure a registered registration source through the shared setup "
      "interfaces.", 200,
      [](ossimObject* object) {
         if(!object)
            return false;
         ossim_autoreg::RegistrationComponentDescriptor descriptor;
         return ossimRegistrationSourceFactory::instance()->getTypeDescriptor(
            object->getClassName(), descriptor);
      },
      [](ossimObject* object, QWidget* parent) {
         return createRegistrationSetupEditor(object, parent);
      }));
#endif
}

bool ossimGui::ObjectEditorFactory::registerEditor(
   const ObjectEditorDescriptor& descriptor)
{
   if(!descriptor.valid())
      return false;
   std::lock_guard<std::mutex> lock(m_mutex);
   for(ObjectEditorDescriptor& current : m_editors)
   {
      if(current.name() == descriptor.name())
      {
         current = descriptor;
         std::sort(m_editors.begin(), m_editors.end(), editorOrder);
         return true;
      }
   }
   m_editors.push_back(descriptor);
   std::sort(m_editors.begin(), m_editors.end(), editorOrder);
   return true;
}

bool ossimGui::ObjectEditorFactory::unregisterEditor(const std::string& name)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   const std::vector<ObjectEditorDescriptor>::iterator found =
      std::find_if(m_editors.begin(), m_editors.end(),
                   [&name](const ObjectEditorDescriptor& descriptor) {
                      return descriptor.name() == name;
                   });
   if(found == m_editors.end())
      return false;
   m_editors.erase(found);
   return true;
}

bool ossimGui::ObjectEditorFactory::getEditor(
   const std::string& name,
   ObjectEditorDescriptor& descriptor) const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   for(const ObjectEditorDescriptor& current : m_editors)
   {
      if(current.name() == name)
      {
         descriptor = current;
         return true;
      }
   }
   return false;
}

std::vector<ossimGui::ObjectEditorDescriptor>
ossimGui::ObjectEditorFactory::editorsFor(ossimObject* object) const
{
   std::vector<ObjectEditorDescriptor> registered;
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      registered = m_editors;
   }
   std::vector<ObjectEditorDescriptor> result;
   for(const ObjectEditorDescriptor& descriptor : registered)
   {
      if(descriptor.appliesTo(object))
         result.push_back(descriptor);
   }
   return result;
}

QWidget* ossimGui::ObjectEditorFactory::create(
   const std::string& name,
   ossimObject* object,
   QWidget* parent) const
{
   ObjectEditorDescriptor descriptor;
   return getEditor(name, descriptor) ? descriptor.create(object, parent) : 0;
}

QWidget* ossimGui::ObjectEditorFactory::createBest(
   ossimObject* object,
   QWidget* parent) const
{
   const std::vector<ObjectEditorDescriptor> descriptors = editorsFor(object);
   return descriptors.empty() ? 0 : descriptors.front().create(object, parent);
}
