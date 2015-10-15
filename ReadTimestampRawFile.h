/**
 * @file ReadTimestampRawFile.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __READ_TIMESTAMP_RAW_FILE__
#define __READ_TIMESTAMP_RAW_FILE__

#include "ReadTimestampFile.h"
// Use Omiscid::TemporaryMemoryBuffer
#include <System/TemporaryMemoryBuffer.h>

#if defined WIN32 || defined WIN64
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#include <Windows.h>
#endif

/**
 * @class ReadTimestampRawFile ReadTimestampRawFile.cpp ReadTimestampRawFile.h
 * @brief Abstract class to read a timestamp file associated to a raw file. The timstamp data
 *		  lines contains as usual a timestamp followed by a frame number if the
 *		  associated raw file. This class could retrieve each frame associated
 *		  to a timestamp according to a provided SizeOfFrame and this frame number.
 *		  All the frames must have the same size. There is also a Subframes mode
 *		  when data at a requested timestamp can contains severals subojects (of the same
 *		  size).
 *		  As for standard timestamped files, textual data could be added at the end
 *		  of each lines.
 *		  Here an example for a Kinect2 video stream:
 *		  @code
		  1432037186.049 1, 20323761405951
		  1432037186.083 2, 20323761746706
		  1432037186.115 3, 20323762075887
		  1432037186.146 4, 20323762405880
		  1432037186.181 5, 20323762746654
		  1432037186.215 6, 20323763075941
		  1432037186.246 7, 20323763405912
		  1432037186.281 8, 20323763746660
		  1432037186.315 9, 20323764075878
		  @endcode
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 */
class ReadTimestampRawFile : public ReadTimestampFile
{
public:
	/** @brief Constructor. Create a ReadTimestampRawFile object using specific files (timestamp + raw).
	 *
	 * @param WorkingFile [in] Name of the timestamp file to open (even with '/' separator under Windows as Windows handles it also as a folder/file separator).
	 * @param RawFile [in] Name of the associated raw file.
	 * @param SizeOfFrame [in] Size of each frame in raw file.
	 */
	ReadTimestampRawFile( const std::string &WorkingFile, const std::string& RawFile, int SizeOfFrame );

	/** @brief virtual destructor (always).
	 */
	~ReadTimestampRawFile() {}
	
	/** @brief Restart file at beginning (if file is closed, file is re-opened).
	 *		   Overload of TimestampFile::Reinit.
	 */
	virtual void Reinit();

	/** @brief Search for the timestamp and if found and process it by calling ProcessElement. 
	 *		   Overload of ReadTimestampFile::Process.
	 *
	 * @param RequestedTimestamp [in] Requested timestamp.
	 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
	 * @return True if everything went fine.
	 */
	virtual bool Process( const TimeB &RequestTimestamp, void * UserData = nullptr );

	/** @brief Abstract function to overload. This function will be called by processing methods.
	 *         It needs to be overload in subclasses in order to do the right processing. Default
	 *		   implementation just returns True.
	 *		   Overload of ReadTimestampFile::ProcessElement.
	 *
	 * @param UserData [in] Pointer on a UserData pass to the ProcessElement function (default=nullptr).
	 * @return True if everything went fine.
	 */
	virtual bool ProcessElement( const TimeB &RequestTimestamp, void * UserData = nullptr ) = 0;

	/** @brief Get the frame number for the current data.
	 *
	 * @return -1 if an error occured or the zero based index for the frame.
	 */
	virtual int GetFrameNumber();

	/** @brief Load frame for a specific timestamp
	 *
	 * @param RequestedTimestamp [in] Requested timestamp.
	 * @return True if the full frame was loaded.
	 */
	virtual bool LoadFrame( const TimeB &RequestTimestamp );

	/** @brief Get frame for a specific index
	 *
	 * @param WantedIndex [in] Frame number in the raw file.
	 * @return True if the full frame was loaded.
	 */
	bool GetFrame( int WantedIndex );

	/** @enum ReadTimestampRawFile::ReadingMode
	 *  @brief Define single frame mode (for RGB, Depth, ...) et SubFramesMode, i.e. mode where several frames
	 *		   like bodies or faces are associated with a unique timestamp
	 */
	enum ReadingMode
	{
		SimpleFrameMode = 0,						/*!< @brief Single frame mode, default value. */
		SubFramesMode = 1							/*!< @brief Subframes mode (for bodies, face, etc.). */
	};

public:
	Omiscid::TemporaryMemoryBuffer FrameBuffer;		/*!< @brief A growing and autodeleting buffer (from Omiscid. See http://omiscid.gforge.inria.fr/) */
	int FrameSize;									/*!< @brief Size of each frame (or subframe) */
	int StartingFrame;								/*!< @brief Number of the first frame of the file (permits to recontsruct zero based index) */
	int CurrentIndex;								/*!< @brief Store current frame number in file (to prevent for uneeded fseek in file) */

	unsigned char Mode;								/*!< @brief Store current mode : single or subframes mode */
	int NumberOfSubFrames;							/*!< @brief When processing in SubFramesMode, store the number of subframes for the current timestamp */

protected:
	DataFile fRaw;									/*!< @brief DataFile object to read usual or compressed raw files. */
	std::string RawFileName;						/*!< @brief Store name of the raw file */
};

#endif // __READ_TIMESTAMP_RAW_FILE__
