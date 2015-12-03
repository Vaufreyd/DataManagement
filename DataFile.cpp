/**
 * @file DataFile.cpp
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#include "DataFile.h"

#include <sys/stat.h>
#include <stdlib.h>

#include <algorithm>

using namespace MobileRGBD;

// static
bool DataFile::OpenCompressedVersionFirst = false;		/*!< Say that we want to try to open compressed version first. Useful when data are over the network. Default, false. */
char DataFile::DropBuffer[DataFile::DropBufferSize];	/*!< Share 1 Mib buffer to drop data when seeking forward in pipes */

#if defined WIN32 || defined WIN64 
	// use 64 bits versions of ftell and fseek, make them POSIX compliant
	#define ftello _ftelli64
	#define fseeko _fseeki64
#else
	// Check that we will use 64 bits of ftell and fseek
	#if !defined _FILE_OFFSET_BITS || _FILE_OFFSET_BITS < 64
		#error You must compile using a _FILE_OFFSET_BITS defined to 64 (-D _FILE_OFFSET_BITS=64)
	#endif
#endif

/** @brief Check if a file exists
 *
 * @param FileName [in] File name.
 * @return true if file actually exists
 */
bool DataFile::FileOrFolderExists(  const char * FileName )
{
	struct stat FileStat;

	if ( FileName == nullptr )
	{
		return false;
	}

	if ( stat(FileName, &FileStat) == 0 )
	{
		// exists
		return true;
	}

	return false;
}

/** @brief constructor. Set LogLevel and ShowInfos and call Init().
 */
DataFile::DataFile()
{
	// By default, we have no pipe
	IsPipe = false;
	Pos = -1;
}

/** @brief Virtual destructor, always.
 */
DataFile::~DataFile()
{
	Close();
}

/** @brief Open a file *always in binary mode*.
	*
	* @param Filename [in] The file name.
	* @param eMode [in] The width of the video stream.
	* @return true if the file version is opened.
	*/
bool DataFile::InternalOpen( const char * Filename, int eMode /* = READ_MODE */ )
{
	const char * ModeRead = "rb";
	const char * ModeWrite = "wb";

	// ( InternalFile == nullptr ) has been check in ::Open. We do not do it again here

	// try to open file the usual way
	if ( eMode == READ_MODE )
	{
		InternalFile = fopen(Filename, ModeRead);
	}
	else if ( eMode == WRITE_MODE )
	{
		InternalFile = fopen(Filename, ModeWrite);
	}
	else
	{
		Pos = -1;
		return false;
	}

	if ( InternalFile != nullptr )
	{
		Pos = 0;
		return true;
	}

	return false;
}

/** @brief Open a 7z version file *always in binary mode*.
	*
	* @param Filename [in] The file name (without .7z extension)
	* @param eMode [in] The width of the video stream.
	* @return true if the 7zip is opened.
	*/
bool DataFile::InternalOpenCompressedVersion( const char * Filename, int eMode /* = READ_MODE */ )
{
	// Initial conditions have been tested in ::Open (public function)
	// InternalFile == nullptr
	// Filename != nullptr
	// Values have been set to:
	// IsPipe = false
	
	// Here, we can not manage to open file for writing, we do not try it compressed
	if ( eMode != READ_MODE )
	{
		// TODO: add support for output compression
		Pos = -1;
		return false;
	}

	// Generate new file name with '.7z' extension
	char NewFileName[1024];
	sprintf( NewFileName, "%s.7z", Filename );

	return InternalOpenCompressed( NewFileName, eMode );
}

/** @brief Open a 7z file *always in binary mode*.
	*
	* @param Filename [in] The 7zip file name.
	* @param eMode [in] The width of the video stream.
	* @return true if the 7zip is opened.
	*/
bool DataFile::InternalOpenCompressed( const char * Filename, int eMode /* = READ_MODE */ )
{
	// Initial conditions have been tested in ::Open (public function)
	// InternalFile == nullptr
	// Filename != nullptr
	// Values have been set to:
	// IsPipe = false

	Pos = -1;

	// Here, we can not manage to open file for writing, we do not try it compressed
	if ( eMode != READ_MODE )
	{
		// TODO: add support for output compression
		return false;
	}

	// Try to open a 7z compressed version of the file
	char _7zPipedCommand[1024];

	// If there is a 7zip version of the file,
	// generate a pipe command to open it
#if defined WIN32 || defined WIN64 
	if ( FileOrFolderExists( Filename ) == false )
	{
		return false;
	}
	sprintf( _7zPipedCommand, "7z e -so \"%s\" 2> %s", Filename, NULL_OUTPUT );
#else
	char resolved_path[1024]; 
    realpath( Filename, resolved_path); 

	if ( FileOrFolderExists( resolved_path ) == false )
	{
		return false;
	}

	sprintf( _7zPipedCommand, "7z e -so \"%s\"", resolved_path );
#endif

	// Store pipe command to reopen it if we want to rewind
	CompressedFileName = Filename;

	// Try to the open the pipe
	if ( Pipe::Open( _7zPipedCommand, Pipe::READ_MODE ) == true )
	{
		IsPipe = true;
		Pos = 0;
		return true;
	}

	// Could not open file
	return false;
}

/** @brief Open a file *always in binary mode* (why convertir \r\n as \n is enough, even on Windows (not in
	*         some strange app anyway). If reading is asked and the file could not be opened,
	*         try to open a 7zip version of the file using 7z.
	*
	* @param Filename [in] The file name.
	* @param eMode [in] The width of the video stream.
	* @return true if the file or its 7z version is opened.
	*/
