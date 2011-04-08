#ifndef __KERNELTIMER_H
#define __KERNELTIMER_H

#include "Timer.hpp"

typedef long long kernel_timer_t;

/*
 * Implements a high precision timer using the kernel's
 * gettimeofday () system call.
 *
 * WARNING
 * The POSIX gettimeofday () is NOT thread safe!  By proxy,
 * KernelTimer's update () and sleepyTime () methods are not 
 * thread safe.
 *
 * Only one thread should call the timer's update () or
 * sleepyTime () methods unprotected.
 */
class KernelTimer : public Timer <kernel_timer_t>
{
public:
   static const kernel_timer_t SECOND = 1000000;

private:
   class KernelTimerCallback : public Timer <kernel_timer_t>::TimeFunctor {
   public:
      KernelTimerCallback ();

      kernel_timer_t operator() () const;

   private:
      static struct timeval _tv;

   };
   
public:
   KernelTimer ();
   void sleepyTime () const;

};

#endif

