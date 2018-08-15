// CFileInfoTab.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "FileInfoTabItem.h"


// CFileInfoTab dialog
IMPLEMENT_DYNAMIC(CFileInfoTabItem, CDialog)

CFileInfoTabItem::CFileInfoTabItem(CWnd* pParent /*=NULL*/)
	: CDialog(CFileInfoTabItem::IDD, pParent), m_dwRow(0)
{

}

CFileInfoTabItem::~CFileInfoTabItem()
{
}

void CFileInfoTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_lvwFileInfo);
}


BEGIN_MESSAGE_MAP(CFileInfoTabItem, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CFileInfoTabItem::OnRclickList2)
	ON_COMMAND(ID_MENU1, &CFileInfoTabItem::OnCopyMenu)
    ON_MESSAGE(WM_COPY_INFO, &CFileInfoTabItem::OnCopy)
//    ON_NOTIFY(LVN_HOTTRACK, IDC_LIST2, &CFileInfoTabItem::OnLvnHotTrackList2)
//ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_LIST2, &CFileInfoTabItem::OnNMReleasedcaptureList2)
END_MESSAGE_MAP()


// CFileInfoTab message handlers
HBRUSH CFileInfoTabItem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));
	return hbr;
}


BOOL CFileInfoTabItem::OnInitDialog()
{
	CDialog::OnInitDialog();

	//////////////////////////////显示结果//////////////////////////////////
	// 设置网格
	DWORD   dwStyle   =   m_lvwFileInfo.GetExtendedStyle(); 
	dwStyle   |=   LVS_EX_GRIDLINES;				//网格线（只适用与report风格的listctrl） 
	dwStyle	  |=   LVS_EX_FULLROWSELECT;			//选中某行使整行高亮
	m_lvwFileInfo.SetExtendedStyle(dwStyle);		//设置扩展风格

	//插入列
	m_lvwFileInfo.InsertColumn( 0, L"文件名", LVCFMT_LEFT, 120 );
	m_lvwFileInfo.InsertColumn( 1, L"MD5", LVCFMT_LEFT, 200 );
	m_lvwFileInfo.InsertColumn( 2, L"版本", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 3, L"大小（字节）", LVCFMT_LEFT, 90 );
	m_lvwFileInfo.InsertColumn( 4, L"签名时间", LVCFMT_LEFT, 120 );
	m_lvwFileInfo.InsertColumn( 5, L"是否有更新", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 6, L"对应存放位置", LVCFMT_LEFT, 1920 );


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CFileInfoTabItem::InsertFileInfo(FILE_INFO *fileInfo)
{
    CString strSize;
    strSize.Format(L"%d B", fileInfo->dwSize);

    int nRow = m_lvwFileInfo.InsertItem(m_dwRow++, fileInfo->strFileName);
    //设置数据
    m_lvwFileInfo.SetItemText(nRow, 1, fileInfo->strMd5);
    m_lvwFileInfo.SetItemText(nRow, 2, fileInfo->strVersion);
    m_lvwFileInfo.SetItemText(nRow, 3, strSize);
    m_lvwFileInfo.SetItemText(nRow, 4, fileInfo->strSignTime);
    m_lvwFileInfo.SetItemText(nRow, 5, fileInfo->fUpdate?L"是":L"否");
    m_lvwFileInfo.SetItemText(nRow, 6, (fileInfo->strFileLocation.IsEmpty())?L"未在配置文件中设置":fileInfo->strFileLocation);
}

/////////////////////////////////////////“文件信息”输出框////////////////////////////////////////////////////
// 右键弹出复制菜单
void CFileInfoTabItem::OnRclickList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU_COPY));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x, point.y, this);
	}
	*pResult = 0;
}

// 实现复制功能
void CFileInfoTabItem::OnCopyMenu()
{
    // TODO: Add your command handler code here
    CString strText;
    POSITION ps;
    UINT nIndex;
    UINT nListCtrlColumnCount;
    nListCtrlColumnCount = m_lvwFileInfo.GetHeaderCtrl()->GetItemCount();

    ps = m_lvwFileInfo.GetFirstSelectedItemPosition();
    if (NULL == ps)
    {
        return ;
    }

    while (ps)
    {
        nIndex = m_lvwFileInfo.GetNextSelectedItem(ps);
        for (UINT j = 0; j < nListCtrlColumnCount; ++j)
        {
            strText = strText + m_lvwFileInfo.GetItemText(nIndex, j) + L" ";
        }
        if (ps) strText += L"\r\n";
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
                wcscpy( pszData, strText );
                GlobalUnlock( hClipboardData );
                SetClipboardData( CF_UNICODETEXT, hClipboardData);//根据相应的数据选择第一个参数，（CF_TEXT）  
            }
            CloseClipboard();
        }
    }
}

BOOL CFileInfoTabItem::PreTranslateMessage(MSG* pMsg)
{
    // TODO: 在此添加专用代码和/或调用基类
    if(pMsg->message == WM_KEYDOWN)
    {
        BOOL b = GetKeyState(VK_CONTROL) & 0X80;
        if(b && (pMsg->wParam == L'c' || pMsg->wParam == L'C'))
        {
            SendMessage(WM_COPY_INFO, NULL, NULL);
            return TRUE;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

HRESULT CFileInfoTabItem::OnCopy(WPARAM wParam, LPARAM lParam)
{
    OnCopyMenu();
    return TRUE;
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

//void CFileInfoTabItem::OnLvnHotTrackList2(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    //LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//    //// TODO: 在此添加控件通知处理程序代码
//    //*pResult = 0;
//
//    //if (!m_fLButtonDown) return ;
//
//    //DWORD dwPos = GetMessagePos();
//    //CPoint point( LOWORD(dwPos), HIWORD(dwPos) );
//
//    //m_lvwFileInfo.ScreenToClient(&point);
//
//    //LVHITTESTINFO lvinfo;
//    //lvinfo.pt = point;
//    //lvinfo.flags = LVHT_ABOVE;
//
//    //UINT nFlag;
//    //int nItem = m_lvwFileInfo.HitTest(point, &nFlag);
//
//    //m_lvwFileInfo.SetItemState(nItem, LVIS_SELECTED,  LVIS_SELECTED);
//}
