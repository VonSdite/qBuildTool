// CFileInfoTab.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "FileInfoTab.h"


// CFileInfoTab dialog

IMPLEMENT_DYNAMIC(CFileInfoTab, CDialog)

CFileInfoTab::CFileInfoTab(CWnd* pParent /*=NULL*/)
	: CDialog(CFileInfoTab::IDD, pParent)
{

}

CFileInfoTab::~CFileInfoTab()
{
}

void CFileInfoTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_listCtrlFileInfo);

	//DDX_Control(pDX, IDC_STATIC3, m_fileinfo);
}


BEGIN_MESSAGE_MAP(CFileInfoTab, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CFileInfoTab::OnRclickList2)
	ON_COMMAND(ID_Menu1, &CFileInfoTab::OnCopyMenu)
END_MESSAGE_MAP()


// CFileInfoTab message handlers

HBRUSH CFileInfoTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


BOOL CFileInfoTab::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//////////////////////////////��ʾ���//////////////////////////////////
	//// ��������
	DWORD   dwStyle   =   m_listCtrlFileInfo.GetExtendedStyle(); 
	dwStyle   |=   LVS_EX_GRIDLINES;				//�����ߣ�ֻ������report����listctrl�� 
	dwStyle	  |=   LVS_EX_FULLROWSELECT;			//ѡ��ĳ��ʹ���и���
	m_listCtrlFileInfo.SetExtendedStyle(dwStyle);   //������չ���


	//������
	m_listCtrlFileInfo.InsertColumn( 0, L"�ļ���", LVCFMT_LEFT, 120 );
	m_listCtrlFileInfo.InsertColumn( 1, L"MD5", LVCFMT_LEFT, 200 );
	m_listCtrlFileInfo.InsertColumn( 2, L"�汾", LVCFMT_LEFT, 80 );
	m_listCtrlFileInfo.InsertColumn( 3, L"��С���ֽڣ�", LVCFMT_LEFT, 90 );
	m_listCtrlFileInfo.InsertColumn( 4, L"ǩ��ʱ��", LVCFMT_LEFT, 120 );
	m_listCtrlFileInfo.InsertColumn( 5, L"�Ƿ��и���", LVCFMT_LEFT, 80 );
	m_listCtrlFileInfo.InsertColumn( 6, L"��Ӧ���λ��", LVCFMT_LEFT, 1920 );

	//������
	int nRow = m_listCtrlFileInfo.InsertItem(0, L"360BAEClient.exe");
	//��������
	m_listCtrlFileInfo.SetItemText(nRow, 1, L"BBF2456792346563FGHSGSHERH");
	m_listCtrlFileInfo.SetItemText(nRow, 2, L"6.3.0.1001");
	m_listCtrlFileInfo.SetItemText(nRow, 3, L"364640");
	m_listCtrlFileInfo.SetItemText(nRow, 4, L"2018-07-23 16:30:22");
	m_listCtrlFileInfo.SetItemText(nRow, 5, L"��");
	m_listCtrlFileInfo.SetItemText(nRow, 6, L"baseclient\\360BAEClient.exe;baseclient\\360BAEClient.exe");

	nRow = m_listCtrlFileInfo.InsertItem(1, L"11360BAEClient.exe");
	//��������
	m_listCtrlFileInfo.SetItemText(nRow, 1, L"BBF2456792346563FGHSGSHERH");
	m_listCtrlFileInfo.SetItemText(nRow, 2, L"6.3.0.1001");
	m_listCtrlFileInfo.SetItemText(nRow, 3, L"364640");
	m_listCtrlFileInfo.SetItemText(nRow, 4, L"2018-07-23 16:30:22");
	m_listCtrlFileInfo.SetItemText(nRow, 5, L"��");
	m_listCtrlFileInfo.SetItemText(nRow, 6, L"baseclient\\360BAEClient.exe;baseclient\\360BAEClient.exe");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////���ļ���Ϣ�������////////////////////////////////////////////////////
// �Ҽ��������Ʋ˵�
void CFileInfoTab::OnRclickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU1));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			point.x, point.y, this);
	}
	*pResult = 0;
}

// ʵ�ָ��ƹ���
void CFileInfoTab::OnCopyMenu()
{
	// TODO: Add your command handler code here
	POSITION ps;
	int nIndex;
	ps = m_listCtrlFileInfo.GetFirstSelectedItemPosition();
	nIndex = m_listCtrlFileInfo.GetNextSelectedItem(ps);

	CString strText;

	int nListCtrlColumnCount = m_listCtrlFileInfo.GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < nListCtrlColumnCount; ++i)
	{
		strText = strText + m_listCtrlFileInfo.GetItemText(nIndex, i) + L" ";
	}

	//���Ƽ��а�
	if( !strText.IsEmpty() )
	{
		if( OpenClipboard() )
		{
			EmptyClipboard();
			TCHAR* pszData;
			HGLOBAL hClipboardData = GlobalAlloc( GMEM_DDESHARE, ( strText.GetLength() + 1) * sizeof(TCHAR) );
			if( hClipboardData )
			{
				pszData = ( TCHAR* )GlobalLock( hClipboardData );
				_tcscpy( pszData, strText );
				GlobalUnlock( hClipboardData );
				SetClipboardData( CF_UNICODETEXT, hClipboardData);//������Ӧ������ѡ���һ����������CF_TEXT��  
			}
			CloseClipboard();
		}
	}
}

//// ���ڽ����Ŵ���������Ӧ
//void CFileInfoTab::OnSize(UINT nType, int cx, int cy)
//{
//	CDialog::OnSize(nType, cx, cy);
//
//	// TODO: Add your message handler code here
//	CRect rcWnd;
//	GetWindowRect(&rcWnd); // ��ô�������
//	//if (m_listCtrlFileInfo.GetSafeHwnd())
//	//{
//	//	CRect rcListCtrl0, rcListCtrl;
//	//	m_listCtrlFileInfo.GetClientRect(&rcListCtrl0);
//	//	m_listCtrlFileInfo.GetWindowRect(&rcListCtrl);
//	//	rcListCtrl.right = rcListCtrl.right * 2 ;
//	//	rcListCtrl.bottom = rcListCtrl.bottom * 2  - 180;
//	//	ScreenToClient(&rcListCtrl);
//	//	m_listCtrlFileInfo.MoveWindow(rcListCtrl, TRUE);
//	//	this->MoveWindow(rcListCtrl, TRUE);
//	//}
//}
