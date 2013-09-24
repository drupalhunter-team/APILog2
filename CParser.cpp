
#include "stdafx.h"
#include <stdio.h>
#include "PTemplates.h"
#include "CParser.h"
#include <malloc.h>
#include "CErrors.h"

TSyntaxParser * PreParser;

typedef struct _DEFINE {
	char * Name;
	char * Value;
	int Type;
} DEFINE, * PDEFINE;


//DEFINE * Defines = 0;
//int DefCount = 0;
#define rAlign 64
#define NextLine 10
char * so;

// Интерпретатор препроцессора ///////////////////////////////////////



bool Plus(int * Val1, int * Val2, int * Result)
{
	*Result = *Val1 + *Val2;
	return true;
}

bool Minus(int * Val1, int * Val2, int * Result)
{
	int v1 = 0;
	if(Val1) v1 = *Val1;
	*Result = v1 - *Val2;
	return true;
}

bool uMinus(int * Val1, int * Val2, int * Result)
{
	*Result = - *Val2;
	return true;
}

bool Mul(int * Val1, int * Val2, int * Result)
{
	*Result = *Val1 * *Val2;
	return true;
}

bool oBR(int * Val1, int * Val2, int * Result)
{
	*Result = (*Val1 >= *Val2);
	return true;
}

bool oB(int * Val1, int * Val2, int * Result)
{
	*Result = (*Val1 > *Val2);
	return true;
}

OPERATOR Operators[] = {
	{"(",  4, OP_BROPEN | OP_PREFIX | OP_BINARY, 0},
	{")",  4, OP_BRCLOSE | OP_POSTFIX, 0},
	{">=", 1, OP_STDBIN, (TFOperator) oBR},
	{">",  1, OP_STDBIN, (TFOperator) oB},

	{"+",  2, OP_STDBIN, (TFOperator) Plus},
	{"-",  2, OP_STDBIN | OP_PREFIX, (TFOperator) Minus},
	{"*",  3, OP_STDBIN, (TFOperator) Mul},
	//{'/',  3, OP_STDBIN}
};


//////////////////////////////////////////////////////////////////////

/*char * strcmpend(char * s, char * subs)
{
	for( ; *subs; subs++)
	if((!*s) || (*(s++) != *subs)) return 0;
	return s;
}*/

int LineNum(char * s, char * c)
{
	int r = 1;
	for( ; s != c; s++)
	if(*s == NextLine) r++;
	return r;
}

extern char * mCaption = "Parsing error.";
extern char Message[255] = "";

#define FIND_DEF (char *) -1

/*DEFINE * GetDefine(char * Name, char * Value)
{
	for(int x = 0; x < DefCount; x++)
	{
		if(!strcmp(Name, Defines[x].Name))
		{
			if(Value != FIND_DEF)
			{
				sprintf(Message, "(%d) Directive redefined: %s", LineNum(so, Name), Name);
				AddMessage;
				Defines[x].Name = Name;
			}
			return Defines + x;
		}
	}

	if(Value == FIND_DEF) return 0;

	DEFINE * r = Defines + DefCount;
	if(!((rAlign - 1) & (DefCount++))) // Требуется увеличить массив.
	Defines = (DEFINE *) realloc(Defines, ((DefCount | (rAlign - 1)) + 1) * sizeof(DEFINE));
	r->Name = Name;
	r->Value = Value;
	return r;
}*/


void CleanComments(char * s)
{
	for(char * r = s; *s; s++)
	{
		switch(*s)
		{
		case '/': // Отсечь комментарии
			if(s[1] == '/')
			{
				for(s++; (*s != 13) && *s; s++);
				*(r++) = *s;
				continue;
			}
			if(*(s + 1) == '*')
			{
				s++;
				if(!*s)break;
				for(s ++; ((*s != '/') || (*(s - 1) != '*')) && *s; s++)
				if((*s == 10)||(*s == 13)) *(r++) = *s;
				continue;
			}
		}
		*(r++) = *s;
	}
}

char * ReadValue(char * Text, VALUE * Result) // Text[0] - Letter, Text[1] - доступен
{
	int S = 10;
	if(*Text == '0')
	{
		S = 8;
		if(*(++Text) == 'x')
		{
			S = 16;
			Text++;
		}
	}

	int Res = 0;
	for( ; IsLetter(*Text); Text++)
	{
		char b = *Text;
		int r;
		if((b >= '0') && (b <= '9')) r = b - '0';
		else
		if((b >= 'a') && (b <= 'f')) r = b - 'a' + 10;
		else
		if((b >= 'A') && (b <= 'F')) r = b - 'A' + 10;
		else return 0;
		if(r >= S) return 0;
		Res = Res * S + r;
	}
	Result->Value = (void *) Res;
	Result->Type = VL_INITED;
	return Text;
}

