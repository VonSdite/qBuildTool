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


// CQbuildAutoToolDlg dialog构造函数
CQbuildAutoToolDlg::CQbuildAutoToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQbuildAutoToolDlg::IDD, pParent), m_fIsEditGitPathChange(FALSE)
    , m_strBranch(_T(""))
    , m_strGitPath(_T(""))
    , m_strNote(_T(""))
	, m_strUrls(_T(""))
    , m_fCompleteDownload(TRUE)
    , m_fCanPush(TRUE)
	, m_fIsDownLoading(FALSE)
{
    _CrtSetBreakAlloc(1533);
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
    m_thrdpoolParse.Initialize(NULL, THREADPOOL_SIZE);
    m_jvRoot.clear();
}

// 析构函数
CQbuildAutoToolDlg::~CQbuildAutoToolDlg()
{
    // 清空“文件消息”框中保存的每行信息
    for (size_t i = 0; i < m_vecFileInfo.size(); ++i)
    {
        delete m_vecFileInfo[i];
    }
    m_vecFileInfo.clear();

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
	DDX_Control(pDX, IDC_BUTTON_PUSH_FILE, m_btnPushFile);
	DDX_Control(pDX, IDC_BUTTON_GET_FILE, m_btnGetFile);
	DDX_Text(pDX, IDC_EDIT_URL_LIST, m_strUrls);
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
    ON_MESSAGE(WM_COMPLETE_DOWNLOAD, &CQbuildAutoToolDlg::OnLogDownloadStatus)
	ON_MESSAGE(WM_LOG_GIT_INFO, &CQbuildAutoToolDlg::OnLogGitInfo)
    ON_MESSAGE(WM_SHOW_FILE_INFO, &CQbuildAutoToolDlg::OnShowFileInfo)
    ON_MESSAGE(WM_DOWNLOAD_FINISHED, &CQbuildAutoToolDlg::OnDownLoadFinished)
    ON_MESSAGE(WM_UPDATE_SUCCESS, &CQbuildAutoToolDlg::OnFileUpdateSuccess)
ON_EN_KILLFOCUS(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnKillfocusEditGitPath)
ON_EN_CHANGE(IDC_EDIT_GIT_PATH, &CQbuildAutoToolDlg::OnEnChangeEditGitPath)
ON_CBN_SELCHANGE(IDC_COMBO_BRANCH, &CQbuildAutoToolDlg::OnCbnSelchangeComboBranch)
ON_EN_CHANGE(IDC_EDIT_URL_LIST, &CQbuildAutoToolDlg::OnEnChangeEditUrlList)
ON_EN_CHANGE(IDC_EDIT_NOTE, &CQbuildAutoToolDlg::OnEnChangeEditNote)
ON_WM_TIMER()
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

	//  set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, FALSE);			// set big icon
	SetIcon(m_hIcon, TRUE);				// set small icon

	// 设置TabControl选项框
	m_tabInfo.InsertItem(0, L"文件信息");
	m_tabInfo.InsertItem(1, L"日志");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabInfo);

	// 设置m_tab控件的大小
	CRect	rec;
	m_tabInfo.GetClientRect(&rec);
	// 去掉选项卡标签页那一块空间
	rec.top += 20; 
	rec.bottom -= 2;
	rec.right -= 2;
	m_tabItemFileInfo.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_SHOWWINDOW | SWP_NOSIZE);
	m_tabItemLog.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_HIDEWINDOW);
	m_tabInfo.SetCurSel(0);

	// 设置按钮不可用
	m_btnGetFile.EnableWindow(FALSE);
	m_btnPushFile.EnableWindow(FALSE);

    // 加载Loading图标
    for (size_t ID = IDI_ICON2; ID <= IDI_ICON9; ++ID)
    {
        m_hIconLoading[ID - IDI_ICON2] = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(ID));
    }

    // 设置默认焦点位置
    GetDlgItem( IDC_BUTTON_BROWSE )->SetFocus();
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

/////////////////////////////////////////“本地git库存放路径”输入框////////////////////////////////////////////////////
// “本地git库存放路径”发生变化
void CQbuildAutoToolDlg::OnEnChangeEditGitPath()
{
	m_fIsEditGitPathChange = TRUE;
}

// 获取“本地git库存放路径”的分支并添加到下拉框中
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    if (!m_fIsEditGitPathChange)
        return;
    m_fIsEditGitPathChange = FALSE;

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

        if (!m_strUrls.IsEmpty())
        {
            m_btnGetFile.EnableWindow(TRUE);
        }
    }
    else
    {
        SetDlgItemText(IDC_EDIT_GIT_PATH, L"");
        MessageBox(L"该路径不是Git仓库", WARNING, MB_OK);
        m_btnGetFile.EnableWindow(FALSE);
        m_btnPushFile.EnableWindow(FALSE);
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
	UpdateData(TRUE);

	// “获取文件”按钮的可用
    if (m_strUrls.IsEmpty())
	    m_btnGetFile.EnableWindow(FALSE);
    else if (!m_strUrls.IsEmpty() && !m_fIsDownLoading)
        m_btnGetFile.EnableWindow(TRUE);
}

