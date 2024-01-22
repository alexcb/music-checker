#include <assert.h>
#include <dirent.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/errors.h"
#include "common/log.h"
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
	set_log_level_from_env_variables( (const char**)env );

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
		LOG_ERROR( "path=s err=d mp3decode failed", mp3_path, err );
		return 1;
	}

	// uncomment to test mp3walk's handling of line parsing
	// printf( "good\nbye\nworld\n" );
	return 0;
}
