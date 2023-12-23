#include "walk.h"

#include "errors.h"
#include "log.h"
#include "sds.h"
#include "string_utils.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

int walk( const char* path )
{
	int error_code = 0;
	struct dirent* de;
	LOG_DEBUG( "path=s opening dir", path );
	sds s = sdsnew( "" );
	DIR* album_dir = opendir( path );
	if( album_dir == NULL ) {
		error_code = FILESYSTEM_ERROR;
		LOG_ERROR( "path=s err=s opendir failed", path, strerror( errno ) );
		goto error;
	}

	while( ( de = readdir( album_dir ) ) != NULL ) {
		if( !has_suffix( de->d_name, ".mp3" ) ) {
			LOG_INFO( "path=s t=d found mp3", de->d_name, de->d_type );
			continue;
		}

		if( de->d_type != DT_DIR || strcmp( de->d_name, "." ) == 0 ||
			strcmp( de->d_name, ".." ) == 0 ) {
			continue;
		}

		// path to album
		sdsclear( s );
		s = sdscatfmt( s, "%s/%s", path, de->d_name );
		error_code = walk( s );
		if( error_code ) {
			goto error;
		}
	}

error:
	if( album_dir ) {
		closedir( album_dir );
	}

	return error_code;
}
