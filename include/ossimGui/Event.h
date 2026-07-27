#ifndef ossimGuiEvent_HEADER
#define ossimGuiEvent_HEADER
#include <QtCore/QEvent>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/parallel/ossimJob.h>
#include <ossimGui/DataManager.h>
#include <string>
namespace ossimGui{
   enum EventId
   {
      IMAGE_OPEN_EVENT_ID              = QEvent::User,
      DATA_MANAGER_EVENT_ID            = (IMAGE_OPEN_EVENT_ID+1),
      DATA_MANAGER_WIDGET_EVENT_ID     = (DATA_MANAGER_EVENT_ID+1),
      DATA_MANAGER_WIDGET_JOB_EVENT_ID = (DATA_MANAGER_WIDGET_EVENT_ID+1),
      WINDOW_REFRESH_ACTIONS_EVENT_ID  = (DATA_MANAGER_WIDGET_JOB_EVENT_ID+1),
      PROGRESS_EVENT_ID                = (WINDOW_REFRESH_ACTIONS_EVENT_ID+1),
      DATA_MANGER_LAST_ID              = (PROGRESS_EVENT_ID+1)
   };
   
   class OSSIMGUI_DLL ImageOpenEvent : public QEvent
   {
   public:
      typedef std::vector<ossimRefPtr<ossimImageHandler> > HandlerList;
      
      ImageOpenEvent():QEvent((QEvent::Type)IMAGE_OPEN_EVENT_ID)
      {
      }
      ImageOpenEvent(ossimImageHandler* handler):QEvent((QEvent::Type)IMAGE_OPEN_EVENT_ID)
      {
         if(handler) m_handlerList.push_back(handler);
      }
      ImageOpenEvent(HandlerList& handlers):QEvent((QEvent::Type)IMAGE_OPEN_EVENT_ID)
      {
         m_handlerList = handlers;
      }
      HandlerList& handlerList(){return m_handlerList;}
      const HandlerList& handlerList()const{return m_handlerList;}
   protected:
      HandlerList m_handlerList;
   };
   
   class OSSIMGUI_DLL DataManagerEvent : public QEvent
   {
   public:
      enum CommandType
      {
         COMMAND_NONE = 0,
         COMMAND_DISPLAY_NODE = 1,
         COMMAND_NODE_ADDED = 2,
         COMMAND_NODE_REMOVED = 3
      };
      DataManagerEvent(int commandType=COMMAND_DISPLAY_NODE):QEvent((QEvent::Type)DATA_MANAGER_EVENT_ID),
      m_commandType(static_cast<CommandType>(commandType))
      {
      }
      
      void setNodeList(DataManager::Node* node){m_nodeList.clear();if(node) m_nodeList.push_back(node);}
      void setNodeList(const DataManager::NodeListType& nodes){m_nodeList = nodes;}
      DataManager::NodeListType& nodeList(){return m_nodeList;}
      const DataManager::NodeListType& nodeList()const{return m_nodeList;}
      void setCommand(int commandType){m_commandType = static_cast<CommandType>(commandType);}
      int command(){return m_commandType;}
     
   protected:
      DataManager::NodeListType m_nodeList;
      CommandType               m_commandType;
   };
   
