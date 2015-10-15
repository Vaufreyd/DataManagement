/**
 * @file ReadTimestamp.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __READ_TIMESTAMP__
#define __READ_TIMESTAMP__

#include <stdio.h>
#include <string>
#include <System/TemporaryMemoryBuffer.h>

#include "DataFile.h"
#include "TimestampTools.h"

/**
 * @class ReadTimestamp ReadTimestamp.cpp ReadTimestamp.h
 * @brief Read timestamp files, i.e. files with timestamp at beginning of each line.
 *		  Timestamp are time in seconds since origine (usually epoch time) dot milliseconds.
 *		  The rest of the line, if any, is ignored. Timestamp *must* be ordered.
 *        This is an example of such file
 *		  @code
 *		  1433341728.727	// Wed, 03 Jun 2015 14:28:48.727 GMT
 *		  1433341728.743
 *		  1433341728.805
 *		  1433341728.868
 *		  1433341728.899
 *		  @endcode
 *
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 */
class ReadTimestamp
{
public:
	const size_t DefaultLineBufferSize =  10*1024*1024;		/*!< @brief Default buffer size for line reading (default 10MiB) */
	
	/** @brief Constructor. Create a ReadTimeStamp object using specific file.
	 *
	 * @param FileName [in] Name of the file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
	 * @param SizeOfLineBuffer [in] Size of buffer to read each line of the file (default=DefaultLineBufferSize).
	 */
	ReadTimestamp( const std::string& FileName, size_t SizeOfLineBuffer = DefaultLineBufferSize );

	/** @brief virtual destructor (always).
	 */
	virtual ~ReadTimestamp();

	/** @brief Restart file at beginning (if file is closed, file is re-opened).
	 */
	virtual void Reinit();

	/** @brief Close file.
	 */
	void Close();

	// Search for a suitable timestamp
	const unsigned short int DefaultValidityTimeInMs = 33;		/*!< @brief When searching for a specified timestamp, DefaultValidityTimeInMs specifies a threshold to for validity. */

	/** @brief Search for a specific timestamp in the file.
	 *
	 * @param RequestedTimestamp [in] Timestamp to search for.
	 * @param ValidityTimeInMs [in] 
	 * @return Searching for a line starting with the RequestedTimestamp. Return true if this line exists. If the previous timestamp is less than ValidityTimeInMs ms before,
			   it is considered as valid (for synchronous read of several files). In all other cases, return false;
	 */
	virtual bool SearchDataForTimestamp( const TimeB &RequestedTimestamp, unsigned short int ValidityTimeInMs = (unsigned short int)DefaultValidityTimeInMs );

	/** @brief Get the next timestamp of the file if any.
	 *
	 * @return True is the next timestamp has been retrieve.
	 */
	virtual bool GetNextTimestamp();

	/** @brief Get the next timestamp of the file if any.
	 *
	 * @return True is the previous timestamp has been retrieve.
	 */	
	bool GetPreviousTimestamp();

	/** @brief Rewind if possible to the previous timestamp.
	 *
	 * @return True is the rewind was possible and done.
	 */	
	bool Rewind();

	char* LineBuffer;							/*!< @brief Buffer to read the line. */
	int LineBufferSize;							/*!< @brief Actual size of the line buffer. */
	int EndOfTimestampPosition;					/*!< @brief Actual size of the line buffer. */
	TimeB CurrentTimestamp;						/*!< @brief Current value for the timestamp extracted from the file. */
	bool  CurrentTimestampIsInitialized;		/*!< @brief CurrentTimestamp is valid. */

protected:
	/** @brief Retrieve position of the current timestamp in the file (for the Rewind method).
	 *
	 */
	void AddTimestampPos()
	{
		if ( fin != (FILE*)NULL )
		{
			PreviousTimestampPosInFile[0] = PreviousTimestampPosInFile[1];
			PreviousTimestampPosInFile[1] = ftell( fin );
		}
	}

	DataFile fin;								/*!< @brief DataFile object to read usual or compressed files. */
	std::string FiletoOpen;						/*!< @brief Store the file name. */

	long int PreviousTimestampPosInFile[2];		/*!< @brief Store previous position in file in order to permit rewind. */
	TimeB PreviousTimestamp;					/*!< @brief Value of the preivous timestamp. */
};

#endif // __READ_TIMESTAMP__
