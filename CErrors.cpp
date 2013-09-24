#include "stdafx.h"
//#include "CErrors.h"
#include <windows.h>
#include "PTemplates.h"

char Caption[] = "Parsing error.";
extern char _ErrMessage_[255] = "";
char cNULL[] = "<NULL>";

void AddMessage(char * Message)
{
	MessageBox(0, Message, Caption, MB_OK);
}

void messagef(char * Message, ...)
{
	// %% = %
	// %d = int
	// %s = char* (0)
	// %n = char* (~Letter)
	// %o = char* (~Op)
	// %x = int hex
	int * op = (int *) &Message + 1;
	char * t;
	int i;
	char * Res;
	for(Res = _ErrMessage_; *Message; Message++)
	{
		if(*Message == '%')
		{
			switch(*(++Message))
			{
			case '%': *(Res++) = '%'; continue;
			case 's': for(t = *op ? (char *) *op : cNULL;           *t;t++) *(Res++) = *t; op++; continue;
			case 'n': for(t = *op ? (char *) *op : cNULL; IsLetter(*t);t++) *(Res++) = *t; op++; continue;
			case 'o': for(t = *op ? (char *) *op : cNULL;   IsSign(*t);t++) *(Res++) = *t; op++; continue;
			case 'd': if(i = *(op++))
					  {
						  if(i < 0) { i = -i; *(Res++) = '-';}
						  for(*(t = Res + 10) = 0; i; i /= 10) *(--t) = '0' + i % 10;
						  while(*t) *(Res++) = *(t++);
					  } else *(Res++) = '0';
					continue;
			case 'x': i = *(op++);
					 for(int c = 28; c >= 0; c -= 4)
					  {
						 char d = (i >> c) & 15;
						 if(d < 10) d += '0';
						 else d += 'A' - 10;
						 *(Res++) = d;
					  }
					continue;
			}
		}
		*(Res++) = *Message;
	}
	*Res = 0;
	MessageBox(0, _ErrMessage_, Caption, MB_OK);
}