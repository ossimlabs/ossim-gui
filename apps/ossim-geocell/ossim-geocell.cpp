// $Id$

#include <iostream>
#include <QtGui/QApplication>
#include <QtGui/QSplashScreen>
#include <QtCore/QThread>
#include <ossimGui/MainWindow.h>
#include <ossimGui/OssimObjectFactory.h>
#include <ossim/base/ossimArgumentParser.h>
#include <ossim/base/ossimApplicationUsage.h>
#include <ossim/base/ossimObjectFactoryRegistry.h>
#include <ossim/init/ossimInit.h>
#include <OpenThreads/Thread>
#ifdef OSSIMQT_USE_WINDOWS_STYLE
#include <QtGui/QWindowsStyle>
#endif
#include <ossim/base/ossimEnvironmentUtility.h>

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
   ossimInit::instance()->addOptions(argumentParser);
   ossimInit::instance()->initialize(argumentParser);
   argumentParser.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   argumentParser.getApplicationUsage()->setApplicationName(argumentParser.getApplicationName());
   argumentParser.getApplicationUsage()->setDescription(argumentParser.getApplicationName()+" GUI application for the ossim core library");
   argumentParser.getApplicationUsage()->setCommandLineUsage(argumentParser.getApplicationName()+" [options]");
   argumentParser.getApplicationUsage()->addCommandLineOption("-project","OPTIONAL: project file");
   argumentParser.getApplicationUsage()->addCommandLineOption("No '-','*.gcl' file","untagged project file with 'gcl' extension");
  
   if (argumentParser.read("-h") || argumentParser.read("--help"))
   {
      argumentParser.getApplicationUsage()->write(std::cout);
      exit(0);
   }

   // project file
   std::string tempString;
   ossimArgumentParser::ossimParameter stringParam(tempString);
   ossimString projFile;
   while(argumentParser.read("-project", stringParam))
   {
      projFile = tempString;
   }

   // additional check for stand-alone ".gcl" project file
   // or list of images
   std::vector<ossimString> ilist;
   if (argc > 1)
   {
      for (int k=1; k<argc; ++k)
      {
         tempString = argv[k];
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

   OpenThreads::Thread::microSleep((1000*1000));
   //splash.finish(mainWindow);
   splash.close();
   int result = app.exec();
   ossimInit::instance()->finalize();

   return result;
}
