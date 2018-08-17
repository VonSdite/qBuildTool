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


// CQbuildAutoToolDlg dialog���캯��
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

// ��������
CQbuildAutoToolDlg::~CQbuildAutoToolDlg()
{
    // ��ա��ļ���Ϣ�����б����ÿ����Ϣ
    for (size_t i = 0; i < m_vecFileInfo.size(); ++i)
    {
        delete m_vecFileInfo[i];
    }
    m_vecFileInfo.clear();

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

	// ����TabControlѡ���
	m_tabInfo.InsertItem(0, L"�ļ���Ϣ");
	m_tabInfo.InsertItem(1, L"��־");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabInfo);

	// ����m_tab�ؼ��Ĵ�С
	CRect	rec;
	m_tabInfo.GetClientRect(&rec);
	// ȥ��ѡ���ǩҳ��һ��ռ�
	rec.top += 20; 
	rec.bottom -= 2;
	rec.right -= 2;
	m_tabItemFileInfo.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_SHOWWINDOW | SWP_NOSIZE);
	m_tabItemLog.SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_HIDEWINDOW);
	m_tabInfo.SetCurSel(0);

	// ���ð�ť������
	m_btnGetFile.EnableWindow(FALSE);
	m_btnPushFile.EnableWindow(FALSE);

    // ����Loadingͼ��
    for (size_t ID = IDI_ICON2; ID <= IDI_ICON9; ++ID)
    {
        m_hIconLoading[ID - IDI_ICON2] = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(ID));
    }

    // ����Ĭ�Ͻ���λ��
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

/////////////////////////////////////////������git����·���������////////////////////////////////////////////////////
// ������git����·���������仯
void CQbuildAutoToolDlg::OnEnChangeEditGitPath()
{
	m_fIsEditGitPathChange = TRUE;
}

// ��ȡ������git����·�����ķ�֧����ӵ���������
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    if (!m_fIsEditGitPathChange)
        return;
    m_fIsEditGitPathChange = FALSE;

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

        if (!m_strUrls.IsEmpty())
        {
            m_btnGetFile.EnableWindow(TRUE);
        }
    }
    else
    {
        SetDlgItemText(IDC_EDIT_GIT_PATH, L"");
        MessageBox(L"��·������Git�ֿ�", WARNING, MB_OK);
        m_btnGetFile.EnableWindow(FALSE);
        m_btnPushFile.EnableWindow(FALSE);
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
	UpdateData(TRUE);

	// ����ȡ�ļ�����ť�Ŀ���
    if (m_strUrls.IsEmpty())
	    m_btnGetFile.EnableWindow(FALSE);
    else if (!m_strUrls.IsEmpty() && !m_fIsDownLoading)
        m_btnGetFile.EnableWindow(TRUE);
}

// ʵ�֡�����ļ��б�ȫѡ����
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

/////////////////////////////////////////����ȡ�ļ�����ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
    // Git�ֿⲻ��Ϊ��
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"�������ñ���Git�ֿ�·��", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_BUTTON_BROWSE)->SetFocus();
        return;
    }

    // ��ȡ�����ļ���Ϣ
    if(!GetFileLocationFromJson()) return;

    // �л����ļ���Ϣtab
    m_tabInfo.SetCurSel(0);
    m_tabItemFileInfo.ShowWindow(TRUE);
    m_tabItemLog.ShowWindow(FALSE);

    // ͼ����ʾΪ����
    SetTimer(ID_EVENT_DOWNLOADING, TIMER_SECOND, NULL);

    // �������
    m_fCanPush = TRUE;

	// ��������
	m_fIsDownLoading = TRUE;

    // ��ʼ��ȡ�ļ�
    m_btnGetFile.EnableWindow(FALSE);
    m_btnPushFile.EnableWindow(FALSE);

    // �����ѹ�������ļ���ʷ
    CUnzipTask::mapRecordExist.clear();

    // �����Ϣ
	Clear();

    CTime tm; 
    tm = CTime::GetCurrentTime();
    CString strTime = tm.Format(L"%Y/%m/%d %X\r\n");
    // ��־��ʾ�����ļ�
    m_tabItemLog.m_strLog += L"======�����ļ�====== " + strTime;

    // ������ʾ��������
    ShowProgress(DOWNLOADING);

    // �ڵ�ǰ���л����½�һ����ʱ�ļ���
    CreateDirectory(SAVE_PATH, NULL);

	// �ָ�õ�url����
    std::set<CString> strEachUrl = CFunction::SplitCString(m_strUrls, L"\r\n");

    // ��ʼִ����������
    CTaskBase *pTask = NULL;
    pTask = new CDownloadTask(this->GetSafeHwnd(), strEachUrl);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
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

    // �ɹ������ļ�����н�ѹ����
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

