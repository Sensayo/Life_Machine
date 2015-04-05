#ifndef _LIFE_MACHINE_H_
#define _LIFE_MACHINE_H_

////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <time.h>

#define Sell		field[i][j]
#define Right		field[i][j + 1]
#define RightDown	field[i + 1][j + 1]
#define Down		field[i + 1][j]
#define LeftDown	field[i + 1][j - 1]
#define Left		field[i][j - 1]
#define LeftTop		field[i - 1][j - 1]
#define Top			field[i - 1][j]
#define RightTop	field[i - 1][j + 1]

//		LT	T	RT
//		L	S	R
//		LD	D	RD

////////////////////////////////////////////////////////////////////////////////////////////////

struct sell
{
	bool value;	// true - мина, false - пусто
	int count;
	bool query;	// True -> на ячейке стоит вопрос-> она была изменена на этом ходе
};

std::vector< std::vector<sell> > field;	//поле 
std::vector< std::vector<sell> > TempField; // дополнительное поле
int height;								    // Высота
int width;								    // Ширина


////////////////////////////////////////////////////////////////////////////////////////////////

void newSize(int _height = 10, int _width = 10);	// Изменяет размеры поля и очищает его
void clearField();	// Обнуляет поле. value = 0; query = false
int aroundPlusPlus(int line, int column); /// определяет количество вирусов вокруг точки
int fill(int line, int column);
bool check(int line, int column);	// проверяет условие жизни и смерти
int route();
////////////////////////////////////////////////////////////////////////////////////////////////

void newSize(int _height, int _width)
{
	height = _height; width = _width;
	clearField();
}

void clearField()
{
	field.clear();

	field.resize(height);
	for (int i = 0; i < height; i++)
		field[i].resize(width);

	TempField.resize(height);
	for (int i = 0; i < height; i++)
		TempField[i].resize(width);
}

int aroundPlusPlus(int line, int column)
{
	int i = line, j = column;
	if (field[line][column].value == true)

	{
		if (LeftTop.value == true) field[line][column].count++;
		if (Left.value == true) field[line][column].count++;
		if (LeftDown.value == true) field[line][column].count++;
		if (Top.value == true) field[line][column].count++;
		if (RightTop.value == true) field[line][column].count++;
		if (Right.value == true) field[line][column].count++;
		if (RightDown.value == true) field[line][column].count++;
		if (Down.value == true) field[line][column].count++;
	}

	return  field[line][column].count++;
}

bool check(int line, int column)
{
	if (field[line][column].count < 2 || field[line][column].count >3) /// если бактерий вокруг меньше 2х и бельше 3х - мы дохнем
	{
		return false;
	}
	else
	{
		if ((field[line][column].value == false) && (field[line][column].count != 3)) // если бактерий у пустой клетки не 3 - мы пустые
		{
			return  false;
		}
		else /// во всех дрыгих случая мы заполняемся
		{
			return true;
		}
	}
}

int route()
{
	int i, j;
	int sum = 0;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			TempField[i][j].value = check(i, j);
		}
	}

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)

		{
			TempField[i][j].count = aroundPlusPlus(i, j);
			field[i][j].value = TempField[i][j].value;
			field[i][j].count = TempField[i][j].count;
			sum += field[i][j].count;
		}
	}

	if (sum == 0) return 0; // если ни у одной точки нет окружения - мы проиграли

	return 1; // если окружение есть - мы продолжаем
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int aroundCount(int line, int column)
{
	int i = line, j = column;
	int Sum = 0;
	if (line == 0)
	{
		if (column == 0)
		{
			if (Right.value == true) Sum++;
			if (RightDown.value == true) Sum++;
			if (Down.value == true) Sum++;
		}
		else if (column == width - 1)
		{
			if (Down.value == true) Sum++;
			if (LeftDown.value == true) Sum++;
			if (Left.value == true) Sum++;
		}
		else
		{
			if (Right.value == true) Sum++;
			if (RightDown.value == true) Sum++;
			if (Down.value == true) Sum++;
			if (LeftDown.value == true) Sum++;
			if (Left.value == true) Sum++;
		}
	}
	else if (line == height - 1)
	{
		if (column == 0)
		{
			if (Top.value == true) Sum++;
			if (RightTop.value == true) Sum++;
			if (Right.value == true) Sum++;
		}
		else if (column == width - 1)
		{
			if (Left.value == true) Sum++;
			if (LeftTop.value == true) Sum++;
			if (Top.value == true) Sum++;
		}
		else
		{
			if (Left.value == true) Sum++;
			if (LeftTop.value == true) Sum++;
			if (Top.value == true) Sum++;
			if (RightTop.value == true) Sum++;
			if (Right.value == true) Sum++;
		}
	}
	else if (column == 0) //Тут только если line != 0 && line != height - 1
	{
		if (Top.value == true) Sum++;
		if (RightTop.value == true) Sum++;
		if (Right.value == true) Sum++;
		if (RightDown.value == true) Sum++;
		if (Down.value == true) Sum++;
	}
	else if (column == width - 1) //Тут только если line != 0 && line != height - 1
	{
		if (Down.value == true) Sum++;
		if (LeftDown.value == true) Sum++;
		if (Left.value == true) Sum++;
		if (LeftTop.value == true) Sum++;
		if (Top.value == true) Sum++;
	}
	else //Общий случай
	{
		if (Top.value == true) Sum++;
		if (RightTop.value == true) Sum++;
		if (Right.value == true) Sum++;
		if (RightDown.value == true) Sum++;
		if (Down.value == true) Sum++;
		if (LeftDown.value == true) Sum++;
		if (Left.value == true) Sum++;
		if (LeftTop.value == true) Sum++;
	}
	return Sum;
}

int nextTime()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			field[i][j].count = aroundCount(i, j);
		}
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (field[i][j].value && (field[i][j].count > 3 || field[i][j].count < 2))
				field[i][j].value = false;
			else if (field[i][j].count == 3)
				field[i][j].value = true;
		}
	}

	return 1;
}
#endif //_LIFE_MACHINE_H_