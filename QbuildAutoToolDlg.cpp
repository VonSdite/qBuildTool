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

// CQbuildAutoToolDlg dialog���캯��
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

// ��������
CQbuildAutoToolDlg::~CQbuildAutoToolDlg()
{
	std::set<FILE_INFO *>::iterator iter = m_setFileInfo.begin(); 
	for (; iter != m_setFileInfo.end();)
	{
		delete *iter;
		m_setFileInfo.erase(iter++);
	}
	// ����ʱ�ļ���ɾ��
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


	///////////////////////////////����TabControlѡ���/////////////////////
	m_tabInfo.InsertItem(0, L"�ļ���Ϣ");
	m_tabInfo.InsertItem(1, L"��־");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabInfo);

	// ��ȡm_tab�ؼ���С������m_tab�ؼ��Ĵ�С�ʵ���С
	CRect	rec;
	m_tabInfo.GetClientRect(&rec);
	// ȥ��ѡ���ǩҳ��һ��ռ�
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


// ���ڽ����Ŵ���������Ӧ
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
//	GetWindowRect(&rcWnd); // ��ô�������
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
//	// ����3��button��ť����Ӧ���ڴ�С
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


/////////////////////////////////////////������git����·���������////////////////////////////////////////////////////
// ������git����·���������仯
void CQbuildAutoToolDlg::OnEnChangeEditGitPath()
{
	m_fEditGitPathChange = TRUE;
	m_fISStartGit = TRUE;
}
// ��ȡ������git����·�����ķ�֧����ӵ���������
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    if (!m_fEditGitPathChange)
        return;
    m_fEditGitPathChange = FALSE;

    //����������������������
    ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->ResetContent();

    // �ж��ļ����Ƿ�Ϊgit���ļ�
    CString strGitPath;
	CString strBranches;
	UpdateData(TRUE);

    strGitPath = m_strGitPath + L"\\.git";
    // ���ļ�����git���ļ�
    if (PathFileExists(strGitPath))
    {
        // ִ�������л��git�ֿ��֧�ַ���
		strBranches = CFunction::ExeCmd(TEXT("git branch"), m_strGitPath);

        // ��ȡĬ�Ϸ�֧
        CString strDefaultBranch = CFunction::GetDefaultBranch(strBranches);   

        // �Ի��з��ָ��ַ���
        strBranches.Remove(L'*');
        strBranches.Remove(L' ');
        std::set<CString> setStrResult = CFunction::SplitCString(strBranches, L"\n");

        //����֧��Ϣ�������������
        std::set<CString>::iterator iter	= setStrResult.begin();
        std::set<CString>::iterator iterEnd = setStrResult.end();
        for(; iter != iterEnd; iter++)
        {
            ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->AddString(*iter);
        }

        // ��Ĭ����Ϣ���á���֧���ơ������
        m_cmbBranch.SetCurSel(m_cmbBranch.FindString(0, strDefaultBranch));
    }
    else
    {
        SetDlgItemText(IDC_EDIT_GIT_PATH, L"");
        MessageBox(L"��·������Git�ֿ�", L"����", MB_OK);
    }
}

// ������������ť��ʵ������ļ����ܻ�ȡ�ļ�Ŀ¼·��
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
    bi.lpszTitle = TEXT("��ѡ�񱾵�gitĿ¼��");   
    bi.ulFlags = 0;   
    bi.lpfn = NULL;   
    bi.lParam = 0;   
    bi.iImage = 0;   
    //����ѡ��Ŀ¼�Ի���
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);   
 
    if(lp && SHGetPathFromIDList(lp, szPath))   
    {
	    SetDlgItemText(IDC_EDIT_GIT_PATH, szPath);     
        OnEnKillfocusEditGitPath();
    }
}


/////////////////////////////////////////��֧�����////////////////////////////////////////////////////
// ����������ĳ����ú�����ʵ�ֵ��ĳ����֧���÷�֧��������
void CQbuildAutoToolDlg::OnCbnSelchangeComboBranch()
{
    UpdateData(TRUE);
	CString szCmdLine;
	szCmdLine = L"git checkout " + m_strBranch ;
	CFunction::ExeCmd(szCmdLine, m_strGitPath);
	m_fISStartGit = TRUE;
}

