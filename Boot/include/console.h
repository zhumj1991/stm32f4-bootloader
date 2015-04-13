#ifndef _CONSOLE_H_
#define _CONSOLE_H_


int tstc (void);
int ctrlc (void);
int disable_ctrlc (int disable);
int had_ctrlc (void);
void clear_ctrlc (void);


#endif
