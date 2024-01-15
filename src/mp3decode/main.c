#include <assert.h>
#include <dirent.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/errors.h"
#include "common/log.h"
#include "common/my_malloc.h"
#include "common/sds.h"
#include "common/string_utils.h"
#include "common/timing.h"

#include "mp3decode/check_file.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#define RATE 44100 // TODO move elsewhere?

int main( int argc, char** argv, char** env )
{
	my_malloc_init();

	char* log_level = (char*)sdsnew( getenv( "LOG_LEVEL" ) );
	str_to_upper( log_level );
	set_log_level_from_env_variables( (const char**)env );

	LOG_INFO( "music-check starting" );
	//LOG_DEBUG( "here-debug" );
	//LOG_TRACE( "here-trace" );

	//int res;
	//ID3Cache* cache;

	if( argc != 2 ) {
		const char* prog_name = "mp3decode";
		if( argc > 0 ) {
			prog_name = argv[0];
		}
		printf( "%s <path>\n", prog_name );
		return 1;
	}
	const char* mp3_path = argv[1];

	mpg123_init();
	mpg123_handle* mh = mpg123_new( NULL, NULL );
	if( mh == NULL ) {
		LOG_ERROR( "failed to create new mpg123 handler" );
		return 1;
	}
	check_file_init( mh );

	mpg123_format_none( mh );
	mpg123_format( mh, RATE, MPG123_STEREO, MPG123_ENC_SIGNED_16 );

	LOG_INFO( "path=s decoding mp3", mp3_path );

	int err = check_file( mp3_path );
	if( err != 0 ) {
		LOG_ERROR( "path=%s failed to walk", mp3_path );
		return 1;
	}

	return 0;
}
