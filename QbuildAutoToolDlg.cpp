// QbuildAutoToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "QbuildAutoToolDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

#define THREADPOOL_SIZE 5
// CQbuildAutoToolDlg dialog
CQbuildAutoToolDlg::CQbuildAutoToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQbuildAutoToolDlg::IDD, pParent), isEditGitPathChange(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_thrdpoolParse.Initialize(NULL, THREADPOOL_SIZE);
}

void CQbuildAutoToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BRANCH, m_cmbBranch);
	DDX_Control(pDX, IDC_TAB2, m_tabFileInfo);
}

BEGIN_MESSAGE_MAP(CQbuildAutoToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CQbuildAutoToolDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_BUTTON_PUSH_FILE, &CQbuildAutoToolDlg::OnBnClickedPushFile)
	ON_BN_CLICKED(IDC_BUTTON_GET_FILE, &CQbuildAutoToolDlg::OnBnClickedGetFile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CQbuildAutoToolDlg::OnBnClickedBrowse)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB2, &CQbuildAutoToolDlg::OnTcnSelchangeTab2)
    ON_MESSAGE(WM_SUCCESS_DOWNLOAD, &CQbuildAutoToolDlg::OnSuccessDownloadFile)
ON_EN_KILLFOCUS(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnKillfocusEditGitPath)
ON_EN_CHANGE(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnChangeEditGitPath)
ON_CBN_SELCHANGE(IDC_COMBO_BRANCH, &CQbuildAutoToolDlg::OnCbnSelchangeComboBranch)
END_MESSAGE_MAP()


// CQbuildAutoToolDlg message handlers
BOOL CQbuildAutoToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, FALSE);			// Set big icon
	SetIcon(m_hIcon, TRUE);		// Set small icon


	///////////////////////////////设置TabControl选项框/////////////////////
	m_tabFileInfo.InsertItem(0, L"文件信息");
	m_tabFileInfo.InsertItem(1, L"日志");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabFileInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabFileInfo);

	// 获取m_tab控件大小，并将m_tab控件的大小适当改小
	CRect	rec;
	m_tabFileInfo.GetClientRect(&rec);
	rec.top += 20; // 去掉选项卡标签页那一块空间
	rec.bottom -= 2;
	rec.right -= 2;

	m_tabItemFileInfo.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_SHOWWINDOW | SWP_NOSIZE);
	m_tabItemLog.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_HIDEWINDOW);
	m_tabFileInfo.SetCurSel(0);

	return FALSE; 
}

void CQbuildAutoToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CQbuildAutoToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CQbuildAutoToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 窗口界面框放大后界面自适应
//void CQbuildAutoToolDlg::OnSize(UINT nType, int cx, int cy)
//{
//	CDialog::OnSize(nType, cx, cy);
//
//	//// TODO: Add your message handler code here
//	if (nType==SIZE_MAXIMIZED || nType==SIZE_RESTORED)
//	{
//		ReSize();
//	/*	if (m_tab.m_hWnd)
//		{
//			m_tab.SendMessage(WM_SIZE,(WPARAM)nType,MAKELONG(cx, cy));
//		}
//		if(m_FileInfoTab.m_hWnd)
//		{
//			m_FileInfoTab.SendMessage(WM_SIZE,(WPARAM)nType,MAKELONG(cx,cy));
//		}
//		if(m_LogTab.m_hWnd)
//		{
//			m_LogTab.SendMessage(WM_SIZE,(WPARAM)nType,MAKELONG(cx,cy));
//		}*/
//	}
//}
//
//void CQbuildAutoToolDlg::ReSize()
//{
//	CRect rcWnd;
//	GetWindowRect(&rcWnd); // 获得窗口坐标
//	//if (CFileInfoTab::m_listCtrlFileInfo.GetSafeHwnd())
//	//{
//	//	CRect rcListCtrl;
//	//	CFileInfoTab::m_listCtrlFileInfo.GetWindowRect(&rcListCtrl);
//	//	rcListCtrl.right = rcWnd.right-92;
//	//	rcListCtrl.bottom = rcWnd.bottom-77;
//	//	ScreenToClient(&rcListCtrl);
//	//	CFileInfoTab::m_listCtrlFileInfo.MoveWindow(rcListCtrl, TRUE);
//	//}
//
//	if (m_tab.GetSafeHwnd())
//	{
//		CRect rc;
//		m_tab.GetWindowRect(&rc);
//		rc.right = rcWnd.right-72;
//		rc.bottom = rcWnd.bottom-66;
//		ScreenToClient(&rc);
//		m_tab.MoveWindow(rc, TRUE);
//
//		rc.top -= 25;
//		m_FileInfoTab.MoveWindow(rc, TRUE);
//		//CFileInfoTab tempFileInfoTab;
//		//tempFileInfoTab = GetDlgItem(IDD_FILE_INFO);
//		//tempFileInfoTab.MoveWindow(rc, TRUE);
//		////GetDlgItem(IDC_LIST2).MoveWindow(rc, TRUE);
//
//	}
//
//	if (m_FileInfoTab.GetSafeHwnd())
//	{
//		CRect rc;
//		m_FileInfoTab.GetWindowRect(&rc);
//		rc.right = rcWnd.right-72;
//		rc.bottom = rcWnd.bottom-66;
//		ScreenToClient(&rc);
//
//		rc.top -= 25;
//		m_FileInfoTab.MoveWindow(rc, TRUE);
//		//CFileInfoTab tempFileInfoTab;
//		//tempFileInfoTab = GetDlgItem(IDD_FILE_INFO);
//		//tempFileInfoTab.MoveWindow(rc, TRUE);
//		////GetDlgItem(IDC_LIST2).MoveWindow(rc, TRUE);
//
//	}
//
//	// 设置3个button按钮自适应窗口大小
//	int nOffsetX = 376;
//	for (int i = IDC_BUTTON_GET_FILE; i <= IDC_BUTTON_EXIT; i++)
//	{
//		CWnd* pWnd = GetDlgItem(i);
//		if (pWnd)
//		{
//			CRect rc;
//			pWnd->GetWindowRect(&rc);
//			int nWidth = rc.Width();
//			int nHeight =rc.Height();
//			rc.left = rcWnd.right-nOffsetX;
//			rc.top = rcWnd.bottom-40;
//			rc.right = rc.left+nWidth;
//			rc.bottom = rc.top+nHeight;
//			ScreenToClient(&rc);
//			pWnd->MoveWindow(rc, TRUE);
//			nOffsetX-=110;
//		}
//	}
//}


/////////////////////////////////////////“本地git库存放路径”输入框////////////////////////////////////////////////////

// “本地git库存放路径”输入框发生变化后执行该函数，用来判断该路径是否为git库文件以及获取该路径下的分支并添加到下拉框中
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    // TODO: 在此添加控件通知处理程序代码

    if (!isEditGitPathChange)
        return;
    isEditGitPathChange = FALSE;

    //消除下拉框现有所有内容
    ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->ResetContent();

    // 判断文件夹是否为git库文件
    CString strGitPath, strPath, strBranches;
    GetDlgItemText(IDC_EDIT_GIT_PATH, strPath);
    
    strGitPath = strPath + L"\\.git";

    // 该文件夹是git库文件
    if (PathFileExists(strGitPath))
    {
        // 执行命令行获得git仓库分支字符串
		strBranches = CFunction::ExeCmd(TEXT("git branch"), strPath);

        // 获取默认分支
        CString strDefaultBranch = CFunction::GetDefaultBranch(strBranches);   

        // 以换行符分割字符串
        strBranches.Remove(L'*');
        strBranches.Remove(L' ');
        std::set<CString> setCStrResult = CFunction::SplitCString(strBranches, L"\n");

        //将分支消息添加下拉框内容
        std::set<CString>::iterator iter = setCStrResult.begin();
        std::set<CString>::iterator iterEnd = setCStrResult.end();
        for(; iter != iterEnd; iter++)
        {
            ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->AddString(*iter);
        }

        // 将默认消息放置“分支名称”输入框
        m_cmbBranch.SetCurSel(m_cmbBranch.FindString(0, strDefaultBranch));
    }
    else
    {
        SetDlgItemText(IDC_EDIT_GIT_PATH, L"");
        MessageBox(L"该路径不是Git仓库", L"警告", MB_OK);
        return;
    }
}


