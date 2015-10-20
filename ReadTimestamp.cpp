/**
 * @file ReadTimestamp.cpp
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#include "ReadTimestamp.h"

using namespace std;
using namespace MobileRGBD;

const size_t ReadTimestamp::DefaultLineBufferSize =  10*1024*1024;		/*!< @brief Default buffer size for line reading (default 10MiB) */
const unsigned short int ReadTimestamp::DefaultValidityTimeInMs = 33;	/*!< @brief When searching for a specified timestamp, DefaultValidityTimeInMs specifies a threshold to for validity (33ms). */

/** @brief Constructor. Create a ReadTimeStamp object using specific file.
 *
 * @param FileName [in] Name of the file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
 * @param SizeOfLineBuffer [in] Size of buffer to read each line of the file (default=DefaultLineBufferSize).
 */
ReadTimestamp::ReadTimestamp( const string& FileName, size_t SizeOfLineBuffer /* = 10 MB */ )
{
	// init internal variables
	FiletoOpen = FileName;
	EndOfTimestampPosition = 0;
	CurrentTimestamp.time = 0;
	CurrentTimestamp.millitm = 0;
	CurrentTimestamp.timezone = 0;
	CurrentTimestampIsInitialized = false;

	PreviousTimestampPosInFile[0] = -1;
	PreviousTimestampPosInFile[1] = -1;

	// Allocate line buffer
	LineBufferSize = 0;
	LineBuffer = new char[SizeOfLineBuffer];

	// Ok size is good
	LineBufferSize = (int)SizeOfLineBuffer;
}

/** @brief virtual destructor (always).
 */
ReadTimestamp::~ReadTimestamp()
{
	Close();

	if ( LineBuffer != nullptr )
	{
		delete LineBuffer;
	}
}

/** @brief Restart file at beginning (if file is closed, file is re-opened).
 */
void ReadTimestamp::Reinit()
{
	if ( fin == (FILE*)NULL )
	{
#ifdef DEBUG
		// fprintf( stderr, "Try to open '%s'\n", FiletoOpen.c_str() );
#endif
		fin.Open( FiletoOpen.c_str(), DataFile::READ_MODE );
	}
	else
	{
		fseek( fin, 0, SEEK_SET );
	}
	PreviousTimestampPosInFile[0] = -1;
	PreviousTimestampPosInFile[1] = -1;

	CurrentTimestamp.time = 0;
	CurrentTimestamp.millitm = 0;
	CurrentTimestamp.timezone = 0;
	CurrentTimestampIsInitialized = false;
}

/** @brief Close file.
 */
void ReadTimestamp::Close()
{
	if ( fin != (FILE*)NULL )
	{
		fin.Close();
	}
}

/** @brief Search for a specific timestamp in the file.
 *
 * @param RequestedTimestamp [in] Timestamp to search for.
 * @param ValidityTimeInMs [in] 
 * @return Searching for a line starting with the RequestedTimestamp. Return true if this line exists. If the previous timestamp is less than ValidityTimeInMs ms before,
		it is considered as valid (for synchronous read of several files). In all other cases, return false;
*/
bool ReadTimestamp::SearchDataForTimestamp(const TimeB &RequestedTimestamp, unsigned short int ValidityTimeInMs /* DefaultValidityTimeInMs */ )
{
	int ValidityTime = -(int)ValidityTimeInMs;

	if ( fin == (FILE*)NULL )
	{
		// Open and get next timestamp
		Reinit();
	}

	if ( fin == (FILE*)NULL )
	{
		return false;
	}

	if ( feof(fin) )
	{
		return CurrentTimestampIsInitialized;
	}

	bool TotalFileParsed = false;

	if ( CurrentTimestampIsInitialized == false )
	{
		GetNextTimestamp();
	}

	while( CurrentTimestampIsInitialized == true )
	{
		int Comp = CompareTime( RequestedTimestamp, CurrentTimestamp );

		// Requested time stamp is in future
		if ( Comp > 0 )
		{
			if ( fin != nullptr && feof(fin) )
			{
				return (CurrentTimestampIsInitialized && Comp <= 100);	// let's say that if last data is older taht 100ms, we did not take care of it anymore
			}

			// Cancel current result
			LineBuffer[0] = '\0';
			EndOfTimestampPosition = 0;
			/*CurrentTimestamp.time = 0;
			CurrentTimestamp.millitm = 0;
			CurrentTimestamp.timezone = 0;*/
			CurrentTimestampIsInitialized = false;

			// Go ahead
			GetNextTimestamp();
			continue;
		}

		// Here we have no data, our data is in the futur
		if ( Comp < 0 )
		{
			if ( fin != nullptr && feof(fin) )
			{
				// Our current data is in the futur, we can not search for new data as we are at end of the file
				return false;
			}

			if ( PreviousTimestamp.time != 0 )
			{
				// We have a previous timestamp, check if it is near enough
				if ( CompareTime( RequestedTimestamp, PreviousTimestamp ) > ValidityTime )
				{
					Rewind();
					GetNextTimestamp();
					return true;
				}
			}

			// First timestamp is after Requested time
			return false;
		}

		// Here we have the right timestamp, Comp == 0
		return true;
	}

	return false;
}

/** @brief Get the next timestamp of the file if any.
 *
 * @return True is the next timestamp has been retrieve.
 */
bool ReadTimestamp::GetPreviousTimestamp()
{
	return (Rewind() && GetNextTimestamp());
}

/** @brief Get the next timestamp of the file if any.
 *
 * @return True is the previous timestamp has been retrieve.
 */	
bool ReadTimestamp::GetNextTimestamp()
{
	if ( fin == (FILE*)NULL )
	{
		Reinit();
	}

	if ( fin == (FILE*)NULL )
	{
		return false;
	}

	if ( feof(fin) )
	{
		// return last value or none
		return CurrentTimestampIsInitialized;
	}
	
	while( !feof(fin) )
	{
		int iTmp;
		int length = 0;
		TimeB lTimestamp;

		// Remember where I am
		AddTimestampPos();

		// try to read a line
		if ( fgets( LineBuffer, LineBufferSize-1, fin ) == (char*)NULL )
		{
			break;
		}

		// Ok, we have a line, remeber CurrentTimestamp
		PreviousTimestamp.time = CurrentTimestamp.time;
		PreviousTimestamp.millitm = CurrentTimestamp.millitm;
		PreviousTimestamp.timezone = CurrentTimestamp.timezone;

		// try to parse line
		if ( sscanf( LineBuffer, "%d.%d%*[ \t]%n", &iTmp, &lTimestamp.millitm, &length ) != 2 )
		{
			continue;
		}
		lTimestamp.time = iTmp;

		// Copy data to internal 
		CurrentTimestamp = lTimestamp;
		EndOfTimestampPosition = length;

		CurrentTimestampIsInitialized = true;

		return true;
	}
	
	// Ok, close file and say we wo not find it
	// fclose( fin );
	// fin = (FILE*)NULL;

	return false;	// say end of file
}

/** @brief Rewind if possible to the previous timestamp.
 *
 * @return True is the rewind was possible and done.
 */	
bool ReadTimestamp::Rewind()
{
	if ( fin != (FILE*)NULL )
	{
		if ( PreviousTimestampPosInFile[0] != -1 )
		{
			fseek( fin, PreviousTimestampPosInFile[0], SEEK_SET );
			PreviousTimestampPosInFile[0] = -1;
			PreviousTimestampPosInFile[1] = -1;

			PreviousTimestamp.time = 0;
			PreviousTimestamp.millitm = 0;
			PreviousTimestamp.timezone = 0;

			return true;
		}
	}
	return false;
}

