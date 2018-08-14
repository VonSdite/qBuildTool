// QbuildAutoToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "QbuildAutoToolDlg.h"
#include "JsonHelper.h"
#include "Resource.h"


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

// CQbuildAutoToolDlg dialog构造函数
CQbuildAutoToolDlg::CQbuildAutoToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQbuildAutoToolDlg::IDD, pParent), m_fEditGitPathChange(FALSE)
    , m_strBranch(_T(""))
    , m_strGitPath(_T(""))
	, m_fIsStartGetFile(FALSE)
	, m_fISStartGit(FALSE)
	, m_fClear(FALSE)
    , m_strNote(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_thrdpoolParse.Initialize(NULL, THREADPOOL_SIZE);
    m_jvRoot.clear();
}

// 析构函数
CQbuildAutoToolDlg::~CQbuildAutoToolDlg()
{
	std::set<FILE_INFO *>::iterator iter = m_setFileInfo.begin(); 
	for (; iter != m_setFileInfo.end();)
	{
		delete *iter;
		m_setFileInfo.erase(iter++);
	}
	// 将临时文件夹删除
	CFunction::RemoveDir(L"Temp");
}

void CQbuildAutoToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_BRANCH, m_cmbBranch);
    DDX_Control(pDX, IDC_TAB_INFO, m_tabInfo);
    DDX_CBString(pDX, IDC_COMBO_BRANCH, m_strBranch);
    DDX_Text(pDX, IDC_EDIT_GIT_PATH, m_strGitPath);
    DDX_Text(pDX, IDC_EDIT_NOTE, m_strNote);
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
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_INFO, &CQbuildAutoToolDlg::OnTcnSelchangeTabInfo)
    ON_MESSAGE(WM_SUCCESS_DOWNLOAD, &CQbuildAutoToolDlg::OnSuccessDownloadFile)
    ON_MESSAGE(WM_COMPLETE_DOWNLOAD, &CQbuildAutoToolDlg::OnLogDownloadStatus)
	ON_MESSAGE(WM_LOG_GIT_INFO, &CQbuildAutoToolDlg::OnLogGitInfo)
    ON_MESSAGE(WM_SHOW_FILE_INFO, &CQbuildAutoToolDlg::OnShowFileInfo)
ON_EN_KILLFOCUS(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnKillfocusEditGitPath)
ON_EN_CHANGE(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnChangeEditGitPath)
ON_CBN_SELCHANGE(IDC_COMBO_BRANCH, &CQbuildAutoToolDlg::OnCbnSelchangeComboBranch)
ON_EN_CHANGE(IDC_EDIT_URL_LIST, &CQbuildAutoToolDlg::OnEnChangeEditUrlList)
ON_EN_CHANGE(IDC_EDIT_NOTE, &CQbuildAutoToolDlg::OnEnChangeEditNote)
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
	SetIcon(m_hIcon, TRUE);				// Set small icon


	///////////////////////////////设置TabControl选项框/////////////////////
	m_tabInfo.InsertItem(0, L"文件信息");
	m_tabInfo.InsertItem(1, L"日志");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabInfo);

	// 获取m_tab控件大小，并将m_tab控件的大小适当改小
	CRect	rec;
	m_tabInfo.GetClientRect(&rec);
	// 去掉选项卡标签页那一块空间
	rec.top += 20; 
	rec.bottom -= 2;
	rec.right -= 2;
	m_tabItemFileInfo.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_SHOWWINDOW | SWP_NOSIZE);
	m_tabItemLog.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_HIDEWINDOW);
	m_tabInfo.SetCurSel(0);

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
// “本地git库存放路径”发生变化
void CQbuildAutoToolDlg::OnEnChangeEditGitPath()
{
	m_fEditGitPathChange = TRUE;
	m_fISStartGit = TRUE;
}
// 获取“本地git库存放路径”的分支并添加到下拉框中
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    if (!m_fEditGitPathChange)
        return;
    m_fEditGitPathChange = FALSE;

    //消除下拉框现有所有内容
    ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->ResetContent();

    // 判断文件夹是否为git库文件
    CString strGitPath;
	CString strBranches;
	UpdateData(TRUE);

    strGitPath = m_strGitPath + L"\\.git";
    // 该文件夹是git库文件
    if (PathFileExists(strGitPath))
    {
        // 执行命令行获得git仓库分支字符串
		strBranches = CFunction::ExeCmd(TEXT("git branch"), m_strGitPath);

        // 获取默认分支
        CString strDefaultBranch = CFunction::GetDefaultBranch(strBranches);   

        // 以换行符分割字符串
        strBranches.Remove(L'*');
        strBranches.Remove(L' ');
        std::set<CString> setStrResult = CFunction::SplitCString(strBranches, L"\n");

        //将分支消息添加下拉框内容
        std::set<CString>::iterator iter	= setStrResult.begin();
        std::set<CString>::iterator iterEnd = setStrResult.end();
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
    }
}

// 点击“浏览”按钮，实现浏览文件功能获取文件目录路径
void CQbuildAutoToolDlg::OnBnClickedBrowse()
{
	LPWSTR	szPath;     
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
	    SetDlgItemText(IDC_EDIT_GIT_PATH, szPath);     
        OnEnKillfocusEditGitPath();
    }
}


