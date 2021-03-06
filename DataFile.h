/**
 * @file DataFile.h
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#ifndef __READ_DATA_H__
#define __READ_DATA_H__

#include <stdio.h>
#include <inttypes.h>

#include <string>

#include "Pipe.h"

namespace MobileRGBD {

/**
 * @class DataFile DataFile.cpp DataFile.h
 * @brief Read from a standard file of try to find a compressed version (7zip) of the file (in read mode).
 *        * Until now, the pipe functionality is only written for the read side.
 *
 * This class is used to pipe data from a 7zip file containing only one file
 * when the original file is not found.
 * 
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 */
class DataFile : public Pipe
{
public:
	/** @brief constructor. Set LogLevel and ShowInfos and call Init().
	 */
	DataFile();

	/** @brief Virtual destructor, always.
	 */
	virtual ~DataFile();

	/** @brief Open a file *always in binary mode* (why convertir \r\n as \n is enough, even on Windows (not in
	 *         some strange app anyway). If reading is asked and the file could not be opened,
	 *         try to open a 7zip version of the file using 7z.
	 *
	 * @param Filename [in] The file name.
	 * @param eMode [in] The width of the video stream.
	 * @return true if the file or its 7z version is opened.
	 */
	bool Open( const char * Filename, int eMode = READ_MODE );

	/** @brief Read bytes from a the file (or pipe). Identical to fread.
	 *
	 * @param ptr [in,out] Pointer to buffer.
	 * @param size [in] Size of element to read.
	 * @param nmemb [in] Number ot element to read.
	 * @return Number of elements read.
	 */
	size_t Read( void *ptr, size_t size, size_t nmemb );

	/** @brief Write bytes to a the file (or pipe). Identical to fwrite.
	 *
	 * @param ptr [in] Pointer to buffer.
	 * @param size [in] Size of element to read.
	 * @param nmemb [in] Number ot element to write.
	 * @return Number of elements written.
	 */
	size_t Write(const void *ptr, size_t size, size_t nmemb );

	/** @brief Close file (or pipe). Identical to fclose/pclose.
	 *
	 */
	int Close();

	/** @brief Write bytes to a the file (or pipe). Identical to fseek.
	 *
	 * @param offset [in] Number of offset bytes.
	 * @param whence [in] Origine of the offset (see fseek).
	 * @return error code, same as fseek.
	 */
	int Seek(int64_t offset, int whence);

	/** @brief Get position in the current file/pipe.
	 *
	 */
	int64_t Tell();

	/** @brief Restart file at beginning.
	 *
	 */
	void Rewind();

	/** @brief Retrive current position in a file/pipe as a fpos_t_ structure (identical to fgetpos).
	 *
	 * @param pos [in,out] pointer to a fpos_t structure to fill.
	 * @return error code, same as fgetpos.
	 */
	int GetPos(fpos_t *pos);

	/** @brief Retrieve current position in a file/pipe as a fpos_t_ structure (identical to fsetpos).
	 *
	 * @param pos [in] pointer to a fpos_t structure to use to set current pos in file/pipe.
	 * @return error code, same as fsetpos.
	 */
	int SetPos(fpos_t *pos); 

	/** @brief Return true if the file/pipe is opened.
	 * @return True is file/pipe is opened.
	 */
	bool IsOpen() { return (InternalFile != nullptr); }

	/** @brief Check if a file exists
	 *
	 * @param FileName [in] File name.
	 * @return true if file actually exists
	 */
	static bool FileOrFolderExists(  const char * FileName );

	static bool OpenCompressedVersionFirst;	/*!< Say that we want to try to open compressed version first. Useful when data are over the network. Default, false. */

protected:
	bool IsPipe;						/*!< Say that the InternalFile is a pipe or a usual file. Default, false. */
	int64_t Pos;						/*!< Say that the InternalFile is a pipe or a usual file. Default, false. */
	std::string CompressedFileName;
	static const size_t DropBufferSize = 1024*1024;
	static char DropBuffer[DropBufferSize];			/*!< Share 1 Mib buffer to drop data when seeking forward in pipes */

	/** @brief Open a file *always in binary mode*.
	 *
	 * @param Filename [in] The file name.
	 * @param eMode [in] The width of the video stream.
	 * @return true if the file version is opened.
	 */
	bool InternalOpen( const char * Filename, int eMode = READ_MODE );

	/** @brief Open a 7z version file *always in binary mode*.
	 *
	 * @param Filename [in] The file name (without .7z extension)
	 * @param eMode [in] The width of the video stream.
	 * @return true if the 7zip is opened.
	 */
	bool InternalOpenCompressedVersion( const char * Filename, int eMode = READ_MODE );

	/** @brief Open a 7z file *always in binary mode*.
	 *
	 * @param Filename [in] The 7zip file name.
	 * @param eMode [in] The width of the video stream.
	 * @return true if the 7zip is opened.
	 */
	bool InternalOpenCompressed( const char * Filename, int eMode = READ_MODE );
};


} // namespace MobileRGBD

#endif // __READ_DATA_H__
