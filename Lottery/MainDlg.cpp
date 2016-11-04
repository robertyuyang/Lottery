#include "stdafx.h"



#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

#include <list>
#include <fstream>
#include <iostream> 
#include <string>
#include <vector>
#include <iosfwd>


#define GO_TIMER_ID 0x10
#define EXTRA_TIMER_ID 0x11
#define INTERVAL 10

volatile LONG g_bStopTimer = FALSE;
volatile LONG g_nMoreTimes = 2;
struct AWARD 
{
	std::wstring strId;
	//std::wstring strAwardTitle;
	std::wstring strAwardName;
	int nCount;
};


std::vector<AWARD> g_AwardList;
std::vector<std::wstring> g_CandidatesList;
std::vector<std::wstring> g_TempWinnerNamesList;

class WINNER
{
public:
	WINNER(std::wstring Name) : strName(Name), bTaken(FALSE){}
private:
	WINNER(){}
public:
	std::wstring strName;
	BOOL bTaken;
};


std::vector<WINNER> g_WinnersList;
int g_nExtraWinnerCount = 0;
int g_nCurrentAwardIndex = -1;
AWARD* g_pCurrentDrawingAward = NULL;

BOOL g_bExtra = FALSE;



void split(std::wstring& s, std::wstring& delim,std::vector< std::wstring >* ret)  
{ 
	ret->clear();
	size_t last = 0;  
	size_t index=s.find_first_of(delim,last);  
	while (index!=std::wstring::npos)  
	{  
		ret->push_back(s.substr(last,index-last));  
		last=index+1;  
		index=s.find_first_of(delim,last);  
	}  
	if (index-last>0)  
	{  
		std::wstring substr = s.substr(last,index-last);
		if(_tcscmp(substr.c_str(), _T("\n")) != 0)
		{
			ret->push_back(substr);  
		}
	}  
}  


VOID CMainDlg::DisplayCandidats()
{
	WTL::CTreeViewCtrl wndTreeCandidates = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE_CANDIDATES));
	wndTreeCandidates.DeleteAllItems();

	HTREEITEM hRoot, hItem;
	TCHAR pc_name[64] = { 0 };
	TV_INSERTSTRUCT TCItem;//插入数据项数据结构
	TCItem.hParent = TVI_ROOT;//增加根项
	TCItem.hInsertAfter = TVI_LAST;//在最后项之后
	TCItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;//设屏蔽
	TCItem.item.pszText = _T("可抽奖人");
	TCItem.item.lParam = 0;//序号 
	TCItem.item.iImage = 0;//正常图标 
	TCItem.item.iSelectedImage = 1;//选中时图标 
	hItem = wndTreeCandidates.InsertItem(&TCItem);//返回根项句柄 
	wndTreeCandidates.Expand(hItem, TVE_EXPAND);//展开上一级树

	hRoot = hItem;


	int nLen = g_CandidatesList.size();
	for(int i = 0; i < nLen ;i++ )
	{
		TCItem.item.pszText = (LPTSTR)g_CandidatesList[i].c_str();
		TCItem.hParent = hRoot;
		TCItem.item.lParam = i;
		hItem = wndTreeCandidates.InsertItem(&TCItem);//返回根项句柄 
	}
	
	wndTreeCandidates.Expand(hRoot, TVE_EXPAND);//展开上一级树



}
BOOL CMainDlg::LoadCandidatesFromFile()
{
	TCHAR szPath[MAX_PATH] = {0};
	if(!GetModuleFileName(NULL,szPath, MAX_PATH))
	{
		return FALSE;
	}

	::PathRemoveFileSpec(szPath);
	::PathAppend(szPath, _T("data\\candidates.txt"));

	TCHAR szNames[2048] = {0};

	std::wifstream wifile(szPath);  
	if(wifile.is_open())    
	{        
		wifile.imbue(std::locale( "", std::locale::all ^ std::locale::numeric));   
		wifile.read(szNames, sizeof(szNames));	
		wifile.close();
	}  
/*	FILE* file = _tfopen(szPath, _T("r"));
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	
	fread(szNames, 1, size, file);
	fclose(file);
*/
	std::wstring strNames(szNames);
	split(strNames, std::wstring(_T(" ")), &g_CandidatesList);

	DisplayCandidats();	
}