   class DataManagerNodeItem;
   class OSSIMGUI_DLL DataManagerWidgetEvent : public QEvent
   {
   public:
      typedef std::vector<DataManagerNodeItem*> ItemListType;
      typedef std::vector<ossimRefPtr<ossimImageHandler> > HandlerListType;
      enum CommandType
      {
         COMMAND_NONE = 0,
         COMMAND_DELETE_NODE = 1,
         COMMAND_REFRESH = 2,
         COMMAND_DISCONNECT_INPUT = 3,
         COMMAND_CONNECT_INPUT = 4,
         COMMAND_RESET = 5
      };
      DataManagerWidgetEvent(int commandType=COMMAND_NONE)
      :QEvent(static_cast<QEvent::Type>(DATA_MANAGER_WIDGET_EVENT_ID)),
      m_registrationStatus(),
      m_registrationSummary(),
      m_registrationAdvisory(),
      m_commandType(static_cast<CommandType>(commandType))
      {
      }
      void setCommand(int commandType){m_commandType = static_cast<CommandType>(commandType);}
      int command(){return m_commandType;}
      void setItemList(DataManagerNodeItem* item){m_itemList.clear();if(item) m_itemList.push_back(item);}
      void setItemList(const ItemListType& items){m_itemList = items;}
      ItemListType& itemList(){return m_itemList;}
      const ItemListType& itemList()const{return m_itemList;}
      void setHandlerList(const HandlerListType& handlers){m_handlerList = handlers;}
      HandlerListType& handlerList(){return m_handlerList;}
      const HandlerListType& handlerList()const{return m_handlerList;}
      void setWarningMessage(const std::string& title,
                             const std::string& message)
      {
         m_warningTitle = title;
         m_warningMessage = message;
      }
      const std::string& warningTitle()const{return m_warningTitle;}
      const std::string& warningMessage()const{return m_warningMessage;}
      void setRegistrationReport(const std::string& text,
                                 const std::string& path)
      {
         m_registrationReportText = text;
         m_registrationReportPath = path;
      }
      const std::string& registrationReportText()const
      {
         return m_registrationReportText;
      }
      const std::string& registrationReportPath()const
      {
         return m_registrationReportPath;
      }
      void setRegistrationResult(const std::string& status,
                                 const std::string& summary,
                                 const std::string& advisory)
      {
         m_registrationStatus = status;
         m_registrationSummary = summary;
         m_registrationAdvisory = advisory;
      }
      const std::string& registrationStatus()const
      {
         return m_registrationStatus;
      }
      const std::string& registrationSummary()const
      {
         return m_registrationSummary;
      }
      const std::string& registrationAdvisory()const
      {
         return m_registrationAdvisory;
      }
      
   protected:
      ItemListType m_itemList;
      HandlerListType m_handlerList;
      std::string m_warningTitle;
      std::string m_warningMessage;
      std::string m_registrationReportText;
      std::string m_registrationReportPath;
      std::string m_registrationStatus;
      std::string m_registrationSummary;
      std::string m_registrationAdvisory;
      CommandType m_commandType;
   };
   class OSSIMGUI_DLL DataManagerWidgetJobEvent : public QEvent
   {
   public:
      typedef std::vector<std::shared_ptr<ossimJob> > JobListType;
      enum CommandType
      {
         COMMAND_NONE = 0,
         COMMAND_JOB_ADD,
         COMMAND_JOB_ADDED,
         COMMAND_JOB_STATE_CHANGED,
         COMMAND_JOB_PROPERTY_CHANGED,
         COMMAND_JOB_PERCENT_COMPLETE
      };
      DataManagerWidgetJobEvent(int commandType=COMMAND_NONE)
      :QEvent(static_cast<QEvent::Type>(DATA_MANAGER_WIDGET_JOB_EVENT_ID)),
      m_commandType(static_cast<CommandType>(commandType)),
      m_percentComplete(0.0)
      {}
      void setCommand(int commandType){m_commandType = static_cast<CommandType>(commandType);}
      int command(){return m_commandType;}
      void setJobList(std::shared_ptr<ossimJob> job){m_jobList.clear();if(job) m_jobList.push_back(job);}
      void setJobList(const JobListType& jobs){m_jobList = jobs;}
      JobListType& jobList(){return m_jobList;}
      const JobListType& jobList()const{return m_jobList;}
      void setJobList(ossim_float64 percentComplete, std::shared_ptr<ossimJob> job){setJobList(job);m_percentComplete=percentComplete;}
      ossim_float64 percentComplete()const{return m_percentComplete;}
      
   protected:
      JobListType m_jobList;
      CommandType m_commandType;
      ossim_float64 m_percentComplete;
   };

   class OSSIMGUI_DLL ProgressEvent : public QEvent
   {
   public:
      ProgressEvent(double value=0.0)
      :QEvent(static_cast<QEvent::Type>(PROGRESS_EVENT_ID)),
      m_percentComplete(value)
      {
      }
      void setPercentComplete(double value){m_percentComplete = value;}
      double percentComplete()const{return m_percentComplete;}
      
   protected:
      double m_percentComplete;
   };

}
#endif
