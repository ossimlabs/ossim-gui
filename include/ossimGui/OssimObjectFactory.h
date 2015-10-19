#ifndef ossimGuiOssimObjectFactory_HEADER
#define ossimGuiOssimObjectFactory_HEADER
#include <ossimGui/Export.h>
#include <ossim/base/ossimObjectFactory.h>

namespace ossimGui 
{
   class OSSIMGUI_DLL OssimObjectFactory : public ossimObjectFactory
   {
   public:
      static OssimObjectFactory* instance(); 
      
      /*!
       * Creates an object given a type name.
       */
      virtual ossimObject* createObject(const ossimString& typeName)const;
      
      /*!
       * Creates and object given a keyword list.
       */
      virtual ossimObject* createObject(const ossimKeywordlist& kwl,
                                        const char* prefix=0)const;
      
      /*!
       * This should return the type name of all objects in all factories.
       * This is the name used to construct the objects dynamially and this
       * name must be unique.
       */
      virtual void getTypeNameList(std::vector<ossimString>& typeList)const;
      
   protected:
      OssimObjectFactory();
      static OssimObjectFactory* m_instance;
   };
}

#endif