/////////////////////////////////////////分支输入框////////////////////////////////////////////////////
// 点击下拉框的某项触发该函数（实现点击某个分支将该分支拉下来）
void CQbuildAutoToolDlg::OnCbnSelchangeComboBranch()
{
    UpdateData(TRUE);
	CString szCmdLine;
	szCmdLine = L"git checkout " + m_strBranch ;
	CFunction::ExeCmd(szCmdLine, m_strGitPath);
	m_fISStartGit = TRUE;
}

/////////////////////////////////////////备注输入框////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnEnChangeEditNote()
{
	// 更新备注变量的值
	UpdateData(TRUE);
}

/////////////////////////////////////////“提测文件列表”输入框////////////////////////////////////////////////
// 该输入框发生变化
void CQbuildAutoToolDlg::OnEnChangeEditUrlList()
{
	// 解决多次点击“获取文件”按钮崩溃问题
	m_fIsStartGetFile				= FALSE;
	m_fISStartGit					= TRUE;
	m_fClear						= TRUE;
	m_tabItemLog.m_fFirstLogGitInfo = TRUE;
	m_tabItemLog.m_fFirstLogUrlInfo = TRUE;
}
// 实现“提测文件列表”全选功能
BOOL CQbuildAutoToolDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		BOOL b = GetKeyState(VK_CONTROL) & 0X80;
		if(b && (pMsg->wParam == L'a' || pMsg->wParam == L'A'))
		{
			SendDlgItemMessage(IDC_EDIT_URL_LIST, EM_SETSEL, 0, -1);
			return TRUE;
		}
	}
   
	return CWnd::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////“获取文件”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
	// 判断是否存在JSON文件
    if (!GetFileLocationFromJson())
    {
        return ;
    }

	// 判断是否需要清空“文件消息”框
	if (m_fClear)
	{
		m_fClear = FALSE;
		m_tabItemFileInfo.m_lvwFileInfo.DeleteAllItems();
		m_tabItemLog.m_strLog.Empty();
	}

	// 判断是否已经开始获取文件
	if (m_fIsStartGetFile)
		return ;
	m_fIsStartGetFile = TRUE;

    // 必须设置git仓库路径后才允许获取文件
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"请设置Git仓库路径", L"警告", MB_OK | MB_ICONERROR); 
        return;
    }

	// 获取edict控件消息
	CString strUrls;
	GetDlgItemText(IDC_EDIT_URL_LIST, strUrls);

	// 分割得到url集合
    std::set<CString>			strEachUrl	= CFunction::SplitCString(strUrls, L"\r\n");
	std::set<CString>::iterator iterUrl		= strEachUrl.begin();
	std::set<CString>::iterator iterUrlEnd	= strEachUrl.end();

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

/**************************************显示相关信息到窗口********************************************/
// 切换选项卡
void CQbuildAutoToolDlg::OnTcnSelchangeTabInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	int nCurSel;
	nCurSel = m_tabInfo.GetCurSel();
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

// 将下载状态输出到日志窗口
LRESULT CQbuildAutoToolDlg::OnLogDownloadStatus(WPARAM wParam, LPARAM lParam)
{
    CString *strUrl = (CString *)wParam;
    m_tabItemLog.ShowLogUrlInfo(*strUrl, lParam);

    delete strUrl;
    return TRUE;
}

// 成功下载文件后执行解压操作
LRESULT CQbuildAutoToolDlg::OnSuccessDownloadFile(WPARAM wParam, LPARAM lParam)
{
    CString		*pstrFileName;
    CTaskBase	*pTask;

	pstrFileName  = (CString *)wParam;
    pTask		  = new CUnzipTask(this->GetSafeHwnd(), *pstrFileName, m_strBranch, m_jvRoot);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);

    delete pstrFileName;
    return TRUE;
}

// 显示FILE_INFO结构体信息到“文件信息”输出框
LRESULT CQbuildAutoToolDlg::OnShowFileInfo(WPARAM wParam, LPARAM lParam)
{
    FILE_INFO *fileInfo = (FILE_INFO *) wParam;
    m_tabItemFileInfo.InsertFileInfo(fileInfo);

    m_setFileInfo.insert(fileInfo); 
    return TRUE;
}

// 从JSON文件读取文件配置信息
BOOL CQbuildAutoToolDlg::GetFileLocationFromJson()
{
	CString strJsonPath = m_strGitPath + L"\\config.json";
	if (PathFileExists(strJsonPath))
	{
		if (m_jvRoot.empty())
			CJsonHelper::LoadJson((LPCTSTR)strJsonPath, m_jvRoot);
		return TRUE;
	}
	else 
	{
		MessageBox(L"Git仓库目录下不存在配置文件config.json, 请先设置配置文件", L"警告", MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

// 将git执行结果显示到日志窗口
LRESULT CQbuildAutoToolDlg::OnLogGitInfo(WPARAM wParam, LPARAM)
{
	CString *strGitResult = (CString *)wParam;
	m_tabItemLog.ShowLogGitInfo(*strGitResult);

	delete strGitResult;
	return TRUE;
}

/////////////////////////////////////////“文件入库”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
    if (!GetFileLocationFromJson())
    {
        return ;
    }

	// 判断备注是否为空
	if (m_strNote.IsEmpty())
	{
		MessageBox(L"请输入备注！", L"警告", MB_ICONWARNING);
		return;
	}

	if (!m_fISStartGit)
	{
		return;
	}
	m_fISStartGit = FALSE;

    CTaskBase *pTask = new CPushTask(this->GetSafeHwnd(), m_jvRoot, m_setFileInfo, m_strGitPath, m_strBranch, m_strNote);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
}


/////////////////////////////////////////“退出”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}







