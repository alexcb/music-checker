#include "log.h"

#include <mpg123.h>

#include <assert.h>
#include <dirent.h>
#include <mpg123.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errors.h"
#include "log.h"
#include "sds.h"
#include "string_utils.h"
#include "timing.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "my_malloc.h"
#include "walk.h"

int main( int argc, char** argv, char** env )
{
	my_malloc_init();

	char* log_level = (char*)sdsnew( getenv( "LOG_LEVEL" ) );
	str_to_upper( log_level );
	set_log_level_from_env_variables( (const char**)env );

	LOG_INFO( "mp3-check starting" );
	//LOG_DEBUG( "here-debug" );
	//LOG_TRACE( "here-trace" );

	//int res;
	//ID3Cache* cache;

	if( argc != 3 ) {
		printf( "%s <albums path> <cache path>\n", argv[0] );
		return 1;
	}
	const char* music_path = argv[1];
	//const char* id3_cache_path = argv[2];

	mpg123_init();
	mpg123_handle* mh = mpg123_new( NULL, NULL );
	if( mh == NULL ) {
		LOG_ERROR( "failed to create new mpg123 handler" );
		return 1;
	}

	int err = walk( music_path );
	if( err != 0 ) {
		LOG_ERROR( "path=%s failed to walk", music_path );
		return 1;
	}

	return 0;
}
