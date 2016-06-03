////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////

#include "BaseSystem.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	BaseSystem* System;
	bool result;
	
	
	// Create the system object.
	System = new BaseSystem();
	if(!System)
	{
		return 0;
	}
#ifndef C_R
	// Initialize and run the system object.
	result = System->InitSys();
	if(result)
	{
		System->Run();
	}
#else
	result = System->InitSysBeforeLoading();
	if(result)
	{
		System->RunLoading();
	}
#endif

	// Shutdown and release the system object.
	System->Shutdown();
	delete System;
	System = 0;

	return 0;
}