#include <assert.h>
#include <dirent.h>
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

#include "mp3walk/walk.h"

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

	if( argc != 2 ) {
		const char* prog_name = "mp3walk";
		if( argc > 0 ) {
			prog_name = argv[0];
		}
		printf( "%s <path>\n", prog_name );
		return 1;
	}
	const char* walk_path = argv[1];

	LOG_INFO( "path=s walking", walk_path );

	int err = walk( walk_path );
	if( err != 0 ) {
		LOG_ERROR( "path=%s failed to walk", walk_path );
		return 1;
	}

	return 0;
}
