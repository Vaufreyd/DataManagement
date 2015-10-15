/**
 * @file DataFile.cpp
 * @ingroup DataManagement
 * @author Dominique Vaufreydaz, Grenoble Alpes University, Inria
 * @copyright All right reserved.
 */

#include "DataFile.h"

#include <sys/stat.h>
#include <stdlib.h>

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

/** @brief Static function to check if a file exists 
 */
static bool FileExists(  const char * FileName )
{
	struct stat FileStat;

	if ( stat(FileName, &FileStat) == 0 )
	{
		// File exists
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
}

/** @brief Virtual destructor, always.
 */
DataFile::~DataFile()
{
	Close();
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
	const char * ModeRead = "rb";
	const char * ModeWrite = "wb";

	if ( InternalFile != nullptr )
	{
		fprintf( stderr, "Could not reopen file, please call DataFile::Close() before.\n" );
		return false;
	}

	// By default it is not a pipe
	IsPipe = false;

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
		return false;
	}

	// File not found, or at least not opened

	// Here, we can not manage to open file for writing, we do not try it compressed
	if ( eMode != READ_MODE )
	{
		// TODO: add support for output compression
		return false;
	}

	// Try to open a 7z compressed version of the file
	char Tmpc[1024];
	char init_path[1024]; 
	sprintf( init_path, "%s.7z", Filename );

	// If there is a 7zip version of the file,
	// generate a pipe command to open it
#if defined WIN32 || defined WIN64 
	if ( FileExists( init_path ) == false )
	{
		return false;
	}
	sprintf( Tmpc, "7z e -so \"%s.7z\"", Filename );
#else
	char resolved_path[1024]; 
    realpath( init_path, resolved_path); 

	if ( FileExists( resolved_path ) == false )
	{
		return false;
	}

	sprintf( Tmpc, "7z e -so \"%s\"", resolved_path );
#endif

	// Try to the open the pipe
	if ( Pipe::Open(Tmpc, Pipe::READ_MODE ) == true )
	{
		IsPipe = true;
		return true;
	}

	return false;
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
	if ( InternalFile != nullptr )
	{
		return fwrite( ptr, size, nmemb, InternalFile );
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
	if ( InternalFile == nullptr || IsPipe == true )
	{
		// Could not seek
		return -1;
	}
	return fseeko( InternalFile, offset, whence );
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
	return ftello(InternalFile);
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
	return rewind(InternalFile);
}

/** @brief Retrive current position in a file/pipe as a fpos_t_ structure (identical to fgetpos).
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

/** @brief Retrive current position in a file/pipe as a fpos_t_ structure (identical to fsetpos).
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