VOID CMainDlg::DisplayAwards()
{

	WTL::CTreeViewCtrl wndTree = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE_AWARDS));
	wndTree.DeleteAllItems();

	HTREEITEM hRoot, hItem;
	TCHAR pc_name[64] = { 0 };
	TV_INSERTSTRUCT TCItem;//插入数据项数据结构
	TCItem.hParent = TVI_ROOT;//增加根项
	TCItem.hInsertAfter = TVI_LAST;//在最后项之后
	TCItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;//设屏蔽
	TCItem.item.pszText = _T("可抽奖奖品");
	TCItem.item.lParam = 0;//序号 
	TCItem.item.iImage = 0;//正常图标 
	TCItem.item.iSelectedImage = 1;//选中时图标 
	hItem = wndTree.InsertItem(&TCItem);//返回根项句柄 
	wndTree.Expand(hItem, TVE_EXPAND);//展开上一级树

	hRoot = hItem;


	int nLen = g_AwardList.size();
	for(int i = 0; i < nLen ;i++ )
	{
		TCItem.item.pszText = (LPTSTR)(g_AwardList[i].strAwardName.c_str());
		TCItem.hParent = hRoot;
		TCItem.item.lParam = i;
		hItem = wndTree.InsertItem(&TCItem);//返回根项句柄 
	}

	wndTree.Expand(hRoot, TVE_EXPAND);//展开上一级树



}
BOOL CMainDlg::LoadAwardsFromFile()
{
	TCHAR szPath[MAX_PATH] = {0};
	if(!GetModuleFileName(NULL,szPath, MAX_PATH))
	{
		return FALSE;
	}

	::PathRemoveFileSpec(szPath);
	::PathAppend(szPath, _T("data\\awards.txt"));

	/*FILE* file = _tfopen(szPath, _T("r"));
	TCHAR szAward[100] = {0};
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);
	TCHAR szNames[2048] = {0};
	fread(szNames, 1, size, file);
	fclose(file);*/

	TCHAR szNames[2048] = {0};

	std::wifstream wifile(szPath);  
	if(wifile.is_open())    
	{        
		wifile.imbue(std::locale( "", std::locale::all ^ std::locale::numeric));   
		wifile.read(szNames, sizeof(szNames));	
		wifile.close();
	}  


	std::vector<std::wstring> AwardStringList;
	split(std::wstring(szNames), std::wstring(_T("\n")), &AwardStringList);

	g_AwardList.clear();
	for(auto it = AwardStringList.begin(); it != AwardStringList.end(); it++)
	{
		AWARD Award;
		std::wstring AwardItemString = *it;
		if(AwardItemString.length() == 0)
		{
			break;
		}
		std::vector<std::wstring> AwardString;
		split(AwardItemString, std::wstring(_T(" ")), &AwardString);
		Award.strId = AwardString[0];
		Award.strAwardName = AwardString[1];
		Award.nCount = _ttoi((AwardString[2]).c_str());
		g_AwardList.push_back(Award);
	}
	DisplayAwards();
}

VOID CMainDlg::InitFonts()
{
	static WTL::CFont TitleFont;
	if(TitleFont.m_hFont == NULL)
	{
		LOGFONT tempfont = {30,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,_T("黑体")};

		TitleFont.CreateFontIndirect(&tempfont);
	}
	CEdit wndEdit = CEdit(GetDlgItem(IDC_EDIT_LOTTERY));
	wndEdit.SetFont(TitleFont);
}
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	InitFonts();
	LoadCandidatesFromFile();
	LoadAwardsFromFile();
	AddLog(_T("hahah"));
	AddLog(_T("h111ah"));
	AddLog(_T("hah66"));

	return TRUE;
}


LRESULT CMainDlg::OnGo(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	if(g_CandidatesList.size() == 0)
	{
		MessageBox(_T("没有参与者了"));
		return 0;
	}


	if(g_pCurrentDrawingAward != NULL)
	{
		MessageBox(_T("上一个抽奖还在进行中(未保存 )"));
		return 0;
	}

	WTL::CTreeViewCtrl wndTreeCandidates = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE_AWARDS));
	HTREEITEM hRoot = wndTreeCandidates.GetRootItem();
	HTREEITEM item = wndTreeCandidates.GetSelectedItem();
	if(item == hRoot)
	{
		return 0;
	}
	if(item == 0)
	{
		MessageBox(_T("选择一个奖品 "));
		return 0;
	}
	int lParam =	wndTreeCandidates.GetItemData(item);

	g_nCurrentAwardIndex = lParam;
	g_pCurrentDrawingAward = &g_AwardList[lParam];

	CStatic wndCurAward = CStatic(GetDlgItem(IDC_STATIC_CURRENT_AWARD));
	TCHAR szText[100] = {0};
	_stprintf(szText, _T("当前正在抽： %s %d个"),  
		g_pCurrentDrawingAward->strAwardName.c_str(), g_pCurrentDrawingAward->nCount);
	wndCurAward.SetWindowText(szText);

	CButton button = CButton(GetDlgItem(IDC_BUTTON_GO));
	button.EnableWindow(FALSE);
	button = CButton(GetDlgItem(IDC_BUTTON_EXTRA));
	button.EnableWindow(FALSE);
	button = CButton(GetDlgItem(IDC_BUTTON_STOP));
	button.EnableWindow(TRUE);

	g_bExtra = FALSE;
	SetTimer(GO_TIMER_ID, INTERVAL, NULL);
	return 0;
}


