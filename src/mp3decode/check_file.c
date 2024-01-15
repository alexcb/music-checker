#include "mp3decode/check_file.h"

#include "common/log.h"

#include <fcntl.h>
#include <mpg123.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

mpg123_handle* g_check_file_mh;

void error( const char* s )
{
	printf( "hello with %s\n", s );
}

void check_file_init( mpg123_handle* mh_inst )
{
	g_check_file_mh = mh_inst;
}

int check_file( const char* path )
{
	bool done;
	int res;
	int fd = -1;
	LOG_INFO( "path=s checking file", path );
	fd = open( path, O_RDONLY );
	if( fd < 0 ) {
		LOG_ERROR( "unable to open" );
		goto done;
	}

	mpg123_handle* mh = g_check_file_mh;

	if( mpg123_open_fd( mh, fd ) != MPG123_OK ) {
		goto done;
	}

	size_t n = 1024 * 1024;
	char* buffer = malloc( n );

	size_t decoded_size;

	size_t total_bytes_read = 0;

	struct mpg123_frameinfo2 mi;
	res = mpg123_info2( mh, &mi );
	if( res != MPG123_OK ) {
		LOG_ERROR( "path=s err=s failed to get frame info", path, mpg123_plain_strerror( res ) );
	}
	else {
		LOG_INFO( "path=s version=d rate=d", path, mi.version, mi.rate );
	}

	done = false;
	while( !done ) {

		off_t next_frame = mpg123_tellframe( mh );
		res = mpg123_read( mh, (unsigned char*)buffer, n, &decoded_size );
		switch( res ) {
		case MPG123_OK:
			total_bytes_read += decoded_size;
			break;
		case MPG123_NEW_FORMAT:
			LOG_DEBUG( "TODO handle new format" );
			break;
		case MPG123_DONE:
			done = true;
			LOG_INFO( "path=s bytes=d done reading", path, total_bytes_read );
			break;
		default:
			LOG_ERROR( "path=s err=s frame=d unhandled mpg123 error",
					   path,
					   mpg123_plain_strerror( res ),
					   next_frame );
			// TODO handle the error in a better way
			done = true;
			break;
		}
	}

done:
	mpg123_close( mh );
	if( fd >= 0 ) {
		close( fd );
	}
	return 0;
}
