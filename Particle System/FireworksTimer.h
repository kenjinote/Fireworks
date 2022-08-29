/*
Gets started by the main application in a separate thread and coordinates rocket launches.
*/

#include "Rocket.h"
#include <thread>

// this is used as a functor
class FireworksTimer
{
public:
	FireworksTimer(Rocket* rockets, int numberOfRockets) : oldTime_(0), rockets_(rockets), numberOfRockets_(numberOfRockets){}
	void operator()(float* launchTimes, bool* doRun)
	{
		// the main thread will signal termination by setting doRun to false
		while(*doRun)
		{
			int i = 0;

			// fire rockets
			while(*doRun && i < numberOfRockets_)
			{
				Sleep(static_cast<DWORD>(launchTimes[i] - oldTime_));	// simply sleep until it is time to fire the next rocket
				rockets_[i].fire();
				oldTime_ = launchTimes[i];
				++i;
			}

			if(*doRun)
			{
				// wait for some time and prepare for another run of the firework

				Sleep(4000);								
				for(int i = 0; i < numberOfRockets_; ++i)
				{
					rockets_[i].reset();	// reset the rockets to be fired again
				}
				oldTime_ = 0;
			}
		}
	}
private: 
	float oldTime_;			// used to calculate the correct waiting time between rockets
	Rocket* rockets_;		// pointer to the rockets to be launched
	int numberOfRockets_;	// the amount of rockets stored in the array pointed to by the above pointer
};