#pragma once

#include <mpg123.h> // TODO forward declare mpg123_handle

void check_file_init( mpg123_handle* mh_inst );

int check_file( const char* path );
