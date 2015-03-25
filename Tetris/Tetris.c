/*-------------------------
	俄罗斯方块游戏模块
	-------------------------*/
#include <Windows.h>
#include "Tetris.h"

static BLOCKTYPE NowBlock,//当前活动方块的类型信息
NextBlock;//下个方块的类型信息
static POINT ptNowPos[4],//活动方块的当前位置（在游戏区结构iClient中的点）
ptNextPos[4];//活动方块的下个位置
static int iClient[VERT_BLK_NUMS][HORZ_BLK_NUMS];//游戏区记录结构
//游戏方块的形状（第一维）及方向（第二维）
//4*4网格的方格状态决定图形：0为白色、1为黑色
static int Member[][4][4][4] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0,
0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0
};
//游戏区方块清空
void ClientInit(int *score)
{
	int i, j;

	*score = 0;//分数清零
	for (i = 0; i < VERT_BLK_NUMS; i++)
	for (j = 0; j < HORZ_BLK_NUMS; j++)
		iClient[i][j] = 0;//游戏区结构重置
	return;
}
//获取下一个方块
void NextBlockInit(void)
{
	int iType, iStyle, i, j;
	//从游戏形状数组中随机选择一个作为下个方块的形状和方向
	iStyle = rand() / (RAND_MAX / 7);
	iType = rand() / (RAND_MAX / 4);
	for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		NextBlock.block[i][j] = Member[iStyle][iType][i][j];

	NextBlock.style = iStyle;
	NextBlock.type = iType;

	return;
}
// 绘制/擦除 客户区活动方块
void PaintNowPos(HWND hwnd, RECT rect, int iBlockSize, BOOL bIsBlack)
{
	int i;
	HDC hdc;

	hdc = GetDC(hwnd);

	for (i = 0; i < 4; i++)
	{
		SelectObject(hdc, GetStockObject(bIsBlack ? BLACK_PEN : WHITE_PEN));//bIsBlack 决定是擦除还是绘制（黑/白画笔）
		Rectangle(hdc,
			rect.left + iBlockSize * ptNowPos[i].x + 1,
			rect.top + iBlockSize * ptNowPos[i].y + 1,
			rect.left + iBlockSize * (ptNowPos[i].x + 1) - 1,
			rect.top + iBlockSize * (ptNowPos[i].y + 1) - 1);
	}
	ReleaseDC(hwnd, hdc);
	return;
}
//绘制下个方块
void PaintNextBlock(HWND hwnd, RECT rect, int iBlockSize)
{
	int i, j;
	HDC hdc;

	hdc = GetDC(hwnd);

	for (i = 0; i < 4; i++)		//绘制下个方块样式
	for (j = 0; j < 4; j++)
	{
		SelectObject(hdc, GetStockObject(NextBlock.block[i][j] ? BLACK_PEN : WHITE_PEN));
		Rectangle(hdc,
			rect.right + iBlockSize * (j + 2) + 1,
			rect.bottom / 4 + iBlockSize * i + 1,
			rect.right + iBlockSize * (j + 3) - 1,
			rect.bottom / 4 + iBlockSize * (i + 1) - 1);
	}

	ReleaseDC(hwnd, hdc);
	return;
}
//绘制整个游戏画面
void PaintClient(HWND hwnd, RECT rect, int iBlockSize, int score, HFONT hfont)
{
	int i, j;
	HDC hdc;
	TCHAR temp[20];

	hdc = GetDC(hwnd);
	//绘制游戏区边框
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	//绘制游戏区方块
	for (i = 0; i < VERT_BLK_NUMS; i++)
	for (j = 0; j < HORZ_BLK_NUMS; j++)
	{
		SelectObject(hdc, GetStockObject(iClient[i][j] ? BLACK_PEN : WHITE_PEN));
		Rectangle(hdc,
			rect.left + iBlockSize * j + 1,
			rect.top + iBlockSize * i + 1,
			rect.left + iBlockSize * (j + 1) - 1,
			rect.top + iBlockSize * (i + 1) - 1);

	}
	PaintNextBlock(hwnd, rect, iBlockSize);

	hfont = (HFONT)SelectObject(hdc, hfont);
	wsprintf(temp, TEXT("分数：%d"), score);//显示分数
	TextOut(hdc, rect.right + iBlockSize, rect.bottom / 2, temp, wcslen(temp));
	hfont = (HFONT)SelectObject(hdc, hfont);

	ReleaseDC(hwnd, hdc);
	return;
}
//根据提供方块样式及其整体坐标x、y（4*4方块图左下角），计算方块在游戏区位置坐标
void GetBlockPos(int x, int y, const BLOCKTYPE *Block, POINT *NextPos)
{
	int i, j, iFirstLine = -1, num = 0;

	for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
	if (Block->block[i][j])
	{
		if (iFirstLine < 0)
			iFirstLine = i;
		NextPos[num].x = j + x;
		NextPos[num++].y = i + (y == -4 ? -iFirstLine : y);
	}
}
//新方块入场，成功返回TRUE,反之为FALSE。同时更新下个方块样式。
BOOL NewBlockEntry(void)
{
	int i;

	NowBlock = NextBlock;//将原“下个”方块变为活动方块
	NextBlockInit();//获得新“下个”方块
	GetBlockPos(8, -4, &NowBlock, ptNextPos);//初始化活动方块位置
	for (i = 0; i < 4; i++)
	if (iClient[ptNextPos[i].y][ptNextPos[i].x] == 1)//判断活动方块出现位置是否已被占用
		return FALSE;//被占用，游戏结束，返回false
	for (i = 0; i < 4; i++)
	{
		ptNowPos[i] = ptNextPos[i];
		iClient[ptNowPos[i].y][ptNowPos[i].x] = 1;//成功入场，在游戏区数组中标记位置
	}
	return TRUE;
}
//获得活动方块网格左下角的当前游戏区位置，用以确定方块的新位置
void TranslatePos(const BLOCKTYPE *Block)
{
	int i, j, x, y;
	BOOL jump = FALSE;

	x = ptNowPos[0].x;
	y = ptNowPos[0].y;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		if (NowBlock.block[i][j])
		{
			jump = TRUE;
			break;
		}
		if (jump)
			break;
	}
	x -= j;
	y -= i;
	GetBlockPos(x, y, Block, ptNextPos);
	return;
}
//计算方块下个位置
BOOL CanToNextPos(int iStyle)
{
	int i, j;
	BOOL bCanChange = TRUE;
	BLOCKTYPE temp;
	//先从游戏区清空移动方块的当前位置，方便计算下个位置
	for (i = 0; i < 4; i++)
	{
		ptNextPos[i] = ptNowPos[i];
		iClient[ptNowPos[i].y][ptNowPos[i].x] = 0;
	}
	switch (iStyle)
	{
	case VK_LEFT:		//向左移动
		for (i = 0; i < 4; i++)
		{
			ptNextPos[i].x -= 1;
			if (ptNextPos[i].x < 0 || iClient[ptNextPos[i].y][ptNextPos[i].x] == 1)
			{
				bCanChange = FALSE;
				break;
			}
		}
		break;
	case VK_RIGHT:		//向右移动
		for (i = 0; i<4; i++)
		{
			ptNextPos[i].x += 1;
			if (ptNextPos[i].x > HORZ_BLK_NUMS - 1 || iClient[ptNextPos[i].y][ptNextPos[i].x] == 1)
			{
				bCanChange = FALSE;
				break;
			}
		}
		break;
	case VK_DOWN://下落
		for (i = 0; i<4; i++)
		{
			ptNextPos[i].y += 1;
			if (ptNextPos[i].y > VERT_BLK_NUMS - 1 || iClient[ptNextPos[i].y][ptNextPos[i].x] == 1)
			{
				bCanChange = FALSE;
				break;
			}
		}
		break;
	case VK_SPACE:		//方块翻转
		//先计算翻转后方块的样式
		temp = NowBlock;
		if (++(temp.type) > 3)
			temp.type = 0;
		for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			temp.block[i][j] = Member[temp.style][temp.type][i][j];
		//确定新样式的坐标
		TranslatePos(&temp);
		//判断新坐标是否合法
		for (i = 0; i<4; i++)
		{
			if (ptNextPos[i].x > HORZ_BLK_NUMS - 1 || ptNextPos[i].x < 0 || ptNextPos[i].y > VERT_BLK_NUMS - 1 ||
				ptNextPos[i].y < 0 || iClient[ptNextPos[i].y][ptNextPos[i].x] == 1)
			{
				bCanChange = FALSE;
				break;
			}

		}
		if (bCanChange)
			NowBlock = temp;
		break;
	}

	for (i = 0; i < 4; i++)
	{
		if (bCanChange)
			ptNowPos[i] = ptNextPos[i];		//新位置合法的话便应用新位置
		iClient[ptNowPos[i].y][ptNowPos[i].x] = 1;	//重新绘制移动方块的位置
	}
	return bCanChange;
}
//消除满行的方块，返回消除的行数
int CleanRow(void)
{
	int i, j, n, num = 0, NeedClean[4];
	BOOL jump;

	for (i = 0; i < VERT_BLK_NUMS; i++)
	{
		jump = FALSE;
		for (j = 0; j < HORZ_BLK_NUMS; j++)
		if (!iClient[i][j])
		{
			jump = TRUE;
			break;
		}
		if (!jump)
			NeedClean[num++] = i;
	}
	for (i = 0; i<num; i++)
	{
		for (j = NeedClean[i]; j>0; j--)
		{
			for (n = 0; n < HORZ_BLK_NUMS; n++)
				iClient[j][n] = iClient[j - 1][n];
		}
		for (n = 0; n < HORZ_BLK_NUMS; n++)
			iClient[0][n] = 0;
	}
	return num;
}