// 点击“浏览”按钮，实现浏览文件功能获取文件目录路径
void CQbuildAutoToolDlg::OnBnClickedBrowse()
{
	LPWSTR szPath=NULL;     //存放选择的目录路径 
    CString str;
 
	szPath = (LPWSTR)malloc(1024);
    ZeroMemory(szPath, sizeof(szPath));   
 
    BROWSEINFO bi;   
    bi.hwndOwner = m_hWnd;   
    bi.pidlRoot = NULL;   
    bi.pszDisplayName = NULL;   
    bi.lpszTitle = TEXT("请选择本地git目录：");   
    bi.ulFlags = 0;   
    bi.lpfn = NULL;   
    bi.lParam = 0;   
    bi.iImage = 0;   
    //弹出选择目录对话框
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);   
 
    if(lp && SHGetPathFromIDList(lp, szPath))   
    {
		SetDlgItemText(IDC_EDIT_GIT_PATH,szPath);     
        OnEnKillfocusEditGitPath();
    }
}


/////////////////////////////////////////分支输入框////////////////////////////////////////////////////
// 点击下拉框的某项触发该函数（实现点击某个分支将该分支拉下来）
void CQbuildAutoToolDlg::OnCbnSelchangeComboBranch()
{
	CString strSelectedBranch;
	CString szCmdLine;
	CString szGitPath;
	CString szResult;
	GetDlgItemText(IDC_COMBO_BRANCH,strSelectedBranch);
	szCmdLine = L"git checkout " + strSelectedBranch ;
	GetDlgItemText(IDC_EDIT_GIT_PATH,szGitPath);
	szResult = CFunction::ExeCmd(szCmdLine, szGitPath);
	//MessageBox(szResult);
}

/////////////////////////////////////////“获取文件”按钮////////////////////////////////////////////////////
#include <utility>
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
	// 获取edict控件消息
	CString strUrls;
	GetDlgItemText(IDC_EDIT_URL_LIST, strUrls);

	// 分割得到url集合
    std::set<CString> strEachUrl = CFunction::SplitCString(strUrls, L"\r\n");
	std::set<CString>::iterator iterUrl = strEachUrl.begin();
	std::set<CString>::iterator iterUrlEnd = strEachUrl.end();

	// 在当前运行环境新建一个临时文件夹
    CString strSavePath = L"Temp\\";
	CreateDirectory(strSavePath, NULL);

    CTaskBase *pTask = NULL;
	for (; iterUrl!= iterUrlEnd; iterUrl++)
	{
        pTask = new CDownloadTask(this->GetSafeHwnd(), *iterUrl, strSavePath);
        m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
	}
}

LRESULT CQbuildAutoToolDlg::OnSuccessDownloadFile(WPARAM wParam, LPARAM lParam)
{
    CString *pstrFileName = (CString *)wParam;
    CTaskBase *pTask = NULL;
    pTask = new CUnzipTask(this->GetSafeHwnd(), *pstrFileName);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
    delete pstrFileName;
    return TRUE;
}

/////////////////////////////////////////“文件入库”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
	// 判断备注是否为空
	CString strNote;
	GetDlgItemText(IDC_EDIT_NOTE, strNote);
	if (strNote.IsEmpty())
	{
		MessageBox(L"请输入备注！");
		return;
	}

	this->SendLogInfo(strNote);

}


/////////////////////////////////////////“退出”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}

// 点击选项卡
void CQbuildAutoToolDlg::OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int nCurSel;
	nCurSel = m_tabFileInfo.GetCurSel();
	switch(nCurSel)
	{
	case 0:
		m_tabItemFileInfo.ShowWindow(TRUE);
		m_tabItemLog.ShowWindow(FALSE);
		break;
	case 1:
		m_tabItemFileInfo.ShowWindow(FALSE);
		m_tabItemLog.ShowWindow(TRUE);
		break;
	default:;
	}
}

void CQbuildAutoToolDlg::OnEnChangeEditGitPath()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    // TODO:  在此添加控件通知处理程序代码
    isEditGitPathChange = TRUE;
}

void CQbuildAutoToolDlg::SendLogInfo(CString strLogInfo)
{
	m_tabItemLog.m_cbEditLog.SetWindowText(strLogInfo);
}
