#ifndef __TIMERSDL_HPP
#define __TIMERSDL_HPP

#include <SDL.h>
#include "Timer.hpp"

/********************************************************************
 * TimerSDL:  A timer which uses SDL's SDL_GetTicks.
 */
class TimerSDL : public Timer <Uint32>
{
public:
   static const Uint32 SECOND = 1000;

   TimerSDL () : Timer <Uint32> (SDL_GetTicks)
   { }

   /*
    * Uses SDL_Delay () to attempt to sleep for the exact amount
    * of time remaining until the timer elapses.
    *
    * This method is useful for timed processes which need to use as
    * little CPU resources as possible by sacrificing a bit of accuracy.
    *
    * Uses waitTime (), which must call SDL_GetTicks () to determine the
    * amount of time remaining.
    *
    * Note that this method will almost always sleep too long.  The error 
    * will be noted and deducted from the next timer interval.
    */
   void sleepyTime () const {
      SDL_Delay (waitTime ());
   }
};

#endif
