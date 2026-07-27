#include <ossimGui/ObjectManipulatorFactory.h>

#include <ossimGui/ImageScrollView.h>
#include <ossimGui/ImageViewManipulator.h>

#include <ossim/base/ossimObject.h>

#include <algorithm>

namespace
{
   bool manipulatorOrder(
      const ossimGui::ObjectManipulatorDescriptor& lhs,
      const ossimGui::ObjectManipulatorDescriptor& rhs)
   {
      if(lhs.priority() != rhs.priority())
         return lhs.priority() > rhs.priority();
      if(lhs.displayName() != rhs.displayName())
         return lhs.displayName() < rhs.displayName();
      return lhs.name() < rhs.name();
   }

   ossimGui::ImageViewManipulator* createStandardNavigation(
      ossimObject*,
      ossimGui::ImageScrollView* view)
   {
      return view ? new ossimGui::ImageViewManipulator(view) : 0;
   }
}

ossimGui::ObjectManipulatorDescriptor::ObjectManipulatorDescriptor()
   : m_name(), m_displayName(), m_description(), m_objectType(), m_priority(0),
     m_applicability(), m_creator()
{
}

ossimGui::ObjectManipulatorDescriptor::ObjectManipulatorDescriptor(
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

ossimGui::ObjectManipulatorDescriptor::ObjectManipulatorDescriptor(
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

const std::string& ossimGui::ObjectManipulatorDescriptor::name() const
{
   return m_name;
}

const std::string&
ossimGui::ObjectManipulatorDescriptor::displayName() const
{
   return m_displayName;
}

const std::string&
ossimGui::ObjectManipulatorDescriptor::description() const
{
   return m_description;
}

const std::string&
ossimGui::ObjectManipulatorDescriptor::objectType() const
{
   return m_objectType;
}

int ossimGui::ObjectManipulatorDescriptor::priority() const
{
   return m_priority;
}

bool ossimGui::ObjectManipulatorDescriptor::appliesTo(
   ossimObject* object,
   ImageScrollView* view) const
{
   if(!view)
      return false;
   if(!m_objectType.empty())
      return object && object->canCastTo(m_objectType.c_str());
   return m_applicability && m_applicability(object, view);
}

ossimGui::ImageViewManipulator*
ossimGui::ObjectManipulatorDescriptor::create(
   ossimObject* object,
   ImageScrollView* view) const
{
   return appliesTo(object, view) && m_creator ?
      m_creator(object, view) : 0;
}

bool ossimGui::ObjectManipulatorDescriptor::valid() const
{
   return !m_name.empty() && !m_displayName.empty() &&
          (!m_objectType.empty() || static_cast<bool>(m_applicability)) &&
          static_cast<bool>(m_creator);
}

ossimGui::ObjectManipulatorFactory*
ossimGui::ObjectManipulatorFactory::instance()
{
   static ObjectManipulatorFactory factory;
   return &factory;
}

ossimGui::ObjectManipulatorFactory::ObjectManipulatorFactory()
   : m_mutex(), m_manipulators()
{
   registerManipulator(ObjectManipulatorDescriptor(
      "standard-navigation",
      "Standard Navigation",
      "Pan, zoom, fit, and inspect the current image view.",
      0,
      [](ossimObject*, ImageScrollView* view) {
         return view != 0;
      },
      &createStandardNavigation));
}

bool ossimGui::ObjectManipulatorFactory::registerManipulator(
   const ObjectManipulatorDescriptor& descriptor)
{
   if(!descriptor.valid())
      return false;
   std::lock_guard<std::mutex> lock(m_mutex);
   for(ObjectManipulatorDescriptor& current : m_manipulators)
   {
      if(current.name() == descriptor.name())
      {
         current = descriptor;
         std::sort(
            m_manipulators.begin(), m_manipulators.end(), manipulatorOrder);
         return true;
      }
   }
   m_manipulators.push_back(descriptor);
   std::sort(
      m_manipulators.begin(), m_manipulators.end(), manipulatorOrder);
   return true;
}

bool ossimGui::ObjectManipulatorFactory::unregisterManipulator(
   const std::string& name)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   const std::vector<ObjectManipulatorDescriptor>::iterator found =
      std::find_if(
         m_manipulators.begin(),
         m_manipulators.end(),
         [&name](const ObjectManipulatorDescriptor& descriptor) {
            return descriptor.name() == name;
         });
   if(found == m_manipulators.end())
      return false;
   m_manipulators.erase(found);
   return true;
}

bool ossimGui::ObjectManipulatorFactory::getManipulator(
   const std::string& name,
   ObjectManipulatorDescriptor& descriptor) const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   for(const ObjectManipulatorDescriptor& current : m_manipulators)
   {
      if(current.name() == name)
      {
         descriptor = current;
         return true;
      }
   }
   return false;
}

std::vector<ossimGui::ObjectManipulatorDescriptor>
ossimGui::ObjectManipulatorFactory::manipulatorsFor(
   ossimObject* object,
   ImageScrollView* view) const
{
   std::vector<ObjectManipulatorDescriptor> registered;
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      registered = m_manipulators;
   }
   std::vector<ObjectManipulatorDescriptor> result;
   for(const ObjectManipulatorDescriptor& descriptor : registered)
   {
      if(descriptor.appliesTo(object, view))
         result.push_back(descriptor);
   }
   return result;
}

ossimGui::ImageViewManipulator*
ossimGui::ObjectManipulatorFactory::create(
   const std::string& name,
   ossimObject* object,
   ImageScrollView* view) const
{
   ObjectManipulatorDescriptor descriptor;
   return getManipulator(name, descriptor) ?
      descriptor.create(object, view) : 0;
}

ossimGui::ImageViewManipulator*
ossimGui::ObjectManipulatorFactory::createBest(
   ossimObject* object,
   ImageScrollView* view,
   std::string* selectedName) const
{
   const std::vector<ObjectManipulatorDescriptor> descriptors =
      manipulatorsFor(object, view);
   if(descriptors.empty())
      return 0;
   ImageViewManipulator* result =
      descriptors.front().create(object, view);
   if(result && selectedName)
      *selectedName = descriptors.front().name();
   return result;
}
