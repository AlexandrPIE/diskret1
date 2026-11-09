#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
Инверсия - !
Дизъюнкция - +
Конъюнкция - *
Сложение по модулю 2 - 2
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

void sdof(int n, int vars[], int answers[], int type, char *group[])
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
				char znU = type ? '+' : '*';
				strcat(*group, znU);
			}
			firstGroup = 0;
			printf("(");
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
						char znI = type ? '*' : '+';
						printf("%c", znI);
					}
					values[i] == !type ? printf("!%c", 'a' + i) : printf("%c", 'a' + i);
				}
			}
			printf(")");
		}
	}
	if (firstGroup)
	{
		char *func = type ? "СДНФ" : "СКНФ";
		printf("Невозможно построить %s из-за того, что в столбце с результатом функции нет значения %d", func, type);
	}
	printf("\n");
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
		// В generateTable первая переменная соответствует старшему биту.
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
	int len = 0;
	char stack[1024];
	int top = 0;

	for (int i = 0; f[i] != '\0'; i++)
	{
		char c = f[i];
		if (c == ' ')
		{
			continue;
		}
		else if (c >= 'a' && c <= 'z')
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
			if (top > 0)
			{
				if (priority(c) > priority(stack[i - 1]))
				{
					stack[top] = c;
					top++;
				}
				else
				{
					result[len] = stack[top - 1];
					stack[top - 1] = c;
					len++;
				}
			}
			else
			{
				stack[0] = c;
				top++;
			}
		}
	}

	while (top > 0)
	{
		top--;
		result[len] = stack[top];
		len++;
	}

	result[len] = '\0';

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

		for (int i = 0; i < len; i++)
		{
			char l = result[i];
			if (l >= 'a' && l <= 'z')
			{
				numbers[size] = values[l - 'a'];
				size++;
			}
			else if (l == '1' || l == '0')
			{
				numbers[size] = (int)l;
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

	char group[1024];

	// СДНФ
	printf("СДНФ: ");
	sdof(n, vars, answers, 1, &group);
	// СКНФ
	printf("СКНФ: ");
	sdof(n, vars, answers, 0, &group);
	// Фиктивные переменные
	fictiv(n, vars, answers);

	// Задание 7
	printf("Для базисов {и, или, не}: ");
	sdof(n, vars, answers, 1, &group);
}
