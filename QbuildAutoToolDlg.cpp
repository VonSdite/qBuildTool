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


	///////////////////////////////����TabControlѡ���/////////////////////
	m_tabFileInfo.InsertItem(0, L"�ļ���Ϣ");
	m_tabFileInfo.InsertItem(1, L"��־");
	m_tabItemFileInfo.Create(IDD_FILE_INFO, &m_tabFileInfo);
	m_tabItemLog.Create(IDD_LOG, &m_tabFileInfo);

	// ��ȡm_tab�ؼ���С������m_tab�ؼ��Ĵ�С�ʵ���С
	CRect	rec;
	m_tabFileInfo.GetClientRect(&rec);
	rec.top += 20; // ȥ��ѡ���ǩҳ��һ��ռ�
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

// ������git����·������������仯��ִ�иú����������жϸ�·���Ƿ�Ϊgit���ļ��Լ���ȡ��·���µķ�֧����ӵ���������
void CQbuildAutoToolDlg::OnEnKillfocusEditGitPath()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������

    if (!isEditGitPathChange)
        return;
    isEditGitPathChange = FALSE;

    //����������������������
    ((CComboBox*)GetDlgItem(IDC_COMBO_BRANCH))->ResetContent();

    // �ж��ļ����Ƿ�Ϊgit���ļ�
    CString strGitPath, strPath, strBranches;
    GetDlgItemText(IDC_EDIT_GIT_PATH, strPath);
    
    strGitPath = strPath + L"\\.git";

    // ���ļ�����git���ļ�
    if (PathFileExists(strGitPath))
    {
        // ִ�������л��git�ֿ��֧�ַ���
		strBranches = CFunction::ExeCmd(TEXT("git branch"), strPath);

        // ��ȡĬ�Ϸ�֧
        CString strDefaultBranch = CFunction::GetDefaultBranch(strBranches);   

        // �Ի��з��ָ��ַ���
        strBranches.Remove(L'*');
        strBranches.Remove(L' ');
        std::set<CString> setCStrResult = CFunction::SplitCString(strBranches, L"\n");

        //����֧��Ϣ�������������
        std::set<CString>::iterator iter = setCStrResult.begin();
        std::set<CString>::iterator iterEnd = setCStrResult.end();
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
        return;
    }
}


// ������������ť��ʵ������ļ����ܻ�ȡ�ļ�Ŀ¼·��
void CQbuildAutoToolDlg::OnBnClickedBrowse()
{
	LPWSTR szPath=NULL;     //���ѡ���Ŀ¼·�� 
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
		SetDlgItemText(IDC_EDIT_GIT_PATH,szPath);     
        OnEnKillfocusEditGitPath();
    }
}


/////////////////////////////////////////��֧�����////////////////////////////////////////////////////
// ����������ĳ����ú�����ʵ�ֵ��ĳ����֧���÷�֧��������
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

/////////////////////////////////////////����ȡ�ļ�����ť////////////////////////////////////////////////////
#include <utility>
void CQbuildAutoToolDlg::OnBnClickedGetFile()
{
	// ��ȡedict�ؼ���Ϣ
	CString strUrls;
	GetDlgItemText(IDC_EDIT_URL_LIST, strUrls);

	// �ָ�õ�url����
    std::set<CString> strEachUrl = CFunction::SplitCString(strUrls, L"\r\n");
	std::set<CString>::iterator iterUrl = strEachUrl.begin();
	std::set<CString>::iterator iterUrlEnd = strEachUrl.end();

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

LRESULT CQbuildAutoToolDlg::OnSuccessDownloadFile(WPARAM wParam, LPARAM lParam)
{
    CString *pstrFileName = (CString *)wParam;
    CTaskBase *pTask = NULL;
    pTask = new CUnzipTask(this->GetSafeHwnd(), *pstrFileName);
    m_thrdpoolParse.QueueRequest((CParseWorker::RequestType) pTask);
    delete pstrFileName;
    return TRUE;
}

/////////////////////////////////////////���ļ���⡱��ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedPushFile()
{
	// �жϱ�ע�Ƿ�Ϊ��
	CString strNote;
	GetDlgItemText(IDC_EDIT_NOTE, strNote);
	if (strNote.IsEmpty())
	{
		MessageBox(L"�����뱸ע��");
		return;
	}

	this->SendLogInfo(strNote);

}


/////////////////////////////////////////���˳�����ť////////////////////////////////////////////////////
void CQbuildAutoToolDlg::OnBnClickedExit()
{
	EndDialog(IDCLOSE);
}

// ���ѡ�
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
    // TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
    // ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
    // ���������� CRichEditCtrl().SetEventMask()��
    // ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    isEditGitPathChange = TRUE;
}

void CQbuildAutoToolDlg::SendLogInfo(CString strLogInfo)
{
	m_tabItemLog.m_cbEditLog.SetWindowText(strLogInfo);
}