//#define ScanName

int CParser(char * FileName)
{
	PreParser = new TSyntaxParser(Operators, sizeof(Operators) / sizeof(OPERATOR), ReadValue);
	HANDLE f = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DWORD l = GetFileSize(f, 0);
	char * s = new char[l + 1];
	DWORD t;
	ReadFile(f, s, l, &t, 0);
	s[l] = 0;
	so = s;
	CloseHandle(f);
	CleanComments(s);
	int IfCount = 0;
	int Disabled = 0;

	for(; *s; s++)
	{
		switch(*s)
		{
		case '#':
			s++;
			if(char * t = CompareLetters(s, "define"))
			{
				for(s = t; *s == ' '; s++);
				t = s; // t - имя макрозамены
				for(s = t; *s > ' '; s++); // s - конец имени макрозамены, начало строки замены
				if(!Disabled)
				{
					VALUE Value;
					Value.Value = s;
					PreParser->AddVariable(&t, &Value);
				}

				continue;
			}
			if(char * t = CompareLetters(s, "if"))
			{
				for(s = t; (*s != 13) && *s; s++);
				IfCount++;
				if(!Disabled)
				{
					char b = *s;
					*s = 0;
					VALUE * R;
					if(!(R = PreParser->Parse("(73)(8)*6"))) return 0;
//					if(!(R = PreParser->Parse(t))) return 0;
					if(!R->Value) Disabled = IfCount;
					*s = b;
				}
				continue;
			}
			if(char * t = CompareLetters(s, "endif"))
			{
				if(IfCount == 0)
				{
					sprintf(Message, "(%d) Unexpected directive: #else", LineNum(so, s));
//					AddError;
				}
				IfCount--;
				if(Disabled > IfCount) Disabled = 0;
				s = t;
				continue;
			}
			if(char * t = CompareLetters(s, "error"))
			{
				for(s = t; (*s != 13) && *s; s++);
				if(!Disabled)
				{
					char b = *s;
					*s = 0;
					sprintf(Message, "(%d) Error: %s", LineNum(so, t), t);
//					AddError;
					*s = b;
				}
				continue;
			}
			if(char * t = CompareLetters(s, "pragma"))
			{
				for(s = t; *s == ' '; s++);
				if(t = CompareLetters(s, "message"))
				{
					if(!Disabled)
					{
						for(s = t; (*s != 13) && *s; s++);
						char b = *s;
						*s = 0;
						sprintf(Message, "(%d) Message: %s", LineNum(so, t), t);
//						AddMessage;
						*s = b;
					}
					continue;
				}
				sprintf(Message, "(%d) Unknown pragma: %s", LineNum(so, s), s);
//				AddMessage;
			}
			if(char * t = CompareLetters(s, "ifndef"))
			{
				for(s = t; *s == ' '; s++);
				t = s;
				for(s = t; *s > ' '; s++);
				*s = 0;
				IfCount++;
				if(!Disabled && PreParser->FindVariable(&t))
				Disabled = IfCount;
				continue;
			}
			if(char * t = CompareLetters(s, "ifdef"))
			{
				for(s = t; *s == ' '; s++);
				t = s;
				for(s = t; *s > ' '; s++);
				*s = 0;
				IfCount++;
				if(!Disabled && !PreParser->FindVariable(&t))
				Disabled = IfCount;
				continue;
			}
			if(char * t = CompareLetters(s, "else"))
			{
				if(IfCount == 0)
				{
					sprintf(Message, "(%d) Unexpected directive: #else", LineNum(so, s));
//					AddError;
				}
				if(Disabled == IfCount) Disabled = 0;
				else if(Disabled == 0) Disabled = IfCount;
				continue;
			}
			char * m = s - 1;
			int n = LineNum(so, s);
			for(s++; (*s != NextLine) && *s; s++);
			*s = 0;
			sprintf(Message, "(%d) Unknown directive: %s", n, m);
//			AddError;

			*s = NextLine;
			continue;

		}









	}

	delete so;
	return 0;
}