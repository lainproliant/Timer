/*
 * Timer: An error-correcting high precision timer 
 *        helper class for non-realtime event triggering.
 *
 * (c) 2011 Lee Supe (lain_proliant)
 * Released under the GNU General Public License, version 3
 */

#ifndef __TIMER_HPP
#define __TIMER_HPP

using namespace std;

/********************************************************************
 * Timer: A high-precision timer helper class.
 */
template <typename T> class Timer {
public:
   typedef T (*timer_proc_t) ();
   
   /*
    * A functor providing time units for a Timer.
    */
   class TimeFunctor {
   public:
      virtual T operator() () const = 0;
   };
   
   Timer (TimeFunctor* timeFunc);
   Timer (timer_proc_t timerProc);
   virtual ~Timer ();

   bool update ();

   void start (T interval);
   void stop ();

   bool started () const;

   void pause ();
   void resume ();

   bool paused () const;

   T elapsed () const;
   T getTicks () const;
   T getFrames () const;

   T waitTime () const;
   
   /* VIRTUAL
    * This method is not abstract, and does not have to be implemented,
    * but is suggested.  Ask the timer to find a way to sleep for the
    * amount of time specified by waitTime ().  For examples, see
    * SDLTimer (using SDL_Delay) or KernelTimer (using nanosleep).
    *
    * By default, this method does nothing.
    */
   virtual void sleepyTime () const
   { }

   void reset ();

   // LRS-DEBUG: remove this method.
   void debugPrint ();

protected:
   void notifyListeners ();
   
   /*
    * A functor wrapper for a callback method providing
    * time units for a Timer.
    */
   class CallbackWrapper : public TimeFunctor {
   public:
      CallbackWrapper (timer_proc_t proc) {
         _proc = proc;
      }

      T operator() () const {
         return _proc ();
      }
   private:
      timer_proc_t _proc;
   };

   /*
    * A functor wrapper for a second timer.  The resulting timer's
    * ticks are relative to the given timer's frames.
    */
   class RelativeTimeWrapper : public TimeFunctor {
   public:
      const Timer<T>& referenceTimer;

      RelativeTimeWrapper (const Timer<T>& timer) {
         referenceTimer = timer;
      }

      T operator() () const {
         return referenceTimer.getFrames ();
      }
   };
   
private:
   void _init ();

   TimeFunctor* _timeFunc;
   T _interval, _t0, _t1, _terr, _tstart, _frames;

   bool _started, _paused;
};


/********************************************************************
 * Timer: Implementation
 */

/*
 * Constructor; accepting a time functor.
 */
template <typename T> Timer<T>::Timer (Timer<T>::TimeFunctor* timeFunc) : 
   _timeFunc (timeFunc)
{
   this->_init ();
}


/*
 * Constructor; accepting a callback method.
 *
 * timerProc:     A function pointer conforming to Timer<T>::timer_proc_t,
 *                i.e. a method that takes no parameters and returns the
 *                templatized type of the timer's time units.
 */
template <typename T> Timer<T>::Timer (Timer<T>::timer_proc_t timerProc) :
   _timeFunc (new typename Timer<T>::CallbackWrapper (timerProc))
{
   this->_init ();
}


/*
 * PRIVATE
 * Shared initializer for constructors.
 */
template <typename T> void Timer<T>::_init ()
{
   _interval = 0;

   this->reset ();

   _started = false;
   _paused = false;
}


/*
 * Virtual destructor.  No cleanup necessary.
 */
template <typename T> Timer<T>::~Timer ()
{ 
   delete this->_timeFunc;
}


/*
 * Updates the timer and increments the frame counter if
 * the timer interval has elapsed.
 *
 * Returns True if the timer interval has elapsed,
 * or False if the timer interval has not elapsed
 * or if the timer is paused.
 */
