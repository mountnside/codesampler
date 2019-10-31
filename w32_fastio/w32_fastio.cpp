//-----------------------------------------------------------------------------
//           Name: w32_fastio.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Demonstrates Fast sequential IO on a 1 GB file using the 
//                 FILE_FLAG_NO_BUFFERING flag.
//
//   Command Line: w <file_name> = Regular write test
//                 W <file_name> = Fast IO write test
//                 r <file_name> = Regular read test
//                 R <file_name> = Fast IO read test
//                 A <file_name> = Fast IO read test (aligned buffer)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// There are three restrictions to using the FILE_FLAG_NO_BUFFERING flag...
//
//
// Restriction #1:
//
// Buffer addresses for read and write operations should be sector aligned 
// (aligned on addresses in memory that are integer multiples of the volume's 
// sector size). Depending on the disk, this requirement may not be enforced. 
//
// Alignment can be guarnateed by using VirtualAlloc(). See code...
//
// --------------------------------------------------------------------------
//
// Restriction #2:
//
// File access must begin at byte offsets within the file that are integer 
// multiples of the volume's sector size.
//
// So calling ReadFile() after setting the file pointer to the second byte in 
// the file will fail with a fast IO set up...
//
// SetFilePointer( hFile, 1, 0, FILE_BEGIN ); // Bad...
//
// But staring at the begining is always safe becasue the file's first byte 
// should always be sector aligned by default.
//
// SetFilePointer( hFile, 0, 0, FILE_BEGIN ); // Good...
//
// --------------------------------------------------------------------------
//
// Restriction #3:
//
// File access must be for numbers of bytes that are integer multiples of the 
// volume's sector size. For example, if the sector size is 512 bytes, an 
// application can request reads and writes of 512, 1024, or 2048 bytes, 
// but not of 335, 981, or 7171 bytes.
//
// This will fail...
//
// ReadFile( hFile, byteBuffer, 1000, &dwBytes, NULL );
//
// This is fine...
//
// ReadFile( hFile, byteBuffer, 1024, &dwBytes, NULL );
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 
// Note on performance timing read/write operations:
//
// The trick to getting realistic read/write times during testing requires 
// you to outsmart the OS cache. After writing a standard size file, the file
// will most likely be cached by the OS, so a buffered read that happens 
// to follow afterwards can seem very fast when in fact its merely pulling 
// form the cache instead of actually reading from the hard-disk media. 
// Even a file created by the last run of the test program can remain resident 
// in the buffer long enough to effect subsequent test runs.
//
// One way to combat this caching is to generate a test file so large that
// the OS refuses to cache it, so this test case uses a 1 GB size file for
// testing purposes.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>

// Use 64 KB chunks for reading and writing
#define BUFFER_SIZE_K (64)

// Same as BUFFER_SIZE_K, but converted to bytes
#define BUFFER_SIZE_B (BUFFER_SIZE_K * 1024)

// Our test file is 1 GB in size (1,048,576 KB or 1,073,741,824 bytes)
#define FILE_SIZE_K (1 * 1024 * 1024)

// Number of read/write operations required to read or write the whole 
// test file with a for() loop.
#define NUM_READ_WRITE ((FILE_SIZE_K) / (BUFFER_SIZE_K))

HANDLE hFile;
_timeb start;
_timeb finish;
float elapsed = 0.0f;
DWORD dwBytes = 0;
BOOL bStatus = TRUE;
char byteBuffer[BUFFER_SIZE_B];
int i;

//-----------------------------------------------------------------------------
// Just incase you forget your memory units...
// 1 KB = 1,024 bytes
// 1 MB = 1000 KB = 1,048,576 bytes
//-----------------------------------------------------------------------------

void write(const char *fileName);
void fastWrite(const char *fileName);
void read(const char *fileName );
void fastRead(const char *fileName);
void fastReadUsingAlignedBuffer(const char *fileName);

void main( int argc, char *argv[] )
{
	if( argc == 3 )
	{
		switch( *argv[1] )
		{
		case 'w':
			write( argv[2] );
			break;

		case 'W':
			fastWrite( argv[2] );
			break;

		case 'r':
			read( argv[2] );
			break;

		case 'R':
			fastRead( argv[2] );
			break;

        case 'A':
			fastReadUsingAlignedBuffer( argv[2] );
			break;

		default: break;
		}
	}
	else
	{
		write( "testfile1" );
		fastWrite( "testfile2" );
		read( "testfile1" );
		fastRead( "testfile2" );
        fastReadUsingAlignedBuffer( "testfile1" );
	}
}

