/**
 * @file ReadTimestampFile.cpp
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#include "ReadTimestampFile.h"

using namespace std;

/** @brief Constructor. Create a ReadTimestampFile object using specific file.
 *
 * @param FileName [in] Name of the file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
 * @param SizeOfLineBuffer [in] Size of buffer to read each line of the file (default=ReadTimestamp::DefaultLineBufferSize).
 */
ReadTimestampFile::ReadTimestampFile( const string& FileName, size_t SizeOfLineBuffer /* = ReadTimestamp::DefaultLineBufferSize */ ) : ReadTimestamp( FileName, SizeOfLineBuffer )
{
	DataBuffer = nullptr;
}

/** @brief Search for a specific timestamp and set the DataBuffer pointer on the beginning of the data.
 *		   Could be overloaded as a virtual function.
 *
 * @param RequestedTimestamp [in] Requested timestamp.
 * @param ValidityTimeInMs [in] Threshold for searching for a timestamp (default=ReadTimestamp::DefaultValidityTimeInMs).
 * @return True if the timestamp has been found.
 */
bool ReadTimestampFile::GetDataForTimestamp(const TimeB &RequestedTimestamp, unsigned short int ValidityTimeInMs /* = (unsigned short int)ReadTimestamp::DefaultValidityTimeInMs */ )
{
	if ( SearchDataForTimestamp(RequestedTimestamp, ValidityTimeInMs) == true )
	{
		DataBuffer = &LineBuffer[EndOfTimestampPosition];
		return true;
	}

	return false;
}


/** @brief Search for the timestamp and set the DataBuffer pointer on the beginning of the data.
 *		   Could be overloaded as a virtual function.
 *
 * @param ValidityTimeInMs [in] Threshold for searching for a timestamp (default=ReadTimestamp::DefaultValidityTimeInMs).
 * @return True if the timestamp has been found.
 */
bool ReadTimestampFile::GetCurrentData(unsigned short int ValidityTimeInMs /* = (unsigned short int)ReadTimestamp::DefaultValidityTimeInMs */ )
{
	// Do we have an internal timestamp ?
	if ( CurrentTimestampIsInitialized == false )
	{
		// no timestamp, no data
		return false;
	}

	// Call function with internal timestamp
	return GetDataForTimestamp( CurrentTimestamp, ValidityTimeInMs );
}

/** @brief Search for the timestamp and if found, and process it by calling ProcessElement. 
 *
 * @param RequestedTimestamp [in] Requested timestamp.
 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
 * @return True if everything went fine.
 */
bool ReadTimestampFile::Process( const TimeB &RequestTimestamp, void * UserData /* = nullptr */ )
{
	if ( GetDataForTimestamp( RequestTimestamp ) == true )
	{
		return ProcessElement( RequestTimestamp, UserData );
	}
	return false;
}

/** @brief Process the current element by calling ProcessElement on it (usefull when we parse at the same time we read timestamp).
 *
 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
 *  @return True if everything went fine.
 */
bool ReadTimestampFile::ProcessCurrent( void * UserData /* = nullptr */ )
{
	return Process( CurrentTimestamp, UserData );
}

