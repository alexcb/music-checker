#include "testRunner.h"
#include <stdio.h>

#include "tests.h"

#include "log.h"

unsigned int testManyFunction( void )
{
	//TEST_ASSERT( !testPlayerLoop() );
	//TEST_ASSERT( !testPlayerSkip() );
	return 0;
}

int main()
{
	set_log_level( LOG_LEVEL_DEBUG );
	return (int)testRunner( testManyFunction );
}
