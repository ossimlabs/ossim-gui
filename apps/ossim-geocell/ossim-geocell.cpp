// $Id$

#include <iostream>
#include <QApplication>
#include <QSplashScreen>
#include <QtCore/QThread>
#include <ossimGui/MainWindow.h>
#include <ossimGui/OssimObjectFactory.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#ifdef OSSIMQT_USE_WINDOWS_STYLE
#include <QtGui/QWindowsStyle>
#endif
#include <ossim/base/ossimEnvironmentUtility.h>
#include <thread>

#ifdef WIN32
int main(int argc, char *argv[]);
#include <windows.h>
#include <shellapi.h>
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
  typedef int (__cdecl *GETMAINARGS)(int*, char***, char***, int, int*);

  int     argc;
  char**  argv;
  char**  env;
  int     new_mode = 0;
  GETMAINARGS getmainargs;

  getmainargs = (GETMAINARGS) GetProcAddress(LoadLibrary("msvcrt"),"__getmainargs");
  getmainargs(&argc,&argv,&env,0,&new_mode);
  int ret = main(argc,argv);
  
  return ret;
}
#endif

int main(int argc, char *argv[])
{
   ossimArgumentParser argumentParser(&argc, argv);
   auto parseViewMode = [](const std::string& value, ossimGui::DataManager::ViewModeType& mode) -> bool
   {
      ossimString modeString(value);
      modeString = modeString.downcase();
      if(modeString == "auto")
      {
         mode = ossimGui::DataManager::VIEW_MODE_AUTO;
         return true;
      }
      if(modeString == "geo")
      {
         mode = ossimGui::DataManager::VIEW_MODE_GEO;
         return true;
      }
      if(modeString == "image")
      {
         mode = ossimGui::DataManager::VIEW_MODE_IMAGE;
         return true;
      }
      return false;
   };
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimString projFile;
   ossimGui::DataManager::ViewModeType viewMode = ossimGui::DataManager::VIEW_MODE_AUTO;
   std::vector<std::string> rawArgs;
   for(int argIdx = 1; argIdx < argc; ++argIdx)
   {
      rawArgs.push_back(argv[argIdx]);
   }
   while(argumentParser.read("-project", stringParam))
   {
      projFile = tempString;
   }
   while(argumentParser.read("--view-mode", stringParam) ||
         argumentParser.read("-view-mode", stringParam))
   {
      if(!parseViewMode(tempString, viewMode))
      {
         std::cerr << "Invalid --view-mode value: " << tempString
                   << " (expected auto, geo, or image)\n";
         return 1;
      }
   }
   for(const auto& rawArg : rawArgs)
   {
      const std::string viewModePrefixLong = "--view-mode=";
      const std::string viewModePrefixShort = "-view-mode=";
      const std::string projectPrefix = "-project=";
      if(rawArg.rfind(viewModePrefixLong, 0) == 0)
      {
         const std::string value = rawArg.substr(viewModePrefixLong.size());
         if(!parseViewMode(value, viewMode))
         {
            std::cerr << "Invalid --view-mode value: " << value
                      << " (expected auto, geo, or image)\n";
            return 1;
         }
      }
      else if(rawArg.rfind(viewModePrefixShort, 0) == 0)
      {
         const std::string value = rawArg.substr(viewModePrefixShort.size());
         if(!parseViewMode(value, viewMode))
         {
            std::cerr << "Invalid --view-mode value: " << value
                      << " (expected auto, geo, or image)\n";
            return 1;
         }
      }
      else if(rawArg.rfind(projectPrefix, 0) == 0)
      {
         projFile = rawArg.substr(projectPrefix.size());
      }
   }
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" GUI application for the ossim core library");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options]");
   argumentParser.getApplicationUsage()->addCommandLineOption("-project","OPTIONAL: project file");
   argumentParser.getApplicationUsage()->addCommandLineOption("--view-mode <auto|geo|image>","OPTIONAL: choose geospatial or image-space viewing");
   argumentParser.getApplicationUsage()->addCommandLineOption("No '-','*.gcl' file","untagged project file with 'gcl' extension");
  
   if (argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }

   // additional check for stand-alone ".gcl" project file
   // or list of images
   std::vector<ossimString> ilist;
   if (argc > 1)
   {
      for (int k=1; k<argc; ++k)
      {
         tempString = argv[k];
         if ((tempString.find("--view-mode=") == 0) ||
             (tempString.find("-view-mode=") == 0) ||
             (tempString == "--view-mode") ||
             (tempString == "-view-mode") ||
             (tempString.find("-project=") == 0) ||
             (tempString == "-project"))
         {
            continue;
         }
         if (tempString.find(".gcl") != std::string::npos)
         {
            projFile = tempString;
         }
         else
         {
            ilist.push_back(tempString);
         }
      }
   }


   argumentParser.reportRemainingOptionsAsUnrecognized();   
   QApplication app(argc, argv);
   QSplashScreen splash(QPixmap(":/splash/GeoCellSplash.png"));
   splash.setWindowFlags(splash.windowFlags()|Qt::WindowStaysOnTopHint);
   splash.show();
#ifdef OSSIMQT_USE_WINDOWS_STYLE
   QWindowsStyle *style = new QWindowsStyle();
   app.setStyle(style);
#endif
   ossimObjectFactoryRegistry::instance()->registerFactory(ossimGui::OssimObjectFactory::instance());
   ossimGui::MainWindow*  mainWindow = new ossimGui::MainWindow();
   mainWindow->setViewMode(viewMode);
   
   // Load command line project file or image files if present
   if (projFile.size()>0)
   {
      mainWindow->loadProjectFile(projFile);
   }
   else if (ilist.size()>0)
   {
      mainWindow->loadImageFileList(ilist);
   }
   mainWindow->show();

   std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   //splash.finish(mainWindow);
   splash.close();
   int result = app.exec();
   ossimInit::instance()->finalize();

   return result;
}
