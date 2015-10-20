/**
 * @file ConstantFpsTimestampFromFile.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __CONSTANT_FPS_TIMESTAMP_FROM_FILE_H__
#define __CONSTANT_FPS_TIMESTAMP_FROM_FILE_H__

#include "ReadTimestamp.h"

namespace MobileRGBD {

class ConstantFpsTimestampFromFile : public ReadTimestamp
{
public :
	ConstantFpsTimestampFromFile( const std::string& FileName, float FrameRate = 30.0f )
			: ReadTimestamp( FileName )
	{
		FirstTimeStamp = true;
		if ( FrameRate <= 0.0f || ReadTimestamp::GetNextTimestamp() == false )
		{
			InitTime.time = InitTime.millitm = 0;
			TimeStep = 1;
			EndTime.time = EndTime.millitm = 0;
		}
		else
		{
			InitTime = CurrentTimestamp;
			TimeStep = (int)((1.0f/FrameRate)*1000.0f);
			// go to get last frame time
			while( ReadTimestamp::GetNextTimestamp() == true )
			{
				EndTime = CurrentTimestamp;
			}
		}
	}

	bool GetNextTimestamp()
	{
		if ( FirstTimeStamp == true )
		{
			FirstTimeStamp = false;
			CurrentTimestamp = InitTime;
			return true;
		}

		TimeB TimeTmp = CurrentTimestamp;

		TimeTmp += (int)TimeStep;
		if ( CompareTime(TimeTmp, EndTime) > 0 )
		{
			// ok, out of scope
			return false;
		}

		CurrentTimestamp = TimeTmp;
		return true;
	}

protected:
	TimeB InitTime;
	int TimeStep;
	TimeB EndTime;

	bool FirstTimeStamp;
};

} // namespace MobileRGBD

#endif // __CONSTANT_FPS_TIMESTAMP_FROM_FILE_H__