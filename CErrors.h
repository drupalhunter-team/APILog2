#ifndef _CErrors_
#define _CErrors_
//#define ADD_MESSAGE { sprintf(_ErrMessage_, 
//#define HALT  ); AddMessage(_ErrMessage_); return 0;}
//#define END  ); AddMessage(_ErrMessage_);}

#define ADD_MESSAGE { messagef(
#define HALT  ); return 0;}
#define HALTv  ); return;}
#define END  );}

void AddMessage(char * Message);
void messagef(char * Message, ...);

extern char _ErrMessage_[255];

#endif