//客户区的方块的横纵数量
#define HORZ_BLK_NUMS		20	//水平方向方块的数量
#define VERT_BLK_NUMS		32	//竖直方向方块的数量
#define ID_TIMER	1	//计时器ID

//方块信息结构体
typedef struct {
	int style;		//方块的形状
	int type;		//方块的方向
	int block[4][4];//方块4*4网格的图形
} BLOCKTYPE;


int CleanRow(void);//消除满行，返回清除的行数
void NextBlockInit(void);//初始化下个方块
BOOL NewBlockEntry(void);//新方块入场
void ClientInit(int *score);//重置游戏区
BOOL CanToNextPos(int iStyle);//能否移动到下个位置
BOOL GameOver(HWND hwnd, BOOL *bGameBegin);
void PaintNowPos(HWND hwnd, RECT rect, int iBlockSize, BOOL bIsBlack);//绘制/清除当前位置图形
void PaintNextBlock(HWND hwnd, RECT rect, int iBlockSize);//绘制下个方块
void PaintClient(HWND hwnd, RECT rect, int iBlockSize, int score, HFONT hfont);//绘制整个游戏区










#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
