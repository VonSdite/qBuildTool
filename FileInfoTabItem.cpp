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
ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST2, &CFileInfoTabItem::OnNMCustomdrawList2)
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
	m_lvwFileInfo.InsertColumn( 1, L"MD5", LVCFMT_LEFT, 200);
	m_lvwFileInfo.InsertColumn( 2, L"版本", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 3, L"大小（字节）", LVCFMT_LEFT, 90 );
	m_lvwFileInfo.InsertColumn( 4, L"签名时间", LVCFMT_LEFT, 120 );
	m_lvwFileInfo.InsertColumn( 5, L"是否有更新", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 6, L"对应存放位置", LVCFMT_LEFT, 250 );

    m_lvwFileInfo.GetHeaderCtrl()->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CFileInfoTabItem::InsertFileInfo(FILE_INFO *fileInfo, BOOL fhadAppeared)
{
    CString strSize;
    strSize.Format(L"%d B", fileInfo->dwSize);

    int nRow = m_lvwFileInfo.InsertItem(m_dwRow++, fileInfo->strFileName);

    if (fhadAppeared)
        m_lvwFileInfo.SetItemData(nRow, COLOR_RED);
    //设置数据
    m_lvwFileInfo.SetItemText(nRow, 1, fileInfo->strMd5);
    m_lvwFileInfo.SetItemText(nRow, 2, fileInfo->strVersion);
    m_lvwFileInfo.SetItemText(nRow, 3, strSize);
    m_lvwFileInfo.SetItemText(nRow, 4, fileInfo->strSignTime);
    m_lvwFileInfo.SetItemText(nRow, 5, fileInfo->fUpdate?L"是":L"否");
    m_lvwFileInfo.SetItemText(nRow, 6, (fileInfo->strFileLocation.IsEmpty())?L"未在配置文件中设置":fileInfo->strFileLocation);

    AutoAdjustColumnWidth(&m_lvwFileInfo);
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
		if (pMsg->wParam == VK_ESCAPE)	return TRUE;
		if (pMsg->wParam == VK_RETURN)	return TRUE;
    }
	
    return CDialog::PreTranslateMessage(pMsg);
}

HRESULT CFileInfoTabItem::OnCopy(WPARAM wParam, LPARAM lParam)
{
    OnCopyMenu();
    return TRUE;
}


void CFileInfoTabItem::AutoAdjustColumnWidth(CListCtrl *pListCtrl)
{
    pListCtrl->SetRedraw(FALSE);
    CHeaderCtrl *pHeader = pListCtrl->GetHeaderCtrl();
    int nColumnCount = pHeader->GetItemCount();


    for(int i = 0; i < nColumnCount; i++)
    {
        pListCtrl->SetColumnWidth(i, LVSCW_AUTOSIZE);
        int nColumnWidth = pListCtrl->GetColumnWidth(i);
        pListCtrl->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
        int nHeaderWidth = pListCtrl->GetColumnWidth(i);


        pListCtrl->SetColumnWidth(i, max(nColumnWidth, nHeaderWidth)+5);
    }
    pListCtrl->SetRedraw(TRUE);
}


void CFileInfoTabItem::OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);

    // TODO: 在此添加控件通知处理程序代码
    NMCUSTOMDRAW nmCustomDraw = pNMCD->nmcd;
    switch(nmCustomDraw.dwDrawStage)
    {
    case CDDS_ITEMPREPAINT:
        {
            if (COLOR_RED == nmCustomDraw.lItemlParam)
            {
                pNMCD->clrTextBk = RGB(255, 0, 0);		    //背景颜色
                pNMCD->clrText = RGB(255, 255, 255);		//文字颜色
            }
           
            break;
        }
    default:
            break;	
    }

    *pResult = 0;
    *pResult |= CDRF_NOTIFYPOSTPAINT;		//必须有，不然就没有效果
    *pResult |= CDRF_NOTIFYITEMDRAW;		//必须有，不然就没有效果
}

void CFileInfoTabItem::SetFileSuccessUpdate(DWORD dwIndex)
{
    m_lvwFileInfo.SetItemText(dwIndex, 5, L"否");
}