bool DataFile::Open( const char *Filename, int eMode /* = READ_MODE */ )
{
	// prior checks
	if ( InternalFile != nullptr )
	{
		fprintf( stderr, "Could not reopen file, please call DataFile::Close() before.\n" );
		return false;
	}

	if ( Filename == nullptr )
	{
		fprintf( stderr, "File name is null.\n" );
		return false;
	}

	// By default it is not a pipe
	IsPipe = false;

	if ( OpenCompressedVersionFirst == true )
	{
		// Ok, try to open first the compressed version
		if ( InternalOpenCompressedVersion( Filename, eMode ) == true )
		{
			return true;
		}

		// ok, open it usualy
		return InternalOpen( Filename, eMode );
	}

	// Here, we try first usual file
	if ( InternalOpen( Filename, eMode ) == true )
	{
		return true;
	}

	// ok, try to open it in its compressed version
	return InternalOpenCompressedVersion( Filename, eMode );
}

/** @brief Read bytes from a the file (or pipe). Identical to fread.
	*
	* @param ptr [in,out] Pointer to buffer.
	* @param size [in] Size of element to read.
	* @param nmemb [in] Number ot element to read.
	* @return Number of element read.
	*/
size_t DataFile::Read( void *ptr, size_t size, size_t nmemb )
{
	size_t RetCode = (size_t)0;
	if ( InternalFile != nullptr )
	{
		RetCode = fread( ptr, size, nmemb, InternalFile );
		// Compile new pos value
		Pos += (int64_t)size*(int64_t)RetCode;
		// DWORD err = GetLastError();
	}

	return RetCode;
}

/** @brief Write bytes to a the file (or pipe). Identical to fwrite.
	*
	* @param ptr [in] Pointer to buffer.
	* @param size [in] Size of element to read.
	* @param nmemb [in] Number ot element to write.
	* @return Number of elements written.
	*/
size_t DataFile::Write(const void *ptr, size_t size, size_t nmemb )
{
	size_t RetCode = (size_t)0;
	if ( InternalFile != nullptr )
	{
		RetCode = fwrite( ptr, size, nmemb, InternalFile );
		// Compile new pos value
		Pos += (int64_t)size*(int64_t)RetCode;
	}

	return (size_t)0;
}

/** @brief Close file (or pipe). Identical to fclose/pclose.
	*
	*/
int DataFile::Close()
{
	int RetCode = 0;

	if ( InternalFile != nullptr )
	{
		if ( IsPipe == false )
		{
			// Usual file
			RetCode = fclose(InternalFile);
			InternalFile = nullptr;
		}
		else
		{
			Pipe::Close();
		}
		IsPipe = false;
	}

	return RetCode;
}

/** @brief Write bytes to a the file (or pipe). Identical to fseek.
	*
	* @param offset [in] Number of offset bytes.
	* @param whence [in] Origine of the offset (see fseek).
	* @return error code, same as fseek.
	*/
int DataFile::Seek(int64_t offset, int whence)
{
	if ( InternalFile == nullptr )
	{
		// Could not seek
		return -1;
	}

	// Specific case for Pipe
	if ( IsPipe == false )
	{
		// usual case, usual file, call the seek function
		return fseeko( InternalFile, offset, whence );
	}

	// Here, we are using a pipe
	switch(whence)
	{
		case SEEK_CUR:
			// Compute position from here
			offset += Pos;
			// Continue in the SEEK_SET condition

		case SEEK_SET:
			if ( offset < Pos )
			{
				// Could not seek backward in pipe
				return  EBADF;
			}

			// Loop to greedily eat data coming from the pipe as we can not seek
			while( Pos < offset )
			{
				// Read at max DropBufferSize
				int NbToRead = std::min( (int)DropBufferSize, (int)(offset-Pos) );

				// Can we read?
				int NbRead = (int)fread( DropBuffer, NbToRead, 1, InternalFile );
				if ( NbRead == 0 )
				{
					// Could not seek
					return EBADF;
				}

				// Yes, increase Pos
				Pos += (int64_t)NbRead*(int64_t)NbToRead;
			}

			return 0;

		case SEEK_END:	// Could not seek from end with pipe
			return EBADF;

		default:
			return EINVAL;
	}
}

/** @brief Get position in the current file/pipe.
	*
	*/
int64_t DataFile::Tell()
{
	if ( InternalFile == nullptr )
	{
		// Could not tell
		return -1;
	}

	// Return current pos file
	return Pos;
}

/** @brief Restart file at beginning.
	*
	*/
void DataFile::Rewind()
{
	if ( InternalFile == nullptr )
	{
		// Could not ftell
		return;
	}

	// In case of a pipe
	if ( IsPipe == true )
	{
		// Close the pipe
		Close();

		if ( CompressedFileName.length() == 0 )
		{
			// impossible to rewind
			return;
		}

		// Reopen the pipe
		InternalOpenCompressed( CompressedFileName.c_str() );

		return;
	}

	return rewind(InternalFile);
}

/** @brief Retrieve current position in a file/pipe as a fpos_t_ structure (identical to fgetpos).
	*
	* @param pos [in,out] pointer to a fpos_t structure to fill.
	* @return error code, same as fgetpos.
	*/
int DataFile::GetPos(fpos_t *pos)
{
	if ( InternalFile == nullptr )
	{
		// Could not fgetpos
		return -1;
	}
	return fgetpos(InternalFile, pos);
}

/** @brief Retrieve current position in a file/pipe as a fpos_t_ structure (identical to fsetpos).
	*
	* @param pos [in] pointer to a fpos_t structure to use to set current pos in file/pipe.
	* @return error code, same as fsetpos.
	*/
int DataFile::SetPos(fpos_t *pos)
{
	if ( InternalFile == nullptr || IsPipe == true )
	{
		// Could not fsetpos
		return -1;
	}
	return fsetpos( InternalFile, pos );
}