// 实现“提测文件列表”全选功能
BOOL CQbuildAutoToolDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		BOOL bCtrl = GetKeyState(VK_CONTROL) & 0X80;
        BOOL bAlt = GetKeyState(VK_MENU) & 0x80;

		if(bCtrl && !bAlt && (pMsg->wParam == L'a' || pMsg->wParam == L'A'))
		{
			SendDlgItemMessage(IDC_EDIT_URL_LIST, EM_SETSEL, 0, -1);
			return TRUE;
		}
        else if (bCtrl && !bAlt && (pMsg->wParam == L'g' || pMsg->wParam == L'G'))
        {
            if (m_btnGetFile.IsWindowEnabled())
                OnBnClickedGetFile();
            return TRUE;
        }
        else if (bCtrl && !bAlt && (pMsg->wParam == L'p' || pMsg->wParam == L'P'))
        {
            if (m_btnPushFile.IsWindowEnabled())
                OnBnClickedPushFile();
            return TRUE;
        }
        else if (bCtrl && !bAlt && (pMsg->wParam == L'b' || pMsg->wParam == L'B'))
        {
            OnBnClickedBrowse();
            return TRUE;
        }
        else if (bCtrl && !bAlt && (pMsg->wParam == L'p' || pMsg->wParam == L'P'))
        {
            OnBnClickedExit();
            return TRUE;
        }
	}
   
	return CWnd::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////“获取文件”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
    // Git仓库不能为空
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"请先设置本地Git仓库路径", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_BUTTON_BROWSE)->SetFocus();
        return;
    }

    // 获取配置文件信息
    if(!GetFileLocationFromJson()) return;

    // 切换成文件信息tab
    m_tabInfo.SetCurSel(0);
    m_tabItemFileInfo.ShowWindow(TRUE);
    m_tabItemLog.ShowWindow(FALSE);

    // 图标显示为加载
    SetTimer(ID_EVENT_DOWNLOADING, TIMER_SECOND, NULL);

    // 允许入库
    m_fCanPush = TRUE;

	// 正在下载
	m_fIsDownLoading = TRUE;

    // 开始获取文件
    m_btnGetFile.EnableWindow(FALSE);
    m_btnPushFile.EnableWindow(FALSE);

    // 清除解压出来的文件历史
    CUnzipTask::mapRecordExist.clear();

    // 清空信息
	Clear();

    CTime tm; 
    tm = CTime::GetCurrentTime();
    CString strTime = tm.Format(L"%Y/%m/%d %X\r\n");
    // 日志显示下载文件
    m_tabItemLog.m_strLog += L"======下载文件====== " + strTime;

    // 标题显示正在下载
    ShowProgress(DOWNLOADING);

    // 在当前运行环境新建一个临时文件夹
    CreateDirectory(SAVE_PATH, NULL);

	// 分割得到url集合
    std::set<CString> strEachUrl = CFunction::SplitCString(m_strUrls, L"\r\n");

    // 开始执行下载任务
    CTaskBase *pTask = NULL;
    pTask = new CDownloadTask(this->GetSafeHwnd(), strEachUrl);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
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

    // 成功下载文件后进行解压操作
    if (lParam)
    {
        CString		strFileName;
        CTaskBase	*pTask;

        strFileName  = CFunction::GetNameFromUrl(*strUrl);
        pTask = new CUnzipTask(this->GetSafeHwnd(), strFileName,  m_strGitPath, m_strBranch, m_jvRoot);
        m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
    }
    else 
        m_fCompleteDownload = FALSE;

    delete strUrl;
    return TRUE;
}

// 所有url下载完成
LRESULT CQbuildAutoToolDlg::OnDownLoadFinished(WPARAM wParam, LPARAM lParam)
{
    KillTimer(ID_EVENT_DOWNLOADING);
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    if (!m_fCompleteDownload)
	{
        MessageBox(L"存在链接下载失败，请查看日志", WARNING, MB_OK|MB_ICONWARNING);

		// 切换成文件信息tab
		m_tabInfo.SetCurSel(1);
		m_tabItemFileInfo.ShowWindow(FALSE);
		m_tabItemLog.ShowWindow(TRUE);
	}
    m_fCompleteDownload = TRUE;
	m_fIsDownLoading = FALSE;

    m_btnGetFile.EnableWindow(TRUE);

	if (m_vecFileInfo.empty())
	{
		m_btnPushFile.EnableWindow(FALSE);
	}
	else
	{
		m_btnPushFile.EnableWindow(TRUE);
	}

    ShowProgress();
    return TRUE;
}

// 显示FILE_INFO结构体信息到“文件信息”输出框
LRESULT CQbuildAutoToolDlg::OnShowFileInfo(WPARAM wParam, LPARAM lParam)
{
    FILE_INFO *fileInfo = (FILE_INFO *) wParam;
    m_tabItemFileInfo.InsertFileInfo(fileInfo, lParam);
    if (lParam)
        m_fCanPush = FALSE;

    m_vecFileInfo.push_back(fileInfo); 

    return TRUE;
}

