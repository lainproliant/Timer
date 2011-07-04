#include <cstdio>
#include <iostream>
#include <time.h>
#include "KernelTimer.hpp"

int main (int argc, char* argv[])
{
   KernelTimer timer;

   timer.start (KernelTimer::SECOND / 10);
   
   while (timer.getFrames () < 30) {
      if (timer.update ()) {
         printf ("Frames: %llu, Usec: %llu\n", timer.getFrames (), timer.getTicks ());
      }

      //printf ("Left: %llu\n", timer.waitTime ());
      
      timer.sleepyTime ();
      
   }
   
   printf ("%s\n", "Goodbye!");
   
   return 0;
}
