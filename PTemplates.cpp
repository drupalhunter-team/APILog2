#include "stdafx.h"
#include "PTemplates.h"
#include "CParser.h"
#include "CErrors.h"
#include <stdio.h>
#include <windows.h>

#define IsLetter(c) (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'))
#define IsDigit(c) ((c >= '0') && (c <= '9'))
#define IsValue(c) (((c >= '0') && (c <= '9')) || (c == '"') || (c == 39))

// ������� ���������� ����� ��������������
int NameLen(char * a)
{
	int l;
	for(l = 0; IsLetter(*a); a++) l++;
	return l;
}

// ������� ��������� ��������� �� �������. ���� ��������� �������, �� ������������ ��������� �� ������ ������ ����� ���������
char * CompareOperators(char * a, char * b)
{
	while(*b)
	if(*(a++) != *(b++)) return false;
	return (*b == 0) ? a : 0;
}

// ������� ��������� ����. ���� ��������� �������, �� ������������ ��������� �� ������ ������ ����� ����� a.
// ���������� ��� ��������� ����� ������ �� IsLetter()
char * CompareLetters(char * a, char * b)
{
	while(IsLetter(*a))
	if(*(a++) != *(b++)) return 0;
	return IsLetter(*a) == IsLetter(*b) ? a : 0;
}

int TSyntaxParser::GetOperatorIndex(OPERATOR * Operator)
{
	int r = (Operator - Operators) / sizeof(OPERATOR);
	if(r >= OpCount) r = -1;
	return r;
}


void Compute(STACKREC * Src, STACKREC * Dst) 
/**************************************************************************************************************

		* ���������� ���������� *

	������� ������ � �������� ������� ����������:

 | ��� ��������� |           ����������� ���������                      |    Var1    |    Var2    |  Result ��  | Result ����� |
 |---------------|------------------------------------------------------|------------|------------|-------------|--------------|
 |   OP_BINARY   | Dst->Value = Src->Value x Dst->Value                 | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |  NullOperator | Dst->Value = Src->Value Dst->Value                   | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |   OP_PREFIX   | Dst->Value = x Dst->Value                            |      0     | Dst->Value |      0      |  Dst->Value  |
 |   OP_POSTFIX  | Dst->Value = Src->Value x                            | Src->Value |      0     |      0      |  Dst->Value  |
 |  OP_BRACKETS  | Dst->Value = Src->Value ( Dst->Value )               | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |  OP_BRTRIVAR  | Dst->Value = Src[-1].Value ? Src->Value : Dst->Value | Src->Value | Dst->Value |Src[-1].Value|  Dst->Value  |
	
		��� 'x', '()', '?:' - ����������� �������� �� Src->Operator.

	������� ��������� ������ ���������� � ���� Src->Operator->Function. ��� �������������� ������ ����������� �������
	�� ����������� ������, ��� ��� ���������� - �����������. ����������� ������������ ������ ��������� � ������� 
	��������� ��� ����� � ��� �� ����������� ������: OP_PREFIX (��������: ������ ��������������� ���������,
	����� �������������� ����) � OP_BINARY (��������: ����� �������, ����� �������������� ����, ��������� �
	�������� �������).

    ���������� ��������� � ���������.

 | ���� ��������� | ��������                               |
 |----------------|----------------------------------------|
 |    OP_INITx	  | �������� x ������ ���� ���������������.|
 |    OP_LEFTx	  | �������� x ������ ���� ��������������. |
 |    OP_NAMEx	  | �������� x ������ ����� ���.           |

    �������� ���������� ���������.

 | ���� ��������� | ��������                   |
 |----------------|----------------------------|
 |    OP_INITR	  | ��������� ���������������. |
 |    OP_LEFTR	  | ��������� ������������.    |

	��������� �� ����� ����� ����� VL_VALISVAR � VL_VALISNAME.

    ������� �������� ��������:
  1. ����������: VL_VALISVAR.
  2. �������� �����������������: VL_INITED. ���� VL_VALISVAR, �� �������� � ������ ����������.
  3. [�����������] ���: VL_VALISNAME.
  4. ���������������� VL_LEFT.

  ���������������� ���������� ����� ����������� ��:
  1. ���������. ������: �������� ������������� ��������� *.
  //2. ����� ����������� ���������������� ��������.



typedef bool (* TFOperator) (VALUE * Var1, VALUE * Var2, VALUE * Result);	// ��������� �� ������� ����������� ��������

  �������� ��������� �������� ��������� (x: A, B)

	���� OP_NAMEx ��
		���� VL_VALISNAME �� VALUE.Value
		���� VL_VALISVAR �� ((VARIABLE *)VALUE.Value)->Name
		����� ������: ��� �����������.
	����� ��������� ����������������. �����
		���� VL_VALISVAR �� ((VARIABLE *)VALUE.Value)->Value
		����� VALUE
		���������, ��������������� �� ���������.

  ���� ������������ Result ��� �������, �� ����� ��������, ��������� ��������� ���������� � Dst->Value

**************************************************************************************************************/
{
	if(!Src->Operator->Function) return; // ������ ���������.
	void * Var1 = 0, * Var2 = 0;
	int Flags = Src->Operator->Flags;
	if(Src->Value.Type)
	{
		if(Flags & OP_NAMEA)
		{
			if(Src->Value.Type & VL_VALISNAME) Var1 = Src->Value.Value;
			else if(Src->Value.Type & VL_VALISVAR) Var1 = ((VARIABLE *)Src->Value.Value)->Name;
			else {
				// ��� �����������!!!
			}
		}
		else
		{
			if(Src->Value.Type & VL_VALISVAR) Var1 = &(((VARIABLE *) Src->Value.Value)->Value);
			else Var1 = &(Src->Value);
		}

	}
	if(Dst->Value.Type)
	{
		if(Flags & OP_NAMEB)
		{
			if(Dst->Value.Type & VL_VALISNAME) Var2 = Dst->Value.Value;
			else if(Dst->Value.Type & VL_VALISVAR) Var2 = ((VARIABLE *)Dst->Value.Value)->Name;
			else {
				// ��� �����������!!!
			}
		}
		else
		{
			if(Dst->Value.Type & VL_VALISVAR) Var2 = &(((VARIABLE *) Dst->Value.Value)->Value);
			else Var2 = &(Dst->Value);
		}

	}

	Src->Operator->Function(Src->Operator, Var1, Var2, &(Dst->Value));
}


