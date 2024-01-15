#include "mp3walk/walk.h"

#include "common/errors.h"
#include "common/log.h"
#include "common/sds.h"
#include "common/string_utils.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wait.h>

int check_file( const char* path )
{

	char* path2 = strdup( path );
	pid_t child_pid;
	char* argv[] = { "cool-program-name", path2, (char*)0 };
	char* envv[] = { (char*)0 };
	int status = -1;
	int s;

	status = posix_spawnp( &child_pid, "./mp3decode", NULL, NULL, argv, envv );
	if( status != 0 ) {
		LOG_ERROR( "spawn failed" );
		return 1;
	}

	do {
		s = waitpid( child_pid, &status, WUNTRACED | WCONTINUED );
		if( s == -1 ) {
			LOG_ERROR( "waitpid failed" );
			return 1;
		}

		if( WIFEXITED( status ) ) {
			printf( "exited, status=%d\n", WEXITSTATUS( status ) );
		}
		else if( WIFSIGNALED( status ) ) {
			printf( "killed by signal %d\n", WTERMSIG( status ) );
		}
		else if( WIFSTOPPED( status ) ) {
			printf( "stopped by signal %d\n", WSTOPSIG( status ) );
		}
		else if( WIFCONTINUED( status ) ) {
			printf( "continued\n" );
		}
	} while( !WIFEXITED( status ) && !WIFSIGNALED( status ) );

	return 0;
}

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
		if( de->d_type == DT_DIR &&
			( strcmp( de->d_name, "." ) == 0 || strcmp( de->d_name, ".." ) == 0 ) ) {
			continue;
		}

		// construct full path
		sdsclear( s );
		s = sdscatfmt( s, "%s/%s", path, de->d_name );

		if( de->d_type == DT_REG && has_suffix( de->d_name, ".mp3" ) ) {
			error_code = check_file( s );
			if( error_code ) {
				goto error;
			}
			continue;
		}

		if( de->d_type == DT_DIR ) {
			error_code = walk( s );
			if( error_code ) {
				goto error;
			}
		}
	}

error:
	if( album_dir ) {
		closedir( album_dir );
	}

	return error_code;
}
