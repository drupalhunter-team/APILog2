#include "stdafx.h"
#include "PTemplates.h"
#include "CParser.h"
#include "CErrors.h"
#include <stdio.h>
#include <windows.h>

#define IsLetter(c) (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_'))
#define IsDigit(c) ((c >= '0') && (c <= '9'))
#define IsValue(c) (((c >= '0') && (c <= '9')) || (c == '"') || (c == 39))

// Функция вычисления длины идентификатора
int NameLen(char * a)
{
	int l;
	for(l = 0; IsLetter(*a); a++) l++;
	return l;
}

// Функция сравнения оператора со строкой. Если сравнение успешно, то возвращается указатель на первый символ после оператора
char * CompareOperators(char * a, char * b)
{
	while(*b)
	if(*(a++) != *(b++)) return false;
	return (*b == 0) ? a : 0;
}

// Функция сравнения имен. Если сравнение успешно, то возвращается указатель на первый символ после имени a.
// Окончанием имён считается любой символ не IsLetter()
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

		* Выполнение операторов *

	Входные данные и операнды функций операторов:

 | Тип оператора |           Выполняемое выражение                      |    Var1    |    Var2    |  Result до  | Result после |
 |---------------|------------------------------------------------------|------------|------------|-------------|--------------|
 |   OP_BINARY   | Dst->Value = Src->Value x Dst->Value                 | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |  NullOperator | Dst->Value = Src->Value Dst->Value                   | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |   OP_PREFIX   | Dst->Value = x Dst->Value                            |      0     | Dst->Value |      0      |  Dst->Value  |
 |   OP_POSTFIX  | Dst->Value = Src->Value x                            | Src->Value |      0     |      0      |  Dst->Value  |
 |  OP_BRACKETS  | Dst->Value = Src->Value ( Dst->Value )               | Src->Value | Dst->Value |      0      |  Dst->Value  |
 |  OP_BRTRIVAR  | Dst->Value = Src[-1].Value ? Src->Value : Dst->Value | Src->Value | Dst->Value |Src[-1].Value|  Dst->Value  |
	
		где 'x', '()', '?:' - выполняемый оператор из Src->Operator.

	Функция оператора должна находиться в поле Src->Operator->Function. Для двухоперандных скобок выполняется функция
	их открывающей скобки, для трёх операндных - закрывающей. Допускается использовать разные операторы с разными 
	функциями для одной и той же открывающей скобки: OP_PREFIX (например: скобки алгебраического выражения,
	явное преобразование типа) и OP_BINARY (например: вызов функции, явное преобразование типа, обращение к
	элементу массива).

    Требования оператора к операндам.

 | Флаг оператора | Описание                               |
 |----------------|----------------------------------------|
 |    OP_INITx	  | Параметр x должен быть инициализирован.|
 |    OP_LEFTx	  | Параметр x должен быть леводопустимым. |
 |    OP_NAMEx	  | Параметр x должен иметь имя.           |

    Признаки результата оператора.

 | Флаг оператора | Описание                   |
 |----------------|----------------------------|
 |    OP_INITR	  | Результат инициализирован. |
 |    OP_LEFTR	  | Результат леводопустим.    |

	Результат не может иметь флаги VL_VALISVAR и VL_VALISNAME.

    Базовые признаки значения:
  1. Переменная: VL_VALISVAR.
  2. Значение инициализированно: VL_INITED. Если VL_VALISVAR, то хранится в флагах переменной.
  3. [Неизвестное] имя: VL_VALISNAME.
  4. Леводопустимость VL_LEFT.

  Леводопустимость результата может происходить от:
  1. Оператора. Пример: оператор разыменования указателя *.
  //2. Может сохраняться леводопустимость операнда.



typedef bool (* TFOperator) (VALUE * Var1, VALUE * Var2, VALUE * Result);	// Указатель на функцию выполняющую оператор

  Алгоритм получения значений операндов (x: A, B)

	Если OP_NAMEx то
		Если VL_VALISNAME то VALUE.Value
		Если VL_VALISVAR то ((VARIABLE *)VALUE.Value)->Name
		Иначе ошибка: имя отсутствует.
	Иначе проверить леводопустимость. Далее
		Если VL_VALISVAR то ((VARIABLE *)VALUE.Value)->Value
		Иначе VALUE
		Проверить, инициализирован ли результат.

  Если используется Result как операнд, то перед запуском, результат алгоритма копируется в Dst->Value