/////////////////////////////////////////��ע�����////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnEnChangeEditNote()
{
	// ���±�ע������ֵ
	UpdateData(TRUE);
}

/////////////////////////////////////////������ļ��б������////////////////////////////////////////////////
// ����������仯
void CQbuildAutoToolDlg::OnEnChangeEditUrlList()
{
	// �����ε������ȡ�ļ�����ť��������
	m_fIsStartGetFile				= FALSE;
	m_fISStartGit					= TRUE;
	m_fClear						= TRUE;
	m_tabItemLog.m_fFirstLogGitInfo = TRUE;
	m_tabItemLog.m_fFirstLogUrlInfo = TRUE;
}
// ʵ�֡�����ļ��б�ȫѡ����
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

/////////////////////////////////////////����ȡ�ļ�����ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
	// �ж��Ƿ����JSON�ļ�
    if (!GetFileLocationFromJson())
    {
        return ;
    }

	// �ж��Ƿ���Ҫ��ա��ļ���Ϣ����
	if (m_fClear)
	{
		m_fClear = FALSE;
		m_tabItemFileInfo.m_lvwFileInfo.DeleteAllItems();
		m_tabItemLog.m_strLog.Empty();
	}

	// �ж��Ƿ��Ѿ���ʼ��ȡ�ļ�
	if (m_fIsStartGetFile)
		return ;
	m_fIsStartGetFile = TRUE;

    // ��������git�ֿ�·����������ȡ�ļ�
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"������Git�ֿ�·��", L"����", MB_OK | MB_ICONERROR); 
        return;
    }

	// ��ȡedict�ؼ���Ϣ
	CString strUrls;
	GetDlgItemText(IDC_EDIT_URL_LIST, strUrls);

	// �ָ�õ�url����
    std::set<CString>			strEachUrl	= CFunction::SplitCString(strUrls, L"\r\n");
	std::set<CString>::iterator iterUrl		= strEachUrl.begin();
	std::set<CString>::iterator iterUrlEnd	= strEachUrl.end();

	// �ڵ�ǰ���л����½�һ����ʱ�ļ���
    CString strSavePath = L"Temp\\";
	CreateDirectory(strSavePath, NULL);

    CTaskBase *pTask = NULL;
	for (; iterUrl!= iterUrlEnd; iterUrl++)
	{
        pTask = new CDownloadTask(this->GetSafeHwnd(), *iterUrl, strSavePath);
        m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
	}
}

/**************************************��ʾ�����Ϣ������********************************************/
// �л�ѡ�
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

// ������״̬�������־����
LRESULT CQbuildAutoToolDlg::OnLogDownloadStatus(WPARAM wParam, LPARAM lParam)
{
    CString *strUrl = (CString *)wParam;
    m_tabItemLog.ShowLogUrlInfo(*strUrl, lParam);

    delete strUrl;
    return TRUE;
}

// �ɹ������ļ���ִ�н�ѹ����
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

// ��ʾFILE_INFO�ṹ����Ϣ�����ļ���Ϣ�������
LRESULT CQbuildAutoToolDlg::OnShowFileInfo(WPARAM wParam, LPARAM lParam)
{
    FILE_INFO *fileInfo = (FILE_INFO *) wParam;
    m_tabItemFileInfo.InsertFileInfo(fileInfo);

    m_setFileInfo.insert(fileInfo); 
    return TRUE;
}

// ��JSON�ļ���ȡ�ļ�������Ϣ
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
		MessageBox(L"Git�ֿ�Ŀ¼�²����������ļ�config.json, �������������ļ�", L"����", MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

// ��gitִ�н����ʾ����־����
LRESULT CQbuildAutoToolDlg::OnLogGitInfo(WPARAM wParam, LPARAM)
{
	CString *strGitResult = (CString *)wParam;
	m_tabItemLog.ShowLogGitInfo(*strGitResult);

	delete strGitResult;
	return TRUE;
}

/////////////////////////////////////////���ļ���⡱��ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
    if (!GetFileLocationFromJson())
    {
        return ;
    }

	// �жϱ�ע�Ƿ�Ϊ��
	if (m_strNote.IsEmpty())
	{
		MessageBox(L"�����뱸ע��", L"����", MB_ICONWARNING);
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


/////////////////////////////////////////���˳�����ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}







