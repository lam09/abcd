#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

const int fd = STDIN_FILENO;
struct termios orig_ts, new_ts;
static int peek = -1;
static int ConsoleInitialised = 0;
static int GetchNewline = 0;

void InitialiseConsole( void )
{
	tcgetattr( 0, &orig_ts );
	new_ts = orig_ts;
	new_ts.c_lflag &= ~ICANON;
	new_ts.c_lflag &= ~ECHO;
	new_ts.c_lflag &= ~ISIG;
	new_ts.c_cc[VMIN] = 1;
	new_ts.c_cc[VTIME] = 0;
	tcsetattr( 0, TCSANOW, &new_ts );

	ConsoleInitialised = 1;
}

void CloseConsole( void )
{
	if ( ConsoleInitialised == 1 ) tcsetattr( 0, TCSANOW, &orig_ts );

	ConsoleInitialised = 0;
}

void DisableEcho( void )
{
	new_ts.c_lflag &= ~ECHO;
	tcsetattr( 0, TCSANOW, &new_ts );
}

void EnableEcho( void )
{
	new_ts.c_lflag |= ECHO;
	tcsetattr( 0, TCSANOW, &new_ts );
}

void EnableGetchNewline( void )
{
	GetchNewline = 1;
}

void DisableGetchNewline( void )
{
	GetchNewline = 0;
}

int _kbhit()
{
	char ch;
	int nread;

	if( peek != -1 ) return( 1 );

	new_ts.c_cc[VMIN] = 0;
	tcsetattr( 0, TCSANOW, &new_ts );
	nread = read( 0, &ch, 1 );
	new_ts.c_cc[VMIN] = 1;
	tcsetattr( 0, TCSANOW, &new_ts );

	if( nread == 1 )
	{
		peek = ch;
		return( 1 );
	}

	return( 0 );
}

char getch( void )
{
	char ch;
	#ifdef X10_LINUX_BUILD
		int ignore __attribute__ ((unused));
	#else
		int ignore;
	#endif

	if( peek != -1 )
	{
		ch = peek;
		peek = -1;
		return( ch );
	}

	if( GetchNewline == 1 ) printf( "\n" );

	ignore = read( fd, &ch, 1 );					// ignore used to quell g++ compiler warning

	return( ch );
}