//-----------------------------------------------------------------------------
// Regular write using 64 KB chuncks...
//-----------------------------------------------------------------------------
void write( const char *fileName )
{
	printf( "Start regular IO write test...\n\n" );

    hFile = CreateFile( fileName,                     // file name
                        GENERIC_READ | GENERIC_WRITE, // desired access
                        0,                            // share mode (none)
                        NULL,                         // security attributes
                        OPEN_ALWAYS,                  // create it, if it doesn't exist
                        NULL,                         // flags & attributes
                        NULL );                       // file template

    if( hFile == INVALID_HANDLE_VALUE )
	{
		printf( "CreateFile() failed!\n" );
		return;
	}

    // Place a recognizable test pattern into the byteBuffer
	for( i = 0; i < BUFFER_SIZE_B; ++i )
		byteBuffer[i] = i % 10;

    printf("Writing %d KB to %s\n", FILE_SIZE_K, fileName);

	_ftime( &start );

    // Resulting file size will be (BUFFER_SIZE_B * NUM_READ_WRITE) or 64 MB
	for( i = 0; i < NUM_READ_WRITE; ++i )
		WriteFile( hFile, byteBuffer, BUFFER_SIZE_B, &dwBytes, NULL );

    _ftime( &finish );

    elapsed  = (float)(finish.time - start.time); // This is accurate to one second
    elapsed += (float)(finish.millitm - start.millitm)/1000.0f; // This gets it down to one ms

	printf( "Time to write %d KB = %fs\r\n\n", FILE_SIZE_K, elapsed );

	bStatus = CloseHandle( hFile );

	if( !bStatus )
	{
		printf( "CloseHandle() failed!\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
// Fast write using 64 KB chuncks...
//-----------------------------------------------------------------------------
void fastWrite( const char *fileName )
{
	printf( "Start fast IO write test...\n\n" );

    hFile = CreateFile( fileName,                     // file name
                        GENERIC_READ | GENERIC_WRITE, // desired access
                        0,                            // share mode (none)
                        NULL,                         // security attributes
                        OPEN_ALWAYS,                  // create it, if it doesn't exist
                        FILE_FLAG_NO_BUFFERING |      // flags & attributes
						FILE_FLAG_SEQUENTIAL_SCAN,    // flags & attributes
                        NULL );                       // file template

    if( hFile == INVALID_HANDLE_VALUE )
	{
		printf( "CreateFile() failed!\n" );
		return;
	}

    // Place a recognizable test pattern into the byteBuffer
	for( i = 0; i < BUFFER_SIZE_B; ++i )
		byteBuffer[i] = i % 10;

    printf("Writing %d KB to %s\n", FILE_SIZE_K, fileName);
    
	_ftime( &start );

	for( i = 0; i < NUM_READ_WRITE; ++i )
		WriteFile( hFile, byteBuffer, BUFFER_SIZE_B, &dwBytes, NULL );

    _ftime( &finish );

    elapsed  = (float)(finish.time - start.time); // This is accurate to one second
    elapsed += (float)(finish.millitm - start.millitm)/1000.0f; // This gets it down to one ms

	printf( "Time to write %d KB = %fs\r\n\n", FILE_SIZE_K, elapsed );

	bStatus = CloseHandle( hFile );

	if( !bStatus )
	{
		printf( "CloseHandle() failed!\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
// Regular IO read using 64 KB chuncks...
//-----------------------------------------------------------------------------
void read( const char *fileName )
{
	printf( "Start regular IO read test...\n\n" );

    hFile = CreateFile( fileName,                     // file name
                        GENERIC_READ | GENERIC_WRITE, // desired access
                        0,                            // share mode (none)
                        NULL,                         // security attributes
                        OPEN_EXISTING,                // fail if it doesn't exist
                        NULL,                         // flags & attributes
                        NULL );                       // file template

    if( hFile == INVALID_HANDLE_VALUE )
	{
		printf( "CreateFile() failed!\n" );
		return;
	}

	// clear out buffer
	for( i = 0; i < BUFFER_SIZE_B; ++i )
		byteBuffer[i] = 0;

	_ftime( &start );

	for( i = 0; i < NUM_READ_WRITE; ++i )
		ReadFile( hFile, byteBuffer, BUFFER_SIZE_B, &dwBytes, NULL );

    _ftime( &finish );

    elapsed  = (float)(finish.time - start.time); // This is accurate to one second
    elapsed += (float)(finish.millitm - start.millitm)/1000.0f; // This gets it down to one ms

    /*
	printf( "Verify byte buffer content...\n" );
	for( i = 0; i <= 100; ++i )
		printf( "%d ", byteBuffer[i] );
    printf( "\n\n" );
    */

	printf( "Time to read %d KB = %fs\r\n\n", FILE_SIZE_K, elapsed );

	bStatus = CloseHandle( hFile );

	if( !bStatus )
	{
		printf( "CloseHandle() failed!\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
// Fast IO read using 64 KB chuncks...
//-----------------------------------------------------------------------------
void fastRead( const char *fileName )
{
	printf( "Start fast IO read test...\n\n" );

    hFile = CreateFile( fileName,                     // file name
                        GENERIC_READ | GENERIC_WRITE, // desired access
                        0,                            // share mode (none)
                        NULL,                         // security attributes
                        OPEN_EXISTING,                // fail if it doesn't exist
                        FILE_FLAG_NO_BUFFERING |      // flags & attributes
						FILE_FLAG_SEQUENTIAL_SCAN,    // flags & attributes
                        NULL );                       // file template

    if( hFile == INVALID_HANDLE_VALUE )
	{
		printf( "CreateFile() failed!\n" );
		return;
	}

	// clear out buffer
	for( i = 0; i < BUFFER_SIZE_B; ++i )
		byteBuffer[i] = 0;

	_ftime( &start );

	for( i = 0; i < NUM_READ_WRITE; ++i )
		ReadFile( hFile, byteBuffer, BUFFER_SIZE_B, &dwBytes, NULL );

    _ftime( &finish );

    elapsed  = (float)(finish.time - start.time); // This is accurate to one second
    elapsed += (float)(finish.millitm - start.millitm)/1000.0f; // This gets it down to one ms

    /*
	printf( "Verify byte buffer content...\n" );
	for( i = 0; i <= 100; ++i )
		printf( "%d ", byteBuffer[i] );
    printf( "\n\n" );
    */

	printf( "Time to read %d KB = %fs\r\n\n", FILE_SIZE_K, elapsed );

	bStatus = CloseHandle( hFile );

	if( !bStatus )
	{
		printf( "CloseHandle() failed!\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
// Fast IO read using 64 KB chuncks and an align buffer...
//-----------------------------------------------------------------------------
void fastReadUsingAlignedBuffer( const char *fileName )
{
	printf( "Start fast IO read test (aligned buffer)...\n\n" );

	// Open the file
    hFile = CreateFile( fileName,                     // file name
                        GENERIC_READ | GENERIC_WRITE, // desired access
                        0,                            // share mode (none)
                        NULL,                         // security attributes
                        OPEN_EXISTING,                // fail if it doesn't exist
                        FILE_FLAG_NO_BUFFERING |      // flags & attributes
						FILE_FLAG_SEQUENTIAL_SCAN,    // flags & attributes
                        NULL );                       // file template

    if( hFile == INVALID_HANDLE_VALUE )
	{
		printf( "CreateFile() failed!\n" );
		return;
	}

    //-------------------------------------------------------------------------
    // Don't use the normal byteBuffer, instead lets ensure sector 
    // alignment by creating a new buffer with VirtualAlloc().
    //-------------------------------------------------------------------------
    char *alignedBuffer = (char*)VirtualAlloc( NULL, BUFFER_SIZE_B, 
                                               MEM_COMMIT, PAGE_READWRITE );
    memset( alignedBuffer, '\0', BUFFER_SIZE_B );

	_ftime( &start );

	for( i = 0; i < NUM_READ_WRITE; ++i )
        ReadFile( hFile, alignedBuffer, BUFFER_SIZE_B, &dwBytes, NULL );

    _ftime( &finish );

    elapsed  = (float)(finish.time - start.time); // This is accurate to one second
    elapsed += (float)(finish.millitm - start.millitm)/1000.0f; // This gets it down to one ms

    /*
	printf( "Verify byte buffer content...\n" );
	for( i = 0; i <= 100; ++i )
		printf( "%d ", alignedBuffer[i] );
    printf( "\n\n" );
    */

	printf( "Time to read %d KB = %fs\r\n\n", FILE_SIZE_K, elapsed );

	bStatus = CloseHandle( hFile );

	if( !bStatus )
	{
		printf( "CloseHandle() failed!\n" );
		return;
	}

    VirtualFree( alignedBuffer, 0, MEM_RELEASE );
}

