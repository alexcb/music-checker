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

typedef int ( *line_callback_func )( const char* l );

bool process_output( int fd, char* buf, size_t buf_size, size_t* buf_n, line_callback_func cb )
{
	bool keep_reading = false;
	int count;
	count = read( fd, buf + *buf_n, buf_size - *buf_n - 1 );
	if( count > 0 ) {
		keep_reading = true;
		*buf_n += count;

		for( ;; ) {
			char* eol = memchr( buf, '\n', *buf_n );
			if( eol ) {
				size_t line_len = eol - buf;
				*eol = '\0';
				cb( buf );
				//printf( "-> %s\n", buf );
				memmove( buf, eol + 1, buf_size - line_len - 1 );
				*buf_n -= ( line_len + 1 );
			}
			else {
				break;
			}
		}
	}
	return keep_reading;
}

int stderr_cb( const char* l )
{
	if( strstr( l, "dequantization failed" ) ) {
		printf( "*** BAD ***\n" );
	}
	printf( "=> %s\n", l );
	return 0;
}

int stdout_cb( const char* l )
{
	printf( "-> %s\n", l );
	return 0;
}

int check_file( const char* path )
{

	char* path2 = strdup( path );

	int pipe_stdout[2]; // 0 -> read, 1 -> write
	int pipe_stderr[2]; // 0 -> read, 1 -> write

	pipe( pipe_stdout );
	pipe( pipe_stderr );

	pid_t child_pid = fork();
	if( !child_pid ) {
		// child process

		// close reading side of pipe
		close( pipe_stderr[0] );
		close( pipe_stdout[0] );

		char* argv[] = { "./mp3decode", path2, (char*)0 };
		//char* argv[] = { "/bin/sh", "-c", "hostname", (char*)0 };

		dup2( pipe_stdout[1], STDOUT_FILENO );
		close( pipe_stdout[1] );

		dup2( pipe_stderr[1], STDERR_FILENO );
		close( pipe_stderr[1] );

		execv( argv[0], argv );
		exit( 1 );
	}
	else if( child_pid > 0 ) {
		size_t stdout_buffer_n = 0;
		size_t stderr_buffer_n = 0;
		size_t stdout_buffer_size = 10240;
		size_t stderr_buffer_size = 10240;
		char stdout_buffer[stdout_buffer_size];
		char stderr_buffer[stderr_buffer_size];
		printf( "waiting for pid %d\n", child_pid );

		// close writing side of pipe
		close( pipe_stderr[1] );
		close( pipe_stdout[1] );

		fcntl( pipe_stderr[0],
			   F_SETFL,
			   fcntl( pipe_stderr[0], F_GETFL ) | O_NONBLOCK ); // TODO error check
		fcntl( pipe_stdout[0],
			   F_SETFL,
			   fcntl( pipe_stdout[0], F_GETFL ) | O_NONBLOCK ); // TODO error check

		// Read from child’s stdout
		bool child_exited = false;
		for( ;; ) {
			bool keep_reading = false;

			if( process_output( pipe_stderr[0],
								stderr_buffer,
								stderr_buffer_size,
								&stderr_buffer_n,
								stderr_cb ) ) {
				keep_reading = true;
			}

			if( process_output( pipe_stdout[0],
								stdout_buffer,
								stdout_buffer_size,
								&stdout_buffer_n,
								stdout_cb ) ) {
				keep_reading = true;
			}

			if( child_exited ) {
				if( keep_reading ) {
					continue;
				}
				break;
			}

			int status = 0;
			int s = waitpid( child_pid, &status, WNOHANG );
			if( s == 0 ) {
				// process is still running
				continue;
			}
			else if( s < 0 ) {
				// waitpid error
				break;
			}
			printf( "process %d returned with status %d\n", s, status );

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
			break;
		}
	}
	else {
		printf( "fork failed %d\n", child_pid );
	}
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
