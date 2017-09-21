#include <ossimGui/DisplayTimerJobQueue.h>
#include <iostream>
#include <QApplication>
ossimGui::DisplayTimerJobQueue::DisplayTimerJobQueue()
:m_displayTimer(0)
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

std::shared_ptr<ossimJob> ossimGui::DisplayTimerJobQueue::nextJob(bool blockIfEmptyFlag)
{
   std::lock_guard<std::mutex> lock(m_timeJobQueueMutex);
   std::shared_ptr<ossimJob> result = ossimJobQueue::nextJob(false);
   if(!result&&m_displayTimer)
   {
      m_displayTimer->stopProcessingJobs();
   }
   
   return result;
}

void ossimGui::DisplayTimerJobQueue::add(std::shared_ptr<ossimJob> job, bool guaranteeUniqueFlag)
{
   if(!m_displayTimer)
   {
      m_displayTimer = new DisplayTimer(std::static_pointer_cast<DisplayTimerJobQueue>(getSharedFromThis()));
   }
   ossimJobQueue::add(job, guaranteeUniqueFlag);
   std::lock_guard<std::mutex> lock(m_timeJobQueueMutex);
   if(m_displayTimer) m_displayTimer->startProcessingJobs();
}


ossimGui::DisplayTimerJobQueue::DisplayTimer::DisplayTimer(std::shared_ptr<DisplayTimerJobQueue> q)
:m_jobQueue(q),
m_timerId(-1),
m_timerInterval(10)
{
}

ossimGui::DisplayTimerJobQueue::DisplayTimer::~DisplayTimer()
{
   m_jobQueue.reset();
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

void ossimGui::DisplayTimerJobQueue::DisplayTimer::setJobQueue(std::shared_ptr<DisplayTimerJobQueue> que)
{
   m_jobQueue = que;
}

void ossimGui::DisplayTimerJobQueue::DisplayTimer::timerEvent ( QTimerEvent * event )
{
   if(event)
   {
      if((event->timerId() == m_timerId)&&m_jobQueue)
      {
         std::shared_ptr<ossimJob> job = m_jobQueue->nextJob(); 
         if(job)
         {
            QApplication::processEvents();
            if(!job->isCanceled())
            {
               job->start();
            }
            job = 0;
         }
         else 
         {
            
         }
         
      }
   }
}

