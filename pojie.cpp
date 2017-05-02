#include <windows.h>  
#include <stdio.h>  

DWORD dwPID = 0; 
HWND hWinmine = 0;
DWORD dwBaseAddr = 0x01005360;//去掉边框雷区开始的位置
DWORD dwWidthAddr = 0x01005334;//横向格子
DWORD dwHeightAddr = 0x01005338;//纵向格子
int dwWidth = 0, dwHeight = 0;


void select();

int check()//检查扫雷游戏是否运行
{
	hWinmine = FindWindow(NULL, "Minesweeper");  
    GetWindowThreadProcessId(hWinmine, &dwPID);//得到PID  
	if(dwPID == 0)
		return 0;
	return 1;
}

void ask()
{
	int ans = 0;
	printf("继续使用其他功能吗？按1继续，按2退出\n");
	scanf("%d",&ans);
	if(ans == 1 && check() == 1)select();
	else return;
}

void display()//显示地雷布局
{
	BYTE flag = 0x8E;//红旗
	DWORD dwflagSize = 0;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);//得到句柄
   
    //读横、纵向格子数目
    ReadProcessMemory(hProcess, (LPVOID)dwWidthAddr, &dwWidth, sizeof(DWORD), 0);  
    ReadProcessMemory(hProcess, (LPVOID)dwHeightAddr, &dwHeight, sizeof(DWORD), 0);  


	for(int i = 0;i < dwHeight;i++)
	{
		for(int j = 0;j < dwWidth+2;j++)//有墙
		{
			DWORD currentInfo = 0;
			DWORD addr = dwBaseAddr+j+(i*0x20);//从某一行第一列到下一行第一列间隔了0x20大小的地址
			ReadProcessMemory(hProcess, (LPVOID)(addr), &currentInfo, sizeof(DWORD), &dwflagSize);
			currentInfo=currentInfo&0x000000ff;
			if(currentInfo == 0x8f)
			{
				WriteProcessMemory(hProcess, (LPVOID)(addr), &flag, sizeof(BYTE),&dwflagSize);//换红旗标注
			}
		}
	}
	CloseHandle(hProcess);   
	printf("\n已显示雷区布局，以红旗标明。\n如果没有显示，请先最小化扫雷游戏，使界面重绘...\n");
	printf("-------------------------------\n\n");
	ask();
}
  
void stoptime()//停止计时器
{
	DWORD dwTimer = 0x01002FF5;//指令地址
	DWORD dwTimeSize = 4;//指令FF059C57

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);//得到句柄

	BYTE* nopsInfo = new BYTE[4];
	for (size_t i = 0; i < 4; i++)  //生成同样长度的NOP
    {  
		nopsInfo[i] = 0x90;
	}
	
	WriteProcessMemory(hProcess, (LPVOID)dwTimer, nopsInfo, sizeof(BYTE), &dwTimeSize);//覆盖
	CloseHandle(hProcess);   

	printf("\n已停止计时，游戏不退出将一直有效...\n");
	printf("-------------------------------\n\n");
	ask();
}

void nofail()//无限模式，点到雷也可以继续玩
{
	DWORD dwFail = 0x010034D6;//指令地址
	DWORD dwFailSize = 10;//指令C7 05 00 50 00 01 10 00 00 00

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);//得到句柄

	BYTE* nopsInfo = new BYTE[10];
	for (size_t i = 0; i < 10; i++)  //生成同样长度的NOP
    {  
		nopsInfo[i] = 0x90;
	}
	
	WriteProcessMemory(hProcess, (LPVOID)dwFail, nopsInfo, sizeof(BYTE), &dwFailSize);//覆盖
	CloseHandle(hProcess);   

	printf("\n已开启无限模式，可以随便点，炸了也可以继续\n游戏不退出将一直有效...\n");
	printf("-------------------------------\n\n");
	ask();
}

void autogame()//自动模式，程序自动玩游戏，会跳过所有雷
{
	DWORD xBegin = 0xC;
	DWORD yBegin = 0x37;//雷区起点
	DWORD mSize = 0x10;//格子大小

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, dwPID);//得到句柄
   
    //得到横纵向格子数目
    ReadProcessMemory(hProcess, (LPVOID)dwWidthAddr, &dwWidth, sizeof(DWORD), 0);  
    ReadProcessMemory(hProcess, (LPVOID)dwHeightAddr, &dwHeight, sizeof(DWORD), 0); 
	
	BYTE flag = 0x8E;//红旗

	//与显示雷区原理一样
	for(int i = 0;i < dwHeight;i++)
	{
		for(int j = 0;j < dwWidth+2;j++)
		{
			DWORD currentInfo = 0;
			DWORD addr = dwBaseAddr+j+(i*0x20);
			//printf("%x\n",i*0x20);
			ReadProcessMemory(hProcess, (LPVOID)(addr), &currentInfo, sizeof(DWORD), 0);
			currentInfo=currentInfo&0x000000ff;
			//printf("%d %d %x %d\n",i,j,currentInfo,i*0x20);

			if(currentInfo == 0x8f)//是雷则跳过
			{
				continue;
			}

			if(currentInfo == 0x0f)//非雷则模拟点击
			{
				//内存雷区begin的位置肯定是墙，所以要从界面begin的位置往左挪mSize/2，点一下墙才对得上
				SendMessage(hWinmine,WM_LBUTTONDOWN,0,MAKELPARAM(xBegin-mSize/2+mSize*j,
					yBegin+mSize/2+mSize*i));
				SendMessage(hWinmine,WM_LBUTTONUP,0,MAKELPARAM(xBegin-mSize/2+mSize*j,
					yBegin+mSize/2+mSize*i));
			}
			else continue;//显示了数字的也跳过
			Sleep(80);	
		}

	}
	CloseHandle(hProcess);   
	printf("\n已成功完成自动游戏模式...\n");
	printf("注意！注意！\n如果扫雷窗口的任何部位与其他窗口有重叠，无论在上还是在下，都可能导致出错！\n");
	printf("-------------------------------\n\n");
	ask();
}


void exit()
{
	printf("\n谢谢使用\n\n");
	return;
}


void select()
{
	printf("请输入序号：\n");
	printf("1 显示游戏布局\n");
	printf("2 停止计时器\n");
	printf("3 无限游戏模式\n");
	printf("4 自动游戏模式\n");
	printf("0 退出\n");
	int op = 0;
	scanf("%d",&op);
	if(op == 1)
		display();
	if(op == 2)
		stoptime();
	if(op == 3)
		nofail();
	if(op == 4)
		autogame();
	if(op == 0)
		exit();
}

void main()  
{
	if(check() == 0)
	{
		printf("请先运行扫雷游戏...\n");
		getchar();
		return;
	}
	printf("已检测到扫雷游戏...\n\n");
	select();
	system("pause");
}  