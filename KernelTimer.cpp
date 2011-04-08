/*
 * KernelTimer: A high precision Timer template specialization
 *    using the gettimeofday () system call.
 *
 * (c) 2011 Lee Supe (lain_proliant)
 *
 * Released under the GNU General Public License, version 3.
 */

#include <cstring>
#include <sys/time.h>
#include "KernelTimer.hpp"

struct timeval KernelTimer::KernelTimerCallback::_tv;

/*
 * KernelTimerCallback; Constructor; default.
 */
KernelTimer::KernelTimerCallback::KernelTimerCallback ()
{ }


/*
 * operator (); Functor method specializing Timer::TimeFunctor.
 * Calls the kernel system call gettimeofday () to determine
 * the current time ticks, in microseconds.
 */
kernel_timer_t KernelTimer::KernelTimerCallback::operator() () const {
   gettimeofday (&_tv, NULL);
   return SECOND * (kernel_timer_t)_tv.tv_sec + (kernel_timer_t)_tv.tv_usec;
}


/*
 * KernelTimer; Constructor; default.
 */
KernelTimer::KernelTimer () : Timer <kernel_timer_t> (
      new KernelTimer::KernelTimerCallback ())
{ }


/*
 * Uses nanosleep () to attempt to sleep for the exact amount
 * of time remaining until the timer elapses.
 *
 * This method is useful for timed processes which need to use as
 * little CPU resources as possible by sacrificing a bit of accuracy.
 *
 * Uses waitTime (), which must call gettimeofday () to determine the
 * amount of time remaining.
 *
 * Note that this method will almost always sleep too long.  The error 
 * will be noted and deducted from the next timer interval.
 * 
 * WARNING
 * This method is NOT thread safe!  See KernelTimer.hpp for more info.
 */
void KernelTimer::sleepyTime () const
{
   static struct timespec req, rem;

   kernel_timer_t twait = waitTime ();

   req.tv_sec = twait / SECOND;
   req.tv_nsec = (twait % SECOND) * 1000;

   while (nanosleep (&req, &rem) == -1) {
      memcpy (&req, &rem, sizeof (struct timespec));
   }
}