**************************************************************************************************************/
{
	if(!Src->Operator->Function) return; // Нечего выполнять.
	void * Var1 = 0, * Var2 = 0;
	int Flags = Src->Operator->Flags;
	if(Src->Value.Type)
	{
		if(Flags & OP_NAMEA)
		{
			if(Src->Value.Type & VL_VALISNAME) Var1 = Src->Value.Value;
			else if(Src->Value.Type & VL_VALISVAR) Var1 = ((VARIABLE *)Src->Value.Value)->Name;
			else {
				// Имя отсутствует!!!
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
				// Имя отсутствует!!!
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

		* Метод пополнения и разрешения стека отложенных операторов *

    Лексические типы получаемых операторов.

  1. Унарный префиксный (OP_PREFIX). Поле Rec.Value пусто.
  2. Унарный постфиксный (OP_POSTFIX).
  3. Бинарный (OP_BINARY).
  4. Разделитель (оператор с нулевой мнемоникой, NullOperator).
  5. Нулевой оператор добавляется в стек как сигнал окончания анализируемой строки (0). Его приоритет - 0.

	Инициализация временной стековой записи и текущего приоритета.

  1. Поле Rec.Value не трогаем.
  2. Заполняем поле Rec.Operator полученным оператором.
  3. Поле Rec.Priority (приоритет стековой записи) заполняем приоритетом полученного оператора.
     Если оператор постфиксный, то максимальный приоритет. Если operator - открывающая скобка, то 0. 
  4. Текущий приоритет заполняем приоритетом полученного оператора. Если порядок выполнения
     равноприоритетных операторов прямой, то уменьшаем текущий приоритет. Если оператор префиксный, то
	 максимальный приоритет. Если Operator - закрывающая скобка, то 0. 

	Цикл разрешения стека отложенных операторов.

  1. Если Rec - закрывающая скобка (или оператор окончания), а вершина - открывающая (или стек пуст)
    (то есть оба приоритета равны 0), то
	а) Проверить их соответствие.
	б) Если не OP_BRTRIVAR то выполняем скобку, сохраняем значение в вершину, завершаем функцию.
  2. Если приоритет вершины стека больше текущего, то
	а) вычисляем (вершина стека, Rec).
	б) сбрасываем вершину. Если было OP_BRTRIVAR, то сбрасываем ещё одну запись.
	в) Вернуться к п.1.
  3. Добавляем Rec в стек.

	Если стек пуст, то считаем приоритет его вершины и флаги равными нулю.

**********************************************************************************************************************/
{
	// Рассчитать текущий приоритет, заполнить временную запись (Rec). /////////////////////////////////////////
#define MaxPrior 0x7FFFFFFF
	int Priority = 0;					
	Rec.Operator = Operator;			
	Rec.Priority = 0;
	int RecFlags = 0; // Флаги оператора в Rec
	if(Operator) // Расчёт приоритетов с учётом скобок и лексических типов
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
		int StackFlags = 0, StackPriority = 0; // Получить информацию о вершине стека
		if(Stack >= StackBase)
		{
			if(Stack->Operator) StackFlags = Stack->Operator->Flags;
			StackPriority = Stack->Priority;
		}
		if(!(StackPriority + Priority)) // Замыкание скобок или начала и конца строки
		{
			if((StackFlags | RecFlags) && !(StackFlags && RecFlags)) ADD_MESSAGE "Brackets error!" HALT  // Скобка без пары!
			if((StackFlags ^ RecFlags) & OP_BRID)                    ADD_MESSAGE "Brackets error!" HALT // Открывающая и закрывающая не соответствуют!
			// Соответствие проверено.
			// 
			if(!(StackFlags | RecFlags)) return true; // Стек пуст и получен оператор завершения => окончание работы
			if(!(OP_BRTRIVAR & StackFlags)) // Обработка одинарных скобок
			{
				Compute(Stack--, &Rec);
				return true;
			}
		}

		PopStack = StackPriority > Priority;
		if(PopStack) // Разрешить стек
		{
			if(OP_BRTRIVAR & StackFlags)
			{
				if(Stack - 1 < StackBase) ADD_MESSAGE "Tri-var brackets error!" HALT // Для выполнения операции OP_BRTRIVAR требуется не менее двух записей.
				Compute(Stack, &Rec);
				Stack -= 2;
			} else
			Compute(Stack--, &Rec);
		}
	} while(PopStack);
	*(++Stack) = Rec; // Добавить в стек временную запись
	return true;
}

