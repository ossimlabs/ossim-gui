#include <ossimGui/DisplayTimerJobQueue.h>
#include <iostream>
#include <QtGui/QApplication>
ossimGui::DisplayTimerJobQueue::DisplayTimerJobQueue()
:m_displayTimer(new DisplayTimer(this))

{
}
ossimGui::DisplayTimerJobQueue::~DisplayTimerJobQueue()
{
   if(m_displayTimer)
   {
      delete m_displayTimer;
      m_displayTimer = 0;
   }
}

ossimRefPtr<ossimJob> ossimGui::DisplayTimerJobQueue::nextJob(bool blockIfEmptyFlag)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_timeJobQueueMutex);
   ossimRefPtr<ossimJob> result = ossimJobQueue::nextJob(false);
   if(!result.valid()&&m_displayTimer)
   {
      m_displayTimer->stopProcessingJobs();
   }
   
   return result;
}

void ossimGui::DisplayTimerJobQueue::add(ossimJob* job, bool guaranteeUniqueFlag)
{
   ossimJobQueue::add(job, guaranteeUniqueFlag);
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_timeJobQueueMutex);
   if(m_displayTimer) m_displayTimer->startProcessingJobs();
}


ossimGui::DisplayTimerJobQueue::DisplayTimer::DisplayTimer(DisplayTimerJobQueue* q)
:m_jobQueue(q),
m_timerId(-1),
m_timerInterval(10)
{
}

ossimGui::DisplayTimerJobQueue::DisplayTimer::~DisplayTimer()
{
   m_jobQueue = 0;
   if(m_timerId >=0)
   {
      killTimer(m_timerId);
      m_timerId = -1;
   }
}

void ossimGui::DisplayTimerJobQueue::DisplayTimer::startProcessingJobs()
{
   if(m_timerId < 0)
   {
      m_timerId = startTimer(m_timerInterval);
   }
}

void ossimGui::DisplayTimerJobQueue::DisplayTimer::stopProcessingJobs()
{
   if(m_timerId >= 0)
   {
      killTimer(m_timerId);
      m_timerId = -1;
   }
}

void ossimGui::DisplayTimerJobQueue::DisplayTimer::setJobQueue(DisplayTimerJobQueue* que)
{
   m_jobQueue = que;
}

void ossimGui::DisplayTimerJobQueue::DisplayTimer::timerEvent ( QTimerEvent * event )
{
   if(event)
   {
      if((event->timerId() == m_timerId)&&m_jobQueue)
      {
         ossimRefPtr<ossimJob> job = m_jobQueue->nextJob(); 
         if(job.valid())
         {
            QApplication::processEvents();
            if(!job->isCanceled())
            {
               job->running();
               job->start();
            }
            job->finished(); // turn on finished
            job = 0;
         }
         else 
         {
            
         }
         
      }
   }
}

