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
	//////////////////////////////显示结果//////////////////////////////////
	//// 设置网格
	DWORD   dwStyle   =   m_listCtrlFileInfo.GetExtendedStyle(); 
	dwStyle   |=   LVS_EX_GRIDLINES;				//网格线（只适用与report风格的listctrl） 
	dwStyle	  |=   LVS_EX_FULLROWSELECT;			//选中某行使整行高亮
	m_listCtrlFileInfo.SetExtendedStyle(dwStyle);   //设置扩展风格


	//插入列
	m_listCtrlFileInfo.InsertColumn( 0, L"文件名", LVCFMT_LEFT, 120 );
	m_listCtrlFileInfo.InsertColumn( 1, L"MD5", LVCFMT_LEFT, 200 );
	m_listCtrlFileInfo.InsertColumn( 2, L"版本", LVCFMT_LEFT, 80 );
	m_listCtrlFileInfo.InsertColumn( 3, L"大小（字节）", LVCFMT_LEFT, 90 );
	m_listCtrlFileInfo.InsertColumn( 4, L"签名时间", LVCFMT_LEFT, 120 );
	m_listCtrlFileInfo.InsertColumn( 5, L"是否有更新", LVCFMT_LEFT, 80 );
	m_listCtrlFileInfo.InsertColumn( 6, L"对应存放位置", LVCFMT_LEFT, 1920 );

	//插入行
	int nRow = m_listCtrlFileInfo.InsertItem(0, L"360BAEClient.exe");
	//设置数据
	m_listCtrlFileInfo.SetItemText(nRow, 1, L"BBF2456792346563FGHSGSHERH");
	m_listCtrlFileInfo.SetItemText(nRow, 2, L"6.3.0.1001");
	m_listCtrlFileInfo.SetItemText(nRow, 3, L"364640");
	m_listCtrlFileInfo.SetItemText(nRow, 4, L"2018-07-23 16:30:22");
	m_listCtrlFileInfo.SetItemText(nRow, 5, L"是");
	m_listCtrlFileInfo.SetItemText(nRow, 6, L"baseclient\\360BAEClient.exe;baseclient\\360BAEClient.exe");

	nRow = m_listCtrlFileInfo.InsertItem(1, L"11360BAEClient.exe");
	//设置数据
	m_listCtrlFileInfo.SetItemText(nRow, 1, L"BBF2456792346563FGHSGSHERH");
	m_listCtrlFileInfo.SetItemText(nRow, 2, L"6.3.0.1001");
	m_listCtrlFileInfo.SetItemText(nRow, 3, L"364640");
	m_listCtrlFileInfo.SetItemText(nRow, 4, L"2018-07-23 16:30:22");
	m_listCtrlFileInfo.SetItemText(nRow, 5, L"是");
	m_listCtrlFileInfo.SetItemText(nRow, 6, L"baseclient\\360BAEClient.exe;baseclient\\360BAEClient.exe");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////“文件信息”输出框////////////////////////////////////////////////////
// 右键弹出复制菜单
void CFileInfoTab::OnRclickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	// TODO: 在此添加控件通知处理程序代码
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

// 实现复制功能
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

	//复制剪切板
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
				SetClipboardData( CF_UNICODETEXT, hClipboardData);//根据相应的数据选择第一个参数，（CF_TEXT）  
			}
			CloseClipboard();
		}
	}
}

//// 窗口界面框放大后界面自适应
//void CFileInfoTab::OnSize(UINT nType, int cx, int cy)
//{
//	CDialog::OnSize(nType, cx, cy);
//
//	// TODO: Add your message handler code here
//	CRect rcWnd;
//	GetWindowRect(&rcWnd); // 获得窗口坐标
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