// 从JSON文件读取文件配置信息
BOOL CQbuildAutoToolDlg::GetFileLocationFromJson()
{
    // 获取程序根目录下config文件夹下config.json的路径
    TCHAR _szPath[MAX_PATH + 1]={0};
    GetModuleFileName(NULL, _szPath, MAX_PATH);
    (_tcsrchr(_szPath, _T('\\')))[1] = 0;     
    CString strJsonPath = _szPath;
    strJsonPath += L"conf\\QbuildAutoToolConfig.json";

	if (PathFileExists(strJsonPath))
	{
		CJsonHelper::LoadJson((LPCTSTR)strJsonPath, m_jvRoot);
		return TRUE;
	}
	else 
	{
		MessageBox(L"程序根目录需新建conf文件夹,\r\n并放入配置文件 \"QbuildAutoToolConfig.json\"\r\n请先设置配置文件", WARNING, MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

/////////////////////////////////////////“文件入库”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"请先设置本地Git仓库路径", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_BUTTON_BROWSE)->SetFocus();
        return;
    }

	// 判断备注是否为空
	if (m_strNote.IsEmpty())
	{
		MessageBox(L"请输入备注！", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_EDIT_NOTE)->SetFocus();
		return;
	}

    if (!m_fCanPush)
    {
        MessageBox(L"有重复的提测文件，请修改提测文件URL，并重新获取文件", MYERROR, MB_ICONERROR);
        return ;
    }

    // 图标显示为加载
    SetTimer(ID_EVENT_PUSHING, TIMER_SECOND, NULL);

    m_btnGetFile.EnableWindow(FALSE);
    m_btnPushFile.EnableWindow(FALSE);

    // 日志输出文件入库
    CTime tm; 
    tm = CTime::GetCurrentTime();
    CString strTime = tm.Format(L"%Y/%m/%d %X\r\n");
    m_tabItemLog.m_strLog += L"\r\n======文件入库====== " + strTime;

	// 提示进度
	ShowProgress(PUSHING);
    CTaskBase *pTask = new CPushTask(this->GetSafeHwnd(), m_jvRoot, m_vecFileInfo, m_strGitPath, m_strBranch, m_strNote);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
}

// 将git执行结果显示到日志窗口
LRESULT CQbuildAutoToolDlg::OnLogGitInfo(WPARAM wParam, LPARAM)
{
    KillTimer(ID_EVENT_PUSHING);
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    MessageBox(L"入库操作完成, 请查看日志", TIP, MB_OK);
	CString *strGitResult = (CString *)wParam;
	m_tabItemLog.ShowLogGitInfo(*strGitResult);

    ShowProgress();

    m_btnGetFile.EnableWindow(TRUE);
    m_btnPushFile.EnableWindow(TRUE);

    m_tabInfo.SetCurSel(1);
    m_tabItemFileInfo.ShowWindow(FALSE);
    m_tabItemLog.ShowWindow(TRUE);

    // 滚动到最底端
    int nline = m_tabItemLog.m_cbEditLog.GetLineCount();
    m_tabItemLog.m_cbEditLog.LineScroll(nline-1);

	delete strGitResult;
	return TRUE;
}

/////////////////////////////////////////“退出”按钮////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}

// 显示状态
void CQbuildAutoToolDlg::ShowProgress(const CString &strProgressStatus /*= L""*/)
{
	this->SetWindowText(L"Qbuild文件获取" + strProgressStatus);
}

void CQbuildAutoToolDlg::Clear()
{
    // 清空“文件消息”框
    m_tabItemFileInfo.m_lvwFileInfo.DeleteAllItems();
    m_tabItemLog.m_strLog.Empty();

    // 清空“文件消息”框中保存的每行信息
    for (size_t i = 0; i < m_vecFileInfo.size(); ++i)
    {
        delete m_vecFileInfo[i];
    }
    m_vecFileInfo.clear();
}

void CQbuildAutoToolDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    static DWORD dwLoadingPicIndex = 0;
    switch(nIDEvent)
    {
    case ID_EVENT_DOWNLOADING:
    case ID_EVENT_PUSHING:
        // 图标变成加载状态
        SetIcon(m_hIconLoading[dwLoadingPicIndex], TRUE);
        SetIcon(m_hIconLoading[dwLoadingPicIndex], FALSE);
        dwLoadingPicIndex = (dwLoadingPicIndex + 1) % 8;
        break;

    default:
        break;
    }

    CDialog::OnTimer(nIDEvent);
}

// 设置某行文件“是否更新”为否
LRESULT CQbuildAutoToolDlg::OnFileUpdateSuccess(WPARAM wParam, LPARAM lParam)
{
    m_tabItemFileInfo.SetFileSuccessUpdate(wParam);
    return TRUE;
}
