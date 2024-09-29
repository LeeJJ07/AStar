#pragma once

#include "resource.h"
#include "vector"
#include "string"
#include "queue"

using namespace std;

enum State { WSTART, WEND, MOVE, FINISH };
enum tileState { BLANK, OPEN, START, END, WALL };
// 짝수 : 10, 홀수 : 14
int dy[] = { -1, -1, 0, 1, 1, 1, 0, -1 };
int dx[] = { 0, 1, 1, 1, 0, -1, -1, -1 };

typedef struct tile
{
	int f;
	int g, h;
	POINT prev;
	tileState tState;
}Tile;

class AStar
{
private:
	int startX = 75, startY = 50;
	int len = 80;
	int rowSize, colSize;
	POINT s, e;
	POINT player;
	Tile** map;
	vector<POINT> road;
	vector<POINT> playerMove;
	vector<POINT> blocks;
	State state = WSTART;

public:
	AStar()
	{
		player = { 0,0 };

		s = { -1, -1 };
		e = { -1, -1 };

		rowSize = 12;
		colSize = 15;
		map = new Tile * [rowSize];
		for (int i = 0; i < rowSize; i++)
			map[i] = new Tile[colSize];
	}
	State GetState() { return state; }
	void SetStateNext()
	{
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
	void SetStart(int x, int y) { s = { x, y }; player = { x, y }; map[y][x].tState = START; }
	void SetEnd(int x, int y) { e = { x, y }; map[y][x].tState = END; }
	POINT GetStart() { return s; }
	POINT GetEnd() { return e; }
	int GetValueG(int x, int y) { return map[y][x].g; }

	void SetMap()
	{
		road.clear();
		blocks.clear();
		s.x = s.y = -1;
		e.x = e.y = -1;
		for (int i = 0; i < rowSize; i++)
		{
			for (int j = 0; j < colSize; j++)
			{
				map[i][j].g = 0;
				map[i][j].h = -1;
				map[i][j].f = map[i][j].g + map[i][j].h;
				map[i][j].tState = BLANK;
				map[i][j].prev = { -1, -1 };
			}
		}
	}
	void AddBlock(int x, int y)
	{
		blocks.push_back({ x,y });
		map[y][x].g = -1;
		map[y][x].tState = WALL;
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
		HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
		HBRUSH orangeBrush = CreateSolidBrush(RGB(255, 165, 0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC, blockBrush);

		HFONT currentFont = CreateFont(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
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
				//_stprintf_s(szString1, _countof(szString1), _T("%3d  %3d"), map[i][j].prev.x, map[i][j].prev.y);
				_stprintf_s(szString1, _countof(szString1), _T("%3d  %3d"), map[i][j].g, map[i][j].h);
				_stprintf_s(szString2, _countof(szString2), _T("  %d "), map[i][j].f);
				TextOut(hMemDC, center.x - 20, center.y - 20, szString1, wcslen(szString1));
				TextOut(hMemDC, center.x - 10, center.y + 8, szString2, wcslen(szString2));
			}
		}
		SelectObject(hMemDC, oldBrush);
		SelectObject(hMemDC, oldFont);

		oldBrush = (HBRUSH)SelectObject(hMemDC, orangeBrush);
		for (POINT r : road)
		{
			center = { startX + r.x * len + len / 2, startY + r.y * len + len / 2 };
			Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
		}
		SelectObject(hMemDC, oldBrush);

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

		if (state == MOVE || state == FINISH)
		{
			oldBrush = (HBRUSH)SelectObject(hMemDC, greenBrush);
			center = { startX + player.x * len + len / 2, startY + player.y * len + len / 2 };
			Rectangle(hMemDC, center.x - len / 2, center.y - len / 2, center.x + len / 2, center.y + len / 2);
			SelectObject(hMemDC, oldBrush);
		}

		BitBlt(hdc, 0, 0, rectView.right, rectView.bottom, hMemDC, 0, 0, SRCCOPY);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(blockBrush);
		DeleteObject(blackBrush);
		DeleteObject(redBrush);
		DeleteObject(blueBrush);
		DeleteObject(greenBrush);
		DeleteObject(orangeBrush);
		DeleteObject(hMemBitmap);
		DeleteObject(oldFont);
		DeleteDC(hMemDC);
	}
	void AStarMapSet()
	{
		//맵 세팅 및 경로 저장

		map[s.y][s.x].g = 0;
		map[s.y][s.x].h = calculateH(s.x, s.y);
		map[s.y][s.x].f = map[s.y][s.x].g + map[s.y][s.x].h;

		priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<pair<int, pair<int, int>>>> q;
		q.push({ map[s.y][s.x].f, { s.x, s.y } });

		while (q.size()) {
			pair<int, pair<int, int>> now = q.top();
			q.pop();

			for (int i = 0; i < 8; i++) {
				int nx = now.second.first + dx[i];
				int ny = now.second.second + dy[i];
				

				if (ny < 0 || nx < 0 || ny >= rowSize || nx >= colSize) continue;
				if (map[ny][nx].tState == WALL || map[ny][nx].tState == START ) continue;
				if (map[ny][nx].tState == END) {
					map[ny][nx].prev = { now.second.first, now.second.second };
					return;
				}
				map[ny][nx].tState = OPEN;
				int ng = map[now.second.second][now.second.first].g + (i % 2 == 0 ? 10 : 14);
				int nh = calculateH(nx, ny);
				int nf = ng + nh;
				if (map[ny][nx].f != -1 && map[ny][nx].f < nf) continue;
				map[ny][nx].g = ng;
				map[ny][nx].h = nh;
				map[ny][nx].f = nf;
				map[ny][nx].prev = { now.second.first, now.second.second };
				q.push({ map[ny][nx].f, {nx, ny} });
			}
		}
	}
	void _SetRoad(int y, int x) {
		if (y == s.y && x == s.x) {
			road.push_back({ x, y });
			playerMove.push_back({ x, y });
			return;
		}

		road.push_back({ x, y });
		playerMove.push_back({ x, y });
		_SetRoad(map[y][x].prev.y, map[y][x].prev.x);
	}

	void SetRoad() {
		_SetRoad(e.y, e.x);
	}
	void Update()
	{
		if (playerMove.empty()) {
			SetStateNext();
			return;
		}
		player.x = playerMove.back().x;
		player.y = playerMove.back().y;
		playerMove.pop_back();
	}

	int calculateH(int x, int y) {
		int width = abs(x - e.x);
		int height = abs(y - e.y);

		int res = width > height ? height * 14 : width * 14;
		res += abs(width - height) * 10;
		
		return res;
	}

	bool IsInMap(int x, int y)
	{
		if (x < 0) return false;
		if (x >= colSize)return false;
		if (y < 0) return false;
		if (y >= rowSize)return false;
		if (y == s.y && x == s.x) return false;
		if (y == e.y && x == e.x) return false;
		if (map[y][x].g != 0) return false;
		return true;
	}
};