bool TSyntaxParser::AddToStack(OPERATOR * Operator)
/**********************************************************************************************************************

		* ����� ���������� � ���������� ����� ���������� ���������� *

    ����������� ���� ���������� ����������.

  1. ������� ���������� (OP_PREFIX). ���� Rec.Value �����.
  2. ������� ����������� (OP_POSTFIX).
  3. �������� (OP_BINARY).
  4. ����������� (�������� � ������� ����������, NullOperator).
  5. ������� �������� ����������� � ���� ��� ������ ��������� ������������� ������ (0). ��� ��������� - 0.

	������������� ��������� �������� ������ � �������� ����������.

  1. ���� Rec.Value �� �������.
  2. ��������� ���� Rec.Operator ���������� ����������.
  3. ���� Rec.Priority (��������� �������� ������) ��������� ����������� ����������� ���������.
     ���� �������� �����������, �� ������������ ���������. ���� operator - ����������� ������, �� 0. 
  4. ������� ��������� ��������� ����������� ����������� ���������. ���� ������� ����������
     ����������������� ���������� ������, �� ��������� ������� ���������. ���� �������� ����������, ��
	 ������������ ���������. ���� Operator - ����������� ������, �� 0. 

	���� ���������� ����� ���������� ����������.

  1. ���� Rec - ����������� ������ (��� �������� ���������), � ������� - ����������� (��� ���� ����)
    (�� ���� ��� ���������� ����� 0), ��
	�) ��������� �� ������������.
	�) ���� �� OP_BRTRIVAR �� ��������� ������, ��������� �������� � �������, ��������� �������.
  2. ���� ��������� ������� ����� ������ ��������, ��
	�) ��������� (������� �����, Rec).
	�) ���������� �������. ���� ���� OP_BRTRIVAR, �� ���������� ��� ���� ������.
	�) ��������� � �.1.
  3. ��������� Rec � ����.

	���� ���� ����, �� ������� ��������� ��� ������� � ����� ������� ����.

**********************************************************************************************************************/
{
	// ���������� ������� ���������, ��������� ��������� ������ (Rec). /////////////////////////////////////////
#define MaxPrior 0x7FFFFFFF
	int Priority = 0;					
	Rec.Operator = Operator;			
	Rec.Priority = 0;
	int RecFlags = 0; // ����� ��������� � Rec
	if(Operator) // ������ ����������� � ������ ������ � ����������� �����
	{
		RecFlags = Rec.Operator->Flags;
		if(!(RecFlags & OP_BRCLOSE))
			Priority = Operator->Priority - (Operator->Flags & OP_FORWARD);
		if(RecFlags & OP_PREFIX) Priority = MaxPrior;
		if(!(RecFlags & OP_BROPEN))
			Rec.Priority = Operator->Priority;
		if(RecFlags & OP_POSTFIX) Rec.Priority = MaxPrior;
	}

	bool PopStack;
	do {
		int StackFlags = 0, StackPriority = 0; // �������� ���������� � ������� �����
		if(Stack >= StackBase)
		{
			if(Stack->Operator) StackFlags = Stack->Operator->Flags;
			StackPriority = Stack->Priority;
		}
		if(!(StackPriority + Priority)) // ��������� ������ ��� ������ � ����� ������
		{
			if((StackFlags | RecFlags) && !(StackFlags && RecFlags)) ADD_MESSAGE "Brackets error!" HALT  // ������ ��� ����!
			if((StackFlags ^ RecFlags) & OP_BRID)                    ADD_MESSAGE "Brackets error!" HALT // ����������� � ����������� �� �������������!
			// ������������ ���������.
			// 
			if(!(StackFlags | RecFlags)) return true; // ���� ���� � ������� �������� ���������� => ��������� ������
			if(!(OP_BRTRIVAR & StackFlags)) // ��������� ��������� ������
			{
				Compute(Stack--, &Rec);
				return true;
			}
		}

		PopStack = StackPriority > Priority;
		if(PopStack) // ��������� ����
		{
			if(OP_BRTRIVAR & StackFlags)
			{
				if(Stack - 1 < StackBase) ADD_MESSAGE "Tri-var brackets error!" HALT // ��� ���������� �������� OP_BRTRIVAR ��������� �� ����� ���� �������.
				Compute(Stack, &Rec);
				Stack -= 2;
			} else
			Compute(Stack--, &Rec);
		}
	} while(PopStack);
	*(++Stack) = Rec; // �������� � ���� ��������� ������
	return true;
}