OPERATOR * TSyntaxParser::GetOperator(char * * Name, int Flags) // Найти оператор по имени и флагам
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

char * ReadIntegerValue(char * Text, VALUE * Result) // Text[0] - Letter, Text[1] - доступен
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


VARIABLE * TSyntaxParser::FindVariable(char * * Name) // Найти переменную по имени
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

VARIABLE * TSyntaxParser::AddVariable(char * * Name, VALUE * Value) // Добавить переменную
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

		* Лексический анализ *
		
  Анализируемая строка s должна завершаться нулём.

    Лексические типы операторов.

  1. Унарный префиксный (OP_PREFIX)
  2. Унарный постфиксный (OP_POSTFIX)
  3. Бинарный (OP_BINARY)
  4. Разделитель (оператор с нулевой мнемоникой, NullOperator) 
  5. Нулевой оператор добавляется в стек как сигнал окончания анализируемой строки (0).

    Действия по обработке операторов при лексическом анализе (найден символ не IsLetter).

  1. Если поле Rec.Value пусто то ищем OP_PREFIX. Далее:
	а) Если оператор найден, то добавляем его в стек.
	б) Иначе ошибка.
  2. Если поле Rec.Value не пусто то ищем OP_POSTFIX и OP_BINARY. Далее:
	а) Если оператор найден, то добавляем его в стек. Если найден OP_BINARY то очистить Rec.Value. 
	б) Если оператор не найден, ищем OP_PREFIX, если успешно, то добавляем NullOperator, очищаем Rec.Value, добавляем OP_PREFIX
	в) Если оператор повторно не найден то ошибка.

    Действия по обработке значений/имён при лексическом анализе (найден символ IsLetter)..

  1. Если поле Rec.Value не пусто, то добавляем NullOperator.
  2. Заполняем поле Rec.Value из полученного значения/имени.

  | Rec.Value |   Оператор   | Очищать Rec.Value? |
  |-----------|--------------|--------------------|
  |    Нет    |   OP_PREFIX  |         -          |
  |    Есть   | NullOperator |        Да          |
  |    Есть   |   OP_BINARY  |        Да          |
  |    Есть   |   OP_POSTFIX |        Нет         |
 
**********************************************************************************************************************/
{
	Rec.Value.Type = 0;
	for( ; *s; s++)
	{
		if(IsLetter(*s)) // Разбор идентификаторов
		{
			if(Rec.Value.Type) // Значение уже есть
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
		if(*s > ' ') // Разбор операторов
		{
			OPERATOR * Op;
			if(Rec.Value.Type) Op = GetOperator(&s, OP_BINARY | OP_POSTFIX);
			else			   Op = GetOperator(&s, OP_PREFIX);

			if(Op) // Оператор найден
			{
				AddToStack(Op);
				if(Op->Flags & OP_BINARY) Rec.Value.Type = 0;
				s--;
			} 
			else // Оператор не найден
			{
				if(Rec.Value.Type)
				if(Op = GetOperator(&s, OP_PREFIX)) // Вместо бинарного/постфиксного найден префиксный
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
		if(!Operators[x].Sign) NullOperator = Operators + x; // Найти оператор без мнемоники
		if(Operators[x].Priority < MinPriority) MinPriority = Operators[x].Priority; // Найти минимальный приоритет
	}
	MinPriority = 2 - MinPriority; // Минимально допустимый приоритет - 2
	for(int x = 0; x < Count; x++)
	Operators[x].Priority += MinPriority;
	ReadValue = ValReader;
}
