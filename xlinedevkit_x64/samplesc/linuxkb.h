#ifndef LINUXKB__H
#define LINUXKB__H

void InitialiseConsole( void );
void CloseConsole( void );
void DisableEcho( void );
void EnableEcho( void );
void EnableGetchNewline( void );
void DisableGetchNewline( void );
int _kbhit();
char getch( void );

#endif