OPERATOR * TSyntaxParser::GetOperator(char * * Name, int Flags) // ����� �������� �� ����� � ������
{
	OPERATOR * Op = Operators;
	for(int x = 0; x < OpCount; x++)
	{
		char * t;
		if((t = CompareOperators(*Name, Op->Sign)) && (Op->Flags & Flags))
		{
			*Name = t;
			return Op;
		}
		Op++;
	}
	return 0;
}

char * ReadIntegerValue(char * Text, VALUE * Result) // Text[0] - Letter, Text[1] - ��������
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


VARIABLE * TSyntaxParser::FindVariable(char * * Name) // ����� ���������� �� �����
{
	VARIABLE * Var = Variables - 1;
	for(int x = 0; x < VarCount; x++)
	if(char * tn = CompareLetters(*Name, (++Var)->Name))
	{
		*Name = tn;
		return Var;
	}
	return 0;
}

VARIABLE * TSyntaxParser::AddVariable(char * * Name, VALUE * Value) // �������� ����������
{
	VARIABLE * Var = Variables + VarCount++;
	Var->Name = *Name;
	if(Value)
	Var->Value = *Value;
	*Name += NameLen(*Name);
	return Var;
}


VALUE * TSyntaxParser::Parse(char * s)
/**********************************************************************************************************************

		* ����������� ������ *
		
  ������������� ������ s ������ ����������� ����.

    ����������� ���� ����������.

  1. ������� ���������� (OP_PREFIX)
  2. ������� ����������� (OP_POSTFIX)
  3. �������� (OP_BINARY)
  4. ����������� (�������� � ������� ����������, NullOperator) 
  5. ������� �������� ����������� � ���� ��� ������ ��������� ������������� ������ (0).

    �������� �� ��������� ���������� ��� ����������� ������� (������ ������ �� IsLetter).

  1. ���� ���� Rec.Value ����� �� ���� OP_PREFIX. �����:
	�) ���� �������� ������, �� ��������� ��� � ����.
	�) ����� ������.
  2. ���� ���� Rec.Value �� ����� �� ���� OP_POSTFIX � OP_BINARY. �����:
	�) ���� �������� ������, �� ��������� ��� � ����. ���� ������ OP_BINARY �� �������� Rec.Value. 
	�) ���� �������� �� ������, ���� OP_PREFIX, ���� �������, �� ��������� NullOperator, ������� Rec.Value, ��������� OP_PREFIX
	�) ���� �������� �������� �� ������ �� ������.

    �������� �� ��������� ��������/��� ��� ����������� ������� (������ ������ IsLetter)..

  1. ���� ���� Rec.Value �� �����, �� ��������� NullOperator.
  2. ��������� ���� Rec.Value �� ����������� ��������/�����.

  | Rec.Value |   ��������   | ������� Rec.Value? |
  |-----------|--------------|--------------------|
  |    ���    |   OP_PREFIX  |         -          |
  |    ����   | NullOperator |        ��          |
  |    ����   |   OP_BINARY  |        ��          |
  |    ����   |   OP_POSTFIX |        ���         |
 
**********************************************************************************************************************/
{
	Rec.Value.Type = 0;
	for( ; *s; s++)
	{
		if(IsLetter(*s)) // ������ ���������������
		{
			if(Rec.Value.Type) // �������� ��� ����
			{
				if(NullOperator) AddToStack(NullOperator);
				else ADD_MESSAGE "Empty operator not allowed!" HALT
			}

			if(IsValue(*s))
			{
				if(char * ts = ReadValue(s, &Rec.Value)) s = ts;
				else ADD_MESSAGE "Value '%n' is invalid!", s HALT
			}
			else
			if(VARIABLE * Var = FindVariable(&s))
			{
				Rec.Value.Value = Var;
				Rec.Value.Type = VL_VALISVAR;
			}
			else
			{
				Rec.Value.Value = s;
				Rec.Value.Type = VL_VALISNAME;
				s += NameLen(s);
			}
			s--;
			continue;
		}
		if(*s > ' ') // ������ ����������
		{
			OPERATOR * Op;
			if(Rec.Value.Type) Op = GetOperator(&s, OP_BINARY | OP_POSTFIX);
			else			   Op = GetOperator(&s, OP_PREFIX);

			if(Op) // �������� ������
			{
				AddToStack(Op);
				if(Op->Flags & OP_BINARY) Rec.Value.Type = 0;
				s--;
			} 
			else // �������� �� ������
			{
				if(Rec.Value.Type)
				if(Op = GetOperator(&s, OP_PREFIX)) // ������ ���������/������������ ������ ����������
				{
					if(!NullOperator) ADD_MESSAGE "Empty operator not allowed!" HALT
					AddToStack(NullOperator);
					Rec.Value.Type = 0;
					AddToStack(Op);
					continue;
				}
				ADD_MESSAGE "Operator '%o' not found!", s HALT;
			}
		}
	}

	if(!Rec.Value.Type) ADD_MESSAGE "Syntax error!" HALT;
	AddToStack(0);

	return &Rec.Value;
}


TSyntaxParser::TSyntaxParser(OPERATOR * Operators, int Count, TFValReader ValReader)
{
	this->Operators = Operators;
	OpCount = Count;
	Stack = StackBase - 1;
	VarCount = 0;
	NullOperator = 0;
	int MinPriority = Operators->Priority;
	for(int x = 0; x < Count; x++)
	{
		if(!Operators[x].Sign) NullOperator = Operators + x; // ����� �������� ��� ���������
		if(Operators[x].Priority < MinPriority) MinPriority = Operators[x].Priority; // ����� ����������� ���������
	}
	MinPriority = 2 - MinPriority; // ���������� ���������� ��������� - 2
	for(int x = 0; x < Count; x++)
	Operators[x].Priority += MinPriority;
	ReadValue = ValReader;
}