// ����url�������
LRESULT CQbuildAutoToolDlg::OnDownLoadFinished(WPARAM wParam, LPARAM lParam)
{
    KillTimer(ID_EVENT_DOWNLOADING);
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    if (!m_fCompleteDownload)
	{
        MessageBox(L"������������ʧ�ܣ���鿴��־", WARNING, MB_OK|MB_ICONWARNING);

		// �л����ļ���Ϣtab
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

// ��ʾFILE_INFO�ṹ����Ϣ�����ļ���Ϣ�������
LRESULT CQbuildAutoToolDlg::OnShowFileInfo(WPARAM wParam, LPARAM lParam)
{
    FILE_INFO *fileInfo = (FILE_INFO *) wParam;
    m_tabItemFileInfo.InsertFileInfo(fileInfo, lParam);
    if (lParam)
        m_fCanPush = FALSE;

    m_vecFileInfo.push_back(fileInfo); 

    return TRUE;
}

// ��JSON�ļ���ȡ�ļ�������Ϣ
BOOL CQbuildAutoToolDlg::GetFileLocationFromJson()
{
    // ��ȡ�����Ŀ¼��config�ļ�����config.json��·��
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
		MessageBox(L"�����Ŀ¼���½�conf�ļ���,\r\n�����������ļ� \"QbuildAutoToolConfig.json\"\r\n�������������ļ�", WARNING, MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

/////////////////////////////////////////���ļ���⡱��ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
    if (m_strGitPath.IsEmpty())
    {
        MessageBox(L"�������ñ���Git�ֿ�·��", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_BUTTON_BROWSE)->SetFocus();
        return;
    }

	// �жϱ�ע�Ƿ�Ϊ��
	if (m_strNote.IsEmpty())
	{
		MessageBox(L"�����뱸ע��", WARNING, MB_ICONWARNING);
        GetDlgItem(IDC_EDIT_NOTE)->SetFocus();
		return;
	}

    if (!m_fCanPush)
    {
        MessageBox(L"���ظ�������ļ������޸�����ļ�URL�������»�ȡ�ļ�", MYERROR, MB_ICONERROR);
        return ;
    }

    // ͼ����ʾΪ����
    SetTimer(ID_EVENT_PUSHING, TIMER_SECOND, NULL);

    m_btnGetFile.EnableWindow(FALSE);
    m_btnPushFile.EnableWindow(FALSE);

    // ��־����ļ����
    CTime tm; 
    tm = CTime::GetCurrentTime();
    CString strTime = tm.Format(L"%Y/%m/%d %X\r\n");
    m_tabItemLog.m_strLog += L"\r\n======�ļ����====== " + strTime;

	// ��ʾ����
	ShowProgress(PUSHING);
    CTaskBase *pTask = new CPushTask(this->GetSafeHwnd(), m_jvRoot, m_vecFileInfo, m_strGitPath, m_strBranch, m_strNote);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
}

// ��gitִ�н����ʾ����־����
LRESULT CQbuildAutoToolDlg::OnLogGitInfo(WPARAM wParam, LPARAM)
{
    KillTimer(ID_EVENT_PUSHING);
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    MessageBox(L"���������, ��鿴��־", TIP, MB_OK);
	CString *strGitResult = (CString *)wParam;
	m_tabItemLog.ShowLogGitInfo(*strGitResult);

    ShowProgress();

    m_btnGetFile.EnableWindow(TRUE);
    m_btnPushFile.EnableWindow(TRUE);

    m_tabInfo.SetCurSel(1);
    m_tabItemFileInfo.ShowWindow(FALSE);
    m_tabItemLog.ShowWindow(TRUE);

    // ��������׶�
    int nline = m_tabItemLog.m_cbEditLog.GetLineCount();
    m_tabItemLog.m_cbEditLog.LineScroll(nline-1);

	delete strGitResult;
	return TRUE;
}

/////////////////////////////////////////���˳�����ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}

// ��ʾ״̬
void CQbuildAutoToolDlg::ShowProgress(const CString &strProgressStatus /*= L""*/)
{
	this->SetWindowText(L"Qbuild�ļ���ȡ" + strProgressStatus);
}

void CQbuildAutoToolDlg::Clear()
{
    // ��ա��ļ���Ϣ����
    m_tabItemFileInfo.m_lvwFileInfo.DeleteAllItems();
    m_tabItemLog.m_strLog.Empty();

    // ��ա��ļ���Ϣ�����б����ÿ����Ϣ
    for (size_t i = 0; i < m_vecFileInfo.size(); ++i)
    {
        delete m_vecFileInfo[i];
    }
    m_vecFileInfo.clear();
}

void CQbuildAutoToolDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    static DWORD dwLoadingPicIndex = 0;
    switch(nIDEvent)
    {
    case ID_EVENT_DOWNLOADING:
    case ID_EVENT_PUSHING:
        // ͼ���ɼ���״̬
        SetIcon(m_hIconLoading[dwLoadingPicIndex], TRUE);
        SetIcon(m_hIconLoading[dwLoadingPicIndex], FALSE);
        dwLoadingPicIndex = (dwLoadingPicIndex + 1) % 8;
        break;

    default:
        break;
    }

    CDialog::OnTimer(nIDEvent);
}

// ����ĳ���ļ����Ƿ���¡�Ϊ��
LRESULT CQbuildAutoToolDlg::OnFileUpdateSuccess(WPARAM wParam, LPARAM lParam)
{
    m_tabItemFileInfo.SetFileSuccessUpdate(wParam);
    return TRUE;
}