LRESULT CMainDlg::OnStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(g_pCurrentDrawingAward == NULL)
	{
		return 0;
	}


	//InterlockedExchange(&g_bStopTimer, TRUE);
	//InterlockedExchange(&)
	KillTimer(GO_TIMER_ID);
	KillTimer(EXTRA_TIMER_ID);

	CButton button = CButton(GetDlgItem(IDC_BUTTON_GO));
	button.EnableWindow(TRUE);
	button = CButton(GetDlgItem(IDC_BUTTON_EXTRA));
	button.EnableWindow(TRUE);
	button = CButton(GetDlgItem(IDC_BUTTON_STOP));
	button.EnableWindow(FALSE);

	if(!g_bExtra)
	{
		g_WinnersList.clear();
	}
	//for(auto it = g_TempWinnerNamesList.begin(); it != g_TempWinnerNamesList.
	g_WinnersList.insert(g_WinnersList.begin(), g_TempWinnerNamesList.begin(), g_TempWinnerNamesList.end());

	//g_pCurrentDrawingAward = NULL;
	//g_nCurrentAwardIndex = -1;

	for(auto it = g_WinnersList.begin(); it != g_WinnersList.end(); it++)
	{
		for(auto srcit = g_CandidatesList.begin(); srcit != g_CandidatesList.end(); srcit++)
		{
			if(_tcscmp(srcit->c_str(), it->strName.c_str()) == 0)
			{
				g_CandidatesList.erase(srcit);
				break;
			}
		}
	}

	DisplayCandidats();
	DisplayWinners();

	return 0;

}
#define random(x) (rand()%x)
#include <map>
VOID Draw(int nCount)
{
	if(g_pCurrentDrawingAward == NULL) 
	{
		return;
	}

	g_TempWinnerNamesList.clear();
	int nCandidatesCount = g_CandidatesList.size();

	std::map<int,int> NumSet;

	nCount = min(nCandidatesCount, nCount);
	for(int i=0;i<nCount;i++)
	{
		srand((int)::GetTickCount());
		int nIndex = random(nCandidatesCount);
		while(NumSet.find(nIndex) != NumSet.end())
		{
			nIndex = random(nCandidatesCount);
		}
		NumSet[nIndex] = 1;
		g_TempWinnerNamesList.push_back(g_CandidatesList[nIndex]);
	}
		
}


LRESULT CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(wParam == GO_TIMER_ID)
	{
		Draw(g_pCurrentDrawingAward->nCount);
		DisplayTempWinnerNames();	


	}
	else if(wParam == EXTRA_TIMER_ID)
	{
		Draw(g_nExtraWinnerCount);
		DisplayTempWinnerNames();
	}
	return 0;
}

VOID CMainDlg::AddLog(LPCTSTR lpszLog)
{
	CEdit wndEdit = CEdit(GetDlgItem(IDC_EDIT_LOG));
	TCHAR szText[10240] = {0};
	wndEdit.GetWindowText(szText, sizeof(szText)/sizeof(szText[0]));
	_tcscat(szText, lpszLog);
	wndEdit.SetWindowText(szText);
}


VOID CMainDlg::DisplayWinners()
{
	WTL::CTreeViewCtrl wndTree = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE_WINNERS));
	wndTree.DeleteAllItems();

	if(g_WinnersList.size() == 0)
	{
		return;
	}

	HTREEITEM hRoot, hItem;
	TCHAR pc_name[64] = { 0 };
	TV_INSERTSTRUCT TCItem;//插入数据项数据结构
	TCItem.hParent = TVI_ROOT;//增加根项
	TCItem.hInsertAfter = TVI_LAST;//在最后项之后
	TCItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;//设屏蔽
	TCItem.item.pszText = _T("获奖人");
	TCItem.item.lParam = 0;//序号 
	TCItem.item.iImage = 0;//正常图标 
	TCItem.item.iSelectedImage = 1;//选中时图标 
	hItem = wndTree.InsertItem(&TCItem);//返回根项句柄 
	wndTree.Expand(hItem, TVE_EXPAND);//展开上一级树

	hRoot = hItem;


	int nLen = g_WinnersList.size();
	for(int i = 0; i < nLen ;i++ )
	{
		TCItem.item.pszText = (LPTSTR)(g_WinnersList[i].strName.c_str());
		TCItem.hParent = hRoot;
		TCItem.item.lParam = i;
		hItem = wndTree.InsertItem(&TCItem);//返回根项句柄 
		wndTree.SetCheckState(hItem, g_WinnersList[i].bTaken);
	}

	wndTree.Expand(hRoot, TVE_EXPAND);//展开上一级树
}



