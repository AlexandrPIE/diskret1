#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
Инверсия - !
Дизъюнкция - +
Конъюнкция - *
Сложение по модулю 2 - $
Эквивалентность - =
Импликация - >
Коимпликация - <
Стрелка Пирса - ^
Штрих Шеффера - #
*/

// Расставление приоритетов операций

int priority(char z)
{
	switch (z)
	{
	case '!':
		return 5;
	case '*':
		return 4;
	case '#':
		return 4;
	case '+':
		return 3;
	case '^':
		return 3;
	case '2':
		return 3;
	case '>':
		return 2;
	case '<':
		return 2;
	case '=':
		return 1;
	default:
		return 0;
	}
}

// Постфиксная запись

void postfix(char f[], char result[])
{
	int len = 0;
	char stack[1024];
	int top = 0;

	for (int i = 0; f[i] != '\0'; i++)
	{
		char c = f[i];
		if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
		{
			continue;
		}
		else if ((c >= 'a' && c <= 'z') || c == '0' || c == '1')
		{
			result[len] = c;
			len++;
		}
		else if (c == '(')
		{
			stack[top] = c;
			top++;
		}
		else if (c == ')')
		{
			while (top > 0 && stack[top - 1] != '(')
			{
				top--;
				result[len] = stack[top];
				len++;
			}
			if (top > 0)
			{
				top--;
			}
		}
		else
		{
			while (c != '!' && top > 0 && stack[top - 1] != '(' &&
					 priority(stack[top - 1]) >= priority(c))
			{
				top--;
				result[len] = stack[top];
				len++;
			}
			stack[top] = c;
			top++;
		}
	}

	while (top > 0)
	{
		top--;
		result[len] = stack[top];
		len++;
	}

	result[len] = '\0';
}

// Создание таблицы 2**n

void generateTable(int n, const int vars[], int row, int values[])
{
	int k = n - 1;
	for (int i = 0; i < 26; i++)
	{
		if (vars[i] == 1)
		{
			values[i] = (row >> k) & 1;
			k--;
		}
	}
}

// СДНФ / СКНФ

void sdof(int n, int vars[], int answers[], int type, char group[])
{
	int firstGroup = 1;
	for (int row = 0; row < (1 << n); row++)
	{
		int values[26] = {0};
		generateTable(n, vars, row, values);
		if (answers[row] == type)
		{
			if (!firstGroup)
			{
				char *znU = type ? "+" : "*";
				strcat(group, znU);
			}
			firstGroup = 0;
			strcat(group, "(");
			int first = 1;
			for (int i = 0; i < 26; i++)
			{
				if (vars[i] == 1)
				{
					if (first)
					{
						first = 0;
					}
					else
					{
						char *znI = type ? "*" : "+";
						strcat(group, znI);
					}
					char a[3] = {'!', 'a' + i, '\0'};
					char b[2] = {'a' + i, '\0'};
					values[i] == !type ? strcat(group, a) : strcat(group, b);
				}
			}
			strcat(group, ")");
		}
	}
	if (firstGroup)
	{
		char *func = type ? "СДНФ" : "СКНФ";
		printf("Невозможно построить %s из-за того, что в столбце с результатом функции нет значения %d", func, type);
	}
}

// Поиск фиктивных переменных

void fictiv(int n, const int vars[], int answers[])
{
	int col = 0;
	int found = 0;
	printf("Фиктивные переменные: ");
	for (int i = 0; i < 26; i++)
	{
		if (vars[i] != 1)
		{
			continue;
		}
		int mask = 1 << (n - 1 - col);
		int isFictive = 1;
		for (int row = 0; row < (1 << n); row++)
		{
			// Сравниваем наборы, отличающиеся только этой переменной.
			if ((row & mask) == 0 && answers[row] != answers[row | mask])
			{
				isFictive = 0;
				break;
			}
		}
		if (isFictive)
		{
			printf("%s%c", found ? ", " : "", 'a' + i);
			found = 1;
		}
		col++;
	}
	if (!found)
	{
		printf("Нет");
	}
	printf("\n");
}

// Задание 7

void reduce(char pos[][8192], char neg[][8192], int count, char op)
{
	char buffer[8192];
	while (count > 1)
	{
		int k = 0;
		for (int i = 0; i < count; i += 2)
		{
			if (i + 1 < count)
			{
				sprintf(buffer, "(%s %c %s)", pos[i], op, pos[i + 1]);
				strcpy(neg[k], buffer);
				sprintf(pos[k], "(%s %c %s)", buffer, op, buffer);
			}
			else
			{
				strcpy(pos[k], pos[i]);
				strcpy(neg[k], neg[i]);
			}
			k++;
		}
		count = k;
	}
}

// Для Шеффера - СДНФ
// Для Пирса - СКНФ

void basis(int n, int vars[], int answers[], char op, char result[])
{
	char groupPos[64][8192];
	char groupNeg[64][8192];
	char literalPos[26][8192];
	char literalNeg[26][8192];
	int type = (op == '#') ? 1 : 0;
	int m = 0; // построенные группы

	for (int row = 0; row < (1 << n); row++)
	{
		if (answers[row] != type)
		{
			continue;
		}
		int values[26] = {0};
		generateTable(n, vars, row, values);
		int k = 0; // счетчик литералов
		for (int i = 0; i < 26; i++)
		{
			if (vars[i] == 1)
			{
				char x = 'a' + i;
				char notX[16];
				sprintf(notX, "(%c %c %c)", x, op, x);
				if (values[i] == type)
				{
					sprintf(literalPos[k], "%c", x);
					strcpy(literalNeg[k], notX);
				}
				else
				{
					strcpy(literalPos[k], notX);
					sprintf(literalNeg[k], "%c", x);
				}
				k++;
			}
		}
		reduce(literalPos, literalNeg, k, op);

		strcpy(groupPos[m], literalNeg[0]);
		strcpy(groupNeg[m], literalPos[0]);
		m++;
	}

	// Функция-константа: нет ни одного нужного набора
	if (m == 0)
	{
		char x = 'a';
		for (int i = 0; i < 26; i++)
		{
			if (vars[i] == 1)
			{
				x = 'a' + i;
				break;
			}
		}
		char c[64];
		sprintf(c, "(%c %c (%c %c %c))", x, op, x, op, x);
		sprintf(result, "(%s %c %s)", c, op, c);
	}
	reduce(groupPos, groupNeg, m, op);
	strcpy(result, groupNeg[0]);
}

// Полином Жегалкина

/* Замены:
1) !a = 1 XOR a
2) a XOR b XOR a = b
3) a * (b XOR c) = a*b XOR a*c
*/

void zhegalkin(int n, int vars[], int answers[], char result[])
{
	int c[1 << n];
	for (int row = 0; row < (1 << n); row++)
	{
		c[row] = answers[row];
	}

	for (int b = 0; b < n; b++)
	{
		for (int row = 0; row < (1 << n); row++)
		{
			if ((row >> b) & 1)
			{
				c[row] = c[row] ^ c[row - (1 << b)];
			}
		}
	}

	int first = 1;
	for (int row = 0; row < (1 << n); row++)
	{
		if (c[row] == 0)
		{
			continue;
		}
		if (!first)
		{
			strcat(result, " $ ");
		}
		first = 0;
		if (row == 0)
		{
			strcat(result, "1");
			continue;
		}
		int values[26] = {0};
		generateTable(n, vars, row, values);
		int firstVar = 1;
		for (int i = 0; i < 26; i++)
		{
			if (vars[i] == 1 && values[i] == 1)
			{
				if (!firstVar)
				{
					strcat(result, "*");
				}
				firstVar = 0;
				char b[2] = {'a' + i, '\0'};
				strcat(result, b);
			}
		}
	}
	if (first)
	{
		strcat(result, "0");
	}
}

int main(int argc, char *argv[])
{

	// Чтение формулы

	FILE *file = fopen(argv[1], "r");
	if (!argv[1])
	{
		printf("Не передан путь файла\n");
		return 1;
	}

	char f[1024];
	fgets(f, 1024, file);
	fclose(file);

	// Подсчет переменных

	int vars[26] = {0};

	for (int i = 0; f[i] != '\0'; i++)
	{
		f[i] = tolower(f[i]);
		char c = f[i];
		if (c >= 'a' && c <= 'z')
		{
			vars[c - 'a'] = 1;
		}
	}

	int n = 0;

	for (int i = 0; i < 26; i++)
	{
		if (vars[i] == 1)
		{
			n++;
		}
	}

	// Постфиксная запись

	char result[1024];
	postfix(f, result);

	// Заполнение переменных

	// Шапка

	for (int i = 0; i < 26; i++)
	{
		if (vars[i] == 1)
		{
			printf("%c ", 'A' + i);
		}
	}
	printf("| F\n");

	int answers[1 << n];

	for (int row = 0; row < (1 << n); row++)
	{
		int values[26] = {0};
		generateTable(n, vars, row, values);
		int numbers[1024];
		int size = 0;

		for (int i = 0; i < strlen(result); i++)
		{
			char l = result[i];
			if (l >= 'a' && l <= 'z')
			{
				numbers[size] = values[l - 'a'];
				size++;
			}
			else if (l == '1' || l == '0')
			{
				numbers[size] = l - '0';
				size++;
			}
			else if (l == '!')
			{
				numbers[size - 1] = !numbers[size - 1];
			}
			else
			{
				size--;
				int rd = numbers[size];
				size--;
				int ld = numbers[size];
				switch (l)
				{
				case '+':
					numbers[size] = rd || ld;
					break;
				case '*':
					numbers[size] = rd && ld;
					break;
				case '2':
					numbers[size] = rd != ld;
					break;
				case '=':
					numbers[size] = rd == ld;
					break;
				case '>':
					numbers[size] = !ld || rd;
					break;
				case '<':
					numbers[size] = ld && !rd;
					break;
				case '^':
					numbers[size] = !(ld || rd);
					break;
				case '#':
					numbers[size] = !(ld && rd);
					break;
				}
				size++;
			}
		}
		for (int i = 0; i < 26; i++)
		{
			if (vars[i] == 1)
			{
				printf("%d ", values[i]);
			}
		}
		printf("| %d\n", numbers[0]);
		answers[row] = numbers[0];
	}

	char sdnf[65536] = "";
	char sknf[65536] = "";

	// СДНФ
	sdof(n, vars, answers, 1, sdnf);
	printf("СДНФ: %s\n", sdnf);
	// СКНФ
	sdof(n, vars, answers, 0, sknf);
	printf("СКНФ: %s\n", sknf);
	// Фиктивные переменные
	fictiv(n, vars, answers);

	// Задание 7

	printf("Базис {*, +, !}: %s\n", sdnf[0] ? sdnf : sknf);

	char spirs[65536] = "";
	basis(n, vars, answers, '^', spirs);
	printf("Базис {^}: %s\n", spirs);

	char shef[65536] = "";
	basis(n, vars, answers, '#', shef);
	printf("Базис {#}: %s\n", shef);

	char zheg[4096] = "";
	zhegalkin(n, vars, answers, zheg);
	printf("Полином Жегалкина: %s\n", zheg);
}
