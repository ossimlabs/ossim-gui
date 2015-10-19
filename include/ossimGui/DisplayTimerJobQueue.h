#ifndef ossimGuiDipalyTimerJobQueue_HEADER
#define ossimGuiDipalyTimerJobQueue_HEADER
#include <QtCore/QObject>
#include <ossim/parallel/ossimJobQueue.h>
#include <ossimGui/Export.h>
#include <QtCore/QTimerEvent>
namespace ossimGui{
   
   
   class OSSIMGUI_DLL DisplayTimerJobQueue : public ossimJobQueue
   {
   public:
      DisplayTimerJobQueue();
      virtual ~DisplayTimerJobQueue();
      virtual void add(ossimJob* job, bool guaranteeUniqueFlag=true);
      virtual ossimRefPtr<ossimJob> nextJob(bool blockIfEmptyFlag=true);
      
      
   protected:

      // this class is protected to the queue and manages the timer that processes jobs
      //
      
      class DisplayTimer : public QObject 
      {
      public:         
         DisplayTimer(DisplayTimerJobQueue* q);
         void startProcessingJobs();
         void stopProcessingJobs();
         virtual ~DisplayTimer();
         void setJobQueue(DisplayTimerJobQueue* que);
         virtual void timerEvent ( QTimerEvent * event );
         
      protected:
         DisplayTimerJobQueue* m_jobQueue;
         int m_timerId;
         int m_timerInterval;
      };
      
      
      mutable OpenThreads::Mutex m_timeJobQueueMutex; 
      DisplayTimer* m_displayTimer;

   };
   
   
}
#endif