LRESULT CMainDlg::OnSave(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CalcTakenAwards();
	if(g_nExtraWinnerCount != 0)
	{
		if(IDNO == MessageBox(_T("尚有未领取奖品，是否要继续保存？"),0, MB_YESNO))
		{
			return 0;
		}
	}

	BackUpData();
	SaveRec();

	g_AwardList.erase(g_AwardList.begin() + g_nCurrentAwardIndex);

	SaveData();


	g_nCurrentAwardIndex = -1;
	g_pCurrentDrawingAward = NULL;
	g_bExtra = FALSE;
	g_nExtraWinnerCount = 0;
	g_WinnersList.clear();	
	g_TempWinnerNamesList.clear();

	DisplayWinners();
	DisplayAwards();
	DisplayTempWinnerNames();


	return 0;
}


VOID CMainDlg::CalcTakenAwards()
{
	if(!g_pCurrentDrawingAward)
	{
		return;
	}

	WTL::CTreeViewCtrl wndTree = WTL::CTreeViewCtrl(GetDlgItem(IDC_TREE_WINNERS));
	HTREEITEM item = wndTree.GetSelectedItem();

	g_nExtraWinnerCount = g_pCurrentDrawingAward->nCount;; 
	auto hRoot = wndTree.GetRootItem();
	auto hChild = wndTree.GetChildItem(hRoot);
	while(hChild)
	{
		DWORD lParam = wndTree.GetItemData(hChild);
		BOOL bChecked = wndTree.GetCheckState(hChild);
		g_WinnersList[lParam].bTaken = bChecked;
		if(bChecked)
		{
			g_nExtraWinnerCount--;
		}
		hChild = wndTree.GetNextSiblingItem(hChild);
	}

}
LRESULT CMainDlg::OnExtra(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{

	if(g_CandidatesList.size() == 0)
	{
		MessageBox(_T("没有参与者了"));
		return 0;
	}

	CalcTakenAwards();	
	if(g_nExtraWinnerCount == 0)
	{
		MessageBox(_T("本轮奖品已领完，无法补抽"), 0, 0);
		return 0;
	}
	CStatic wndCurAward = CStatic(GetDlgItem(IDC_STATIC_CURRENT_AWARD));
	TCHAR szText[100] = {0};
	_stprintf(szText, _T("当前正在抽： %s 补抽%d个"),  
		g_pCurrentDrawingAward->strAwardName.c_str(), g_nExtraWinnerCount);
	wndCurAward.SetWindowText(szText);
	g_bExtra = TRUE;
	SetTimer(EXTRA_TIMER_ID, INTERVAL, NULL);

	CButton button = CButton(GetDlgItem(IDC_BUTTON_GO));
	button.EnableWindow(FALSE);
	button = CButton(GetDlgItem(IDC_BUTTON_EXTRA));
	button.EnableWindow(FALSE);
	button = CButton(GetDlgItem(IDC_BUTTON_STOP));
	button.EnableWindow(TRUE);

	return 0;
}

VOID CMainDlg::DisplayTempWinnerNames()
{
	std::wstring strText;
	int i = 0;
	for (auto it = g_TempWinnerNamesList.begin(); it != g_TempWinnerNamesList.end(); it++)
	{
		strText.append(*it);
		i++;
		if(i % 2 == 0)
		{
			strText.append(_T("\r\n"));
		}
		else
		{
			strText.append(_T("\t"));
		}
	}

	CEdit wndLottery(GetDlgItem(IDC_EDIT_LOTTERY));
	wndLottery.SetWindowText(strText.c_str());
}

#include <locale>
#include <fstream>
BOOL CMainDlg::SaveData()
{
	
	TCHAR szDir[MAX_PATH] = {0};	
	if(!GetModuleFileName(NULL,szDir, MAX_PATH))
	{
		return FALSE;
	}

	::PathRemoveFileSpec(szDir);


	TCHAR szDataFilePath[MAX_PATH] = {0};

	std::wstring strText;
	//rec

	
	//candidates

	{


		strText.clear();
		for(auto it = g_CandidatesList.begin(); it != g_CandidatesList.end(); it++)
		{
			strText.append(*it);
			strText.append(_T(" "));
		}

		_tcsnccpy(szDataFilePath, szDir, MAX_PATH);
		::PathAppend(szDataFilePath, _T("data//candidates.txt"));


		std::wofstream wofile(szDataFilePath);  
		if(wofile.is_open())    
		{        
			wofile.imbue(std::locale( "", std::locale::all ^ std::locale::numeric));   
			wofile << strText.c_str() <<std::endl;		
			wofile.close();
		}  
	}

	//awards

	_tcsnccpy(szDataFilePath, szDir, MAX_PATH);
	::PathAppend(szDataFilePath, _T("data//awards.txt"));
	
	/*
	strText.clear();
	for(auto it = g_AwardList.begin(); it != g_AwardList.end(); it++)
	{
		TCHAR szTemp[1000] = {0};
		_stprintf(szTemp, _T("%s %s %d\r\n"), it->strId.c_str(), it->strAwardName.c_str(), it->nCount);
		strText.append(szTemp);
	}
*/


	std::wofstream wofile(szDataFilePath);  
	if(wofile.is_open())    
	{ 
		wofile.imbue(std::locale( "", std::locale::all ^ std::locale::numeric));   
		for(auto it = g_AwardList.begin(); it != g_AwardList.end(); it++)
		{
			TCHAR szTemp[1000] = {0};
			_stprintf(szTemp, _T("%s %s %d"), it->strId.c_str(), it->strAwardName.c_str(), it->nCount);
			wofile << szTemp <<std::endl;		
		}
		wofile.close();
	} 
	return TRUE;

	
}

BOOL CMainDlg::BackUpData()
{
	TCHAR szDir[MAX_PATH] = {0};	
	if(!GetModuleFileName(NULL,szDir, MAX_PATH))
	{
		return FALSE;
	}

	::PathRemoveFileSpec(szDir);

	//backup

	SYSTEMTIME st;
	::GetSystemTime(&st);
	TCHAR szSubDirName[MAX_PATH] = {0};
	_stprintf(szSubDirName, _T("%.4d%.2d%.2d%.2d%.2d%.2d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	TCHAR szBackUpSubDir[MAX_PATH] = {0};
	_tcsnccpy(szBackUpSubDir, szDir, MAX_PATH);
	::PathAppend(szBackUpSubDir, _T("data//backup"));
	CreateDirectory(szBackUpSubDir, NULL);
	::PathAppend(szBackUpSubDir, szSubDirName);
	CreateDirectory(szBackUpSubDir, NULL);

	TCHAR* DataFileNames[] = {_T("candidates.txt"), _T("awards.txt"), _T("records.txt")};
	for(int i = 0; i < sizeof(DataFileNames)/sizeof(DataFileNames[0]); i++)
	{
		TCHAR szDataFile[MAX_PATH] = {0};
		TCHAR szDstPath[MAX_PATH] = {0};
		_tcsnccpy(szDataFile, szDir, MAX_PATH);
		::PathAppend(szDataFile, _T("data"));
		::PathAppend(szDataFile, DataFileNames[i]);

		::PathAppend(szDstPath, szBackUpSubDir);
		::PathAppend(szDstPath, DataFileNames[i]);
		::MoveFile(szDataFile, szDstPath);
	}

}

BOOL CMainDlg::SaveRec()
{

	TCHAR szDir[MAX_PATH] = {0};	
	if(!GetModuleFileName(NULL,szDir, MAX_PATH))
	{
		return FALSE;
	}

	::PathRemoveFileSpec(szDir);


	TCHAR szDataFilePath[MAX_PATH] = {0};

	std::wstring strText;


	TCHAR szText[1000] = {0};
	_stprintf(szText, _T("%s %d个:  "), g_pCurrentDrawingAward->strAwardName.c_str(), g_pCurrentDrawingAward->nCount);
	strText = szText;
	for(auto it = g_WinnersList.begin(); it != g_WinnersList.end(); it++)
	{
		strText.append(it->strName);
		if(!it->bTaken)
		{
			strText.append(_T("(未领奖)"));
		}
		strText.append(_T(","));
	}

	_tcsnccpy(szDataFilePath, szDir, MAX_PATH);
	::PathAppend(szDataFilePath, _T("data//records.txt"));


	std::wofstream wofile(szDataFilePath, std::ios::app);  
	if(wofile.is_open())    
	{        
		wofile.imbue(std::locale( "", std::locale::all ^ std::locale::numeric));   
		wofile << strText.c_str() <<std::endl;		
		wofile.close();
	} 

}


