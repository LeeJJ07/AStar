#pragma once

#include "resource.h"
#include "vector"
#include "string"

using namespace std;

enum State { WSTART, WEND, MOVE, FINISH };

typedef struct tile
{
	int f;
	int g, h;
}Tile;

class AStar
{
private:
	int startX = 75, startY = 50;
	int len = 50;
	int rowSize, colSize;
	POINT s, e;
	POINT player;
	Tile **map;
	vector<POINT> road;
	vector<POINT> blocks;
	State state = WSTART;

public:
	AStar()
	{
		player = { 0,0 };

		s = { -1, -1 };
		e = { -1, -1 };

		rowSize = colSize = 20;
		map = new Tile * [rowSize];
		for (int i = 0; i < rowSize; i++)
			map[i] = new Tile[colSize];
		/*fillBlock();
		for (pair<int, int> pos : blocks)
		{
			map[pos.second][pos.first].g = -1;
			map[pos.second][pos.first].h = -1;
			map[pos.second][pos.first].f = -1;
		}*/
	}
	State GetState() { return state; }
	void SetStateNext() { 
		switch (state)
		{
		case WSTART:
			state = WEND;
			break;
		case WEND:
			state = MOVE;
			break;
		case MOVE:
			state = FINISH;
			break;
		case FINISH:
			state = WSTART;
			break;
		}
	}
	POINT GetMapStartPos() { return { startX, startY }; }
	int GetTileLen() { return len; }
	void SetStart(int x, int y) { s = { x, y }; player = { x,y }; }
	void SetEnd(int x, int y) { e = { x, y }; }
	POINT GetStart() { return s; }
	POINT GetEnd() { return e; }
	int GetValueG(int x, int y) { return map[y][x].g; }

	void SetMap()
	{
		for (int i = 0; i < rowSize; i++)
		{
			for (int j = 0; j < colSize; j++)
			{
				if (map[i][j].g == -1) continue;
				map[i][j].g = 0;
				map[i][j].h = -1;
				map[i][j].f = map[i][j].g + map[i][j].h;
			}
		}
	}
	void AddBlock(int x, int y)
	{
		blocks.push_back({ x,y });
		map[y][x].g = -1;
	}
	void Draw(HDC hdc, RECT& rectView)
	{
		// 메모리 DC 및 버퍼 생성
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hMemBitmap = CreateCompatibleBitmap(hdc, rectView.right, rectView.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

		// 배경을 흰색으로 채우기
		HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		FillRect(hMemDC, &rectView, hBrush);

		// 블록 그리기
		HBRUSH blockBrush = CreateSolidBrush(RGB(255, 255, 255));
		HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
		HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
		HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC, blockBrush);

		HFONT currentFont = CreateFont(9, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 
			VARIABLE_PITCH | FF_ROMAN, _T("Arial"));
		HFONT oldFont = (HFONT)SelectObject(hMemDC, currentFont);

		POINT center;
		for (int i = 0; i < rowSize; i++)
		{
			for (int j = 0; j < colSize; j++)
			{
				center = { startX + j * len + len / 2, startY + i * len + len / 2 };
				Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
				
				TCHAR szString1[15], szString2[15];
				_stprintf_s(szString1, _countof(szString1), _T("%3d    %3d"), map[i][j].g, map[i][j].h);
				_stprintf_s(szString2, _countof(szString2), _T("  %d "), map[i][j].f);
				TextOut(hMemDC, center.x - 20, center.y - 20, szString1, wcslen(szString1));
				TextOut(hMemDC, center.x - 10, center.y + 8 , szString2, wcslen(szString2));
			}
		}
		SelectObject(hMemDC, oldBrush);
		SelectObject(hMemDC, oldFont);

		oldBrush = (HBRUSH)SelectObject(hMemDC, blackBrush);
		for (POINT block : blocks)
		{
			center = { startX + block.x * len + len / 2, startY + block.y * len + len / 2 };
			Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
		}
		SelectObject(hMemDC, oldBrush);

		if (s.x != -1)
		{
			oldBrush = (HBRUSH)SelectObject(hMemDC, redBrush);
			center = { startX + s.x * len + len / 2, startY + s.y * len + len / 2 };
			Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
			SelectObject(hMemDC, oldBrush);
		}
		if (e.x != -1)
		{
			oldBrush = (HBRUSH)SelectObject(hMemDC, blueBrush);
			center = { startX + e.x * len + len / 2, startY + e.y * len + len / 2 };
			Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
			SelectObject(hMemDC, oldBrush);
		}
		BitBlt(hdc, 0, 0, rectView.right, rectView.bottom, hMemDC, 0, 0, SRCCOPY);
		
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(blockBrush);
		DeleteObject(blackBrush);
		DeleteObject(redBrush);
		DeleteObject(blueBrush);
		DeleteObject(hMemBitmap);
		DeleteObject(oldFont);
		DeleteDC(hMemDC);
	}
	void AStarMapSet()
	{
		
		
	}
	void Update()
	{
		if (player.x == e.x && player.y == e.y)
		{
			SetStateNext();
			return;
		}
	}
};
