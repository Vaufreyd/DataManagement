/**
 * @file ReadTimestampRawFile.cpp
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#include "ReadTimestampRawFile.h"

#include <string.h>

using namespace MobileRGBD;

/** @brief Constructor. Create a ReadTimestampRawFile object using specific files (timestamp + raw).
 *
 * @param WorkingFile [in] Name of the timestamp file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
 * @param RawFile [in] Name of the associated raw file.
 * @param SizeOfFrame [in] Size of each frame in raw file.
 */
ReadTimestampRawFile::ReadTimestampRawFile( const std::string &WorkingFile, const std::string& RawFile, int SizeOfFrame )
	: ReadTimestampFile( WorkingFile )
{
	Mode = SimpleFrameMode;
	NumberOfSubFrames = 0;

	StartingFrame = 0;
	CurrentIndex = 0;
	RawFileName = RawFile;
	FrameSize = SizeOfFrame;

	FrameBuffer.SetNewBufferSize(SizeOfFrame+1);
}

/** @brief Restart file at beginning (if file is closed, file is re-opened).
 *	 *  Overload of TimestampFile::Reinit.
 */
void ReadTimestampRawFile::Reinit()
{
	ReadTimestampFile::Reinit();
	if ( fin.IsOpen() == false )
	{
		return;
	}

	// Reinit number of subframes
	NumberOfSubFrames = 0;
	
	// try to read a line
	LineBuffer[0] = '\0';
	if ( fgets( LineBuffer, LineBufferSize-1, fin ) == (char*)NULL )
	{
		return;
	}

	// Get Starting Frame (in case it is not 0)
	if ( sscanf( LineBuffer, "%*d.%*d%*[ \t]%d", &StartingFrame ) != 1 )
	{
		return;
	}

	// Reset file to begining
	fin.Seek( 0L, SEEK_SET );
}

/** @brief Search for the timestamp and if found and process it by calling ProcessElement. 
 *	 *  Overload of ReadTimestampFile::Process.
 *
 * @param RequestedTimestamp [in] Requested timestamp.
 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
 * @return True if everything went fine.
 */
bool ReadTimestampRawFile::Process( const TimeB &RequestTimestamp, void * UserData /* = nullptr */ )
{
	if ( LoadFrame( RequestTimestamp ) == true )
	{
		return ProcessElement( RequestTimestamp, UserData );
	}
	return false;
}

/** @brief Get the frame number for the current data.
 *
 * @return -1 if an error occured or the zero based index for the frame.
 */
int ReadTimestampRawFile::GetFrameNumber()
{
	int FrameIndex = -1;

	// If sscanf failed, FrameIndex remains -1
	sscanf( DataBuffer, "%d", &FrameIndex );

	return FrameIndex;
}

/** @brief Load frame for a specific timestamp
 *
 * @param RequestedTimestamp [in] Requested timestamp.
 * @return True if the full frame was loaded.
 */
bool ReadTimestampRawFile::LoadFrame( const TimeB &RequestTimestamp )
{
	if ( GetDataForTimestamp( RequestTimestamp ) == false )
	{
		// Can not get data
		return false;
	}

	// Get corresponding frame index
	int FrameIndex = GetFrameNumber();
	if( FrameIndex >= 0 )
	{
		return GetFrame( FrameIndex );
	}

	return false;
}

/** @brief Get frame for a specific index
 *
 * @param WantedIndex [in] Frame number in the raw file.
 * @return True if the full frame was loaded.
 */
bool ReadTimestampRawFile::GetFrame( int WantedIndex )
{
	int Index = WantedIndex - StartingFrame;
	NumberOfSubFrames = 0;

	if ( fRaw.IsOpen() == false )
	{
		if ( fRaw.Open( RawFileName.c_str(), DataFile::READ_MODE ) == false )
		{
			return false;
		}
	}

	// If we are in multiple mode, we want to load all frame at once
	// Default values for single mode
	int LoadSize;
	if ( Mode == SubFramesMode )
	{
		// Here we need to get the number of sub-frames
		if ( sscanf( DataBuffer, "%*d, %d", &NumberOfSubFrames ) != 1 )
		{
			// Could not find subframe number
			fprintf( stderr, "Could not retrieve number of subFrame in SubFramesMode\n" );
			return false;
		}

		if ( NumberOfSubFrames == 0 )
		{
			// Here, nothing to load timestamp with empty data, loaded !
			return true;
		}
		
		// Compute new load size
		LoadSize = FrameSize * NumberOfSubFrames;

		// Increase if needed size of buffer
		FrameBuffer.SetNewBufferSize( LoadSize );
	}
	else
	{
		// In simple mode, there is alsways a frame
		NumberOfSubFrames = 1;
		LoadSize = FrameSize;
	}

	if ( Index == CurrentIndex )
	{
		// idilic case
		if ( fRaw.Read( FrameBuffer, LoadSize, 1) != 1 )
		{
			return false;
		}
		CurrentIndex += NumberOfSubFrames;
		return true;
	}

	// Try to go to new position and read a frame
	int64_t NewPos = (int64_t)Index*(int64_t)FrameSize;
	if ( fRaw.Seek( NewPos, SEEK_SET ) != 0 )
	{
		return false;
	}
	if ( fRaw.Read( FrameBuffer, LoadSize, 1 ) != 1 )
	{
		return false;
	}

	// Current Index is now the next one
	CurrentIndex = Index + NumberOfSubFrames;

	return true;
}