template <typename T> bool Timer<T>::update ()
{
   if (! this->started () || this->paused ()) {
      // If the timer is stopped or paused, skip updating.
      return false;
   }

   T tnow = (*this->_timeFunc) ();

   if (tnow < _t0) {
      // The timer function has wrapped.
      // TODO: Find a better solution to timer wrapping.
      // For now, let's just reset the timer.
      // Wrapping will occur so rarely in most functions
      // that this should not be a huge problem.
     
      _t0 = tnow;
      _t1 = tnow;
      _tstart = tnow;

      return false;
   }

   // If the error accumulator has more time left than
   // the interval, clear the accumulator and return
   // true immediately.
   //
   // We should only make up for as much as one total
   // missed tick.  If we simply decrement the accumulator,
   // we will eventually overflow the accumulator if we
   // try to fire the timer more often than the system
   // can keep up.
  
   if (_interval < _terr) {
      _terr = 0;
      _t0 = _t1;
      _t1 = tnow;
      _frames ++;

      return true;
   }

   if (tnow - _t0 >= _interval - _terr) {
      // If more time has elapsed than the timer interval,
      // reset the timer deltas and add any error in timing
      // to the error accumulator.  Return True to signify
      // that the designated time has elapsed.

      if (_interval - _terr > tnow - _t0) {
         _terr = 0;
      } else {
         _terr = (tnow - _t0) - (_interval + _terr);
      }

      _t1 = tnow;
      _t0 = _t1;
      _frames ++;

      return true;

   } else {
      // The timer interval has not elapsed.  Return False.
      return false;
   }
}


/*
 * Starts the timer with the given interval.  Assigns the new
 * timer interval, resets the time values, and resumes the 
 * timer if it was paused.
 * 
 * interval:      The interval, in T units, to assign to the tiemr.
 */
template <typename T> void Timer<T>::start (T interval)
{
   _interval = interval;

   this->reset ();
   this->resume ();

   _started = true;
}


/*
 * Stops the timer.  This will cause the timer to forget
 * its time interval, and will no longer update until
 * start () is called again with a new interval.
 */
template <typename T> void Timer<T>::stop ()
{
   _interval = 0;

   _started = false;
}


/*
 * Determines if the timer has been started.
 *
 * Returns True if the timer has been started, False otherwise.
 */
template <typename T> bool Timer<T>::started () const
{
   return _started;
}


/*
 * Pauses the timer.  The timer will remember its interval,
 * and updating will resume after resume () is called.
 */
template <typename T> void Timer<T>::pause ()
{
   _paused = true;
}


/*
 * Resumes the timer.  If the timer was paused, the internal
 * time values are updated so that the same time elapsed
 * before the pause occured is shown.
 */
template <typename T> void Timer<T>::resume ()
{
   if (this->paused ()) {
      // Update the internal time values to reflect the
      // current time.
      
      T dt = _t1 - _t0;
      T tnow = (*this->_timeFunc) ();

      _t0 = tnow;
      _t1 = tnow + dt;

      _paused = false;
   }
}


/*
 * Determines if the timer is paused.
 *
 * Returns True if the timer is paused, False otherwise.
 */
template <typename T> bool Timer<T>::paused () const
{
   return _paused;
}


/*
 * Determines how long the timer has been running.
 *
 * Returns the amount of time, in units T, the timer
 * has been running or was running before stopped.
 */
template <typename T> T Timer<T>::elapsed () const
{
   return _t1 - _tstart;
}


/*
 * Returns the current ticks as reported by the timer functor
 * since the last call to update ().
 *
 * Used by slave timers to compute time deltas, in lieu of
 * calling the timer proc again.  This helps to keep all of the
 * objects in a system operating on the same time frame.
 */
template <typename T> T Timer<T>::getTicks () const
{
   return _t1;
}


/*
 * Returns the number of frames, or elapsed intervals, which have
 * occured since the last call to update ().
 *
 * Used to determine how many frames have elapsed, or for simpler
 * objects to keep track of time in relativistic frames.
 */
template <typename T> T Timer<T>::getFrames () const
{
   return _frames;
}


/*
 * Determines how much time in units T we must wait to reach
 * the next timer interval.  Note that this method will
 * call the timer functor to calculate a wait time.
 *
 * Returns the wait time, or 0 if the timer has already
 * elapsed but we haven't called update () yet.
 */
template <typename T> T Timer<T>::waitTime () const
{
   T tnow = (*this->_timeFunc) ();
   
   if (tnow > _t0 + _terr + _interval) {
      return 0;
   } else {
      return (_t0 + _terr + _interval) - tnow;
   }
}


/*
 * Resets and reinitializes the timer.  The state of the
 * timer after the reset is the same as if it had
 * just been started.  The timer interval is preserved.
 */
template <typename T> void Timer<T>::reset ()
{
   _t0 = (*this->_timeFunc) ();
   _t1 = _t0;
   _tstart = _t0;
   _terr = 0;
   _frames = 0;
}

#endif
