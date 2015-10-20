/**
 * @file ReadTimestampFile.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __READ_TIMESTAMP_FILE__
#define __READ_TIMESTAMP_FILE__

#include <stdio.h>
#include <string>

#include "DataFile.h"
#include "ReadTimestamp.h"

namespace MobileRGBD {

/**
 * @class ReadTimestampFile ReadTimestampFile.cpp ReadTimestampFile.h
 * @brief Read file with timestamp and data on each files. For each line,
 *        the timestamp is followed by an space and then data. Each line
 *        without timestamp is ignored. An exemple of localisation file
 *		  for a robot:
 *		  @code
		  1432037186.065 x=2.316 y=5.295 o=-1.052 647640
		  1432037186.112 x=2.314 y=5.296 o=-1.051 647720
		  1432037186.212 x=2.311 y=5.300 o=-1.053 647800
		  1432037186.271 x=2.311 y=5.297 o=-1.053 647880
		  1432037186.367 x=2.317 y=5.293 o=-1.054 647960
		  1432037186.467 x=2.318 y=5.294 o=-1.057 648040
		  1432037186.512 x=2.319 y=5.291 o=-1.057 648120
		  1432037186.613 x=2.323 y=5.288 o=-1.055 648200
		  1432037186.667 x=2.324 y=5.280 o=-1.053 648280
		  1432037186.769 x=2.327 y=5.277 o=-1.053 648360
		  @endcode
 *
 * 
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 */
class ReadTimestampFile : public ReadTimestamp
{
public:
	/** @brief Constructor. Create a ReadTimestampFile object using specific file.
	 *
	 * @param FileName [in] Name of the file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
	 * @param SizeOfLineBuffer [in] Size of buffer to read each line of the file (default=ReadTimestamp::DefaultLineBufferSize).
	 */
	ReadTimestampFile( const std::string& FileName, size_t SizeOfLineBuffer = ReadTimestamp::DefaultLineBufferSize );

	/** @brief virtual destructor (always).
	 */
	virtual ~ReadTimestampFile() {}

	/** @brief Search for a specific timestamp and set the DataBuffer pointer on the beginning of the data.
	 *		   Could be overloaded as a virtual function.
	 *
	 * @param RequestedTimestamp [in] Requested timestamp.
	 * @param ValidityTimeInMs [in] Threshold for searching for a timestamp (default=ReadTimestamp::DefaultValidityTimeInMs).
	 * @return True if the timestamp has been found.
	 */
	virtual bool GetDataForTimestamp(const TimeB &RequestedTimestamp, unsigned short int ValidityTimeInMs = (unsigned short int)ReadTimestamp::DefaultValidityTimeInMs );

	/** @brief Search for the timestamp and set the DataBuffer pointer on the beginning of the data.
	 *		   Could be overloaded as a virtual function.
	 *
	 * @param ValidityTimeInMs [in] Threshold for searching for a timestamp (default=ReadTimestamp::DefaultValidityTimeInMs).
	 * @return True if the timestamp has been found.
	 */
	virtual bool GetCurrentData(unsigned short int ValidityTimeInMs = (unsigned short int)ReadTimestamp::DefaultValidityTimeInMs );

	/** @brief Search for the timestamp and if found and process it by calling ProcessElement. 
	 *
	 * @param RequestedTimestamp [in] Requested timestamp.
	 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
	 * @return True if everything went fine.
	 */
	virtual bool Process( const TimeB &RequestTimestamp, void * UserData = nullptr );

	/** @brief Process the current element by calling ProcessElement on it (usefull when we parse at the same time we read timestamp).
	 *
	 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
	 *  @return True if everything went fine.
	 */
	virtual bool ProcessCurrent( void * UserData = nullptr );

	/** @brief Virtual function to overload. This function will be called by processing methods.
	 *         It needs to be overload in subclasses in order to do the right processing. Default
	 *		   implementation just returns True.
	 *
	 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
	 * @return True if everything went fine.
	 */
	virtual bool ProcessElement( const TimeB &RequestTimestamp, void * UserData = nullptr  )
	{
		return true;
	}

	char * DataBuffer;		/*!< @brief Pointer on the first character of the data on the current line. */
};

} // using namespace MobileRGBD;

#endif // __READ_TIMESTAMP_FILE__
