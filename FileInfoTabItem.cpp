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

	//////////////////////////////��ʾ���//////////////////////////////////
	// ��������
	DWORD   dwStyle   =   m_lvwFileInfo.GetExtendedStyle(); 
	dwStyle   |=   LVS_EX_GRIDLINES;				//�����ߣ�ֻ������report����listctrl�� 
	dwStyle	  |=   LVS_EX_FULLROWSELECT;			//ѡ��ĳ��ʹ���и���
	m_lvwFileInfo.SetExtendedStyle(dwStyle);		//������չ���

	//������
	m_lvwFileInfo.InsertColumn( 0, L"�ļ���", LVCFMT_LEFT, 120 );
	m_lvwFileInfo.InsertColumn( 1, L"MD5", LVCFMT_LEFT, 200);
	m_lvwFileInfo.InsertColumn( 2, L"�汾", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 3, L"��С���ֽڣ�", LVCFMT_LEFT, 90 );
	m_lvwFileInfo.InsertColumn( 4, L"ǩ��ʱ��", LVCFMT_LEFT, 120 );
	m_lvwFileInfo.InsertColumn( 5, L"�Ƿ��и���", LVCFMT_LEFT, 80 );
	m_lvwFileInfo.InsertColumn( 6, L"��Ӧ���λ��", LVCFMT_LEFT, 250 );

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
    //��������
    m_lvwFileInfo.SetItemText(nRow, 1, fileInfo->strMd5);
    m_lvwFileInfo.SetItemText(nRow, 2, fileInfo->strVersion);
    m_lvwFileInfo.SetItemText(nRow, 3, strSize);
    m_lvwFileInfo.SetItemText(nRow, 4, fileInfo->strSignTime);
    m_lvwFileInfo.SetItemText(nRow, 5, fileInfo->fUpdate?L"��":L"��");
    m_lvwFileInfo.SetItemText(nRow, 6, (fileInfo->strFileLocation.IsEmpty())?L"δ�������ļ�������":fileInfo->strFileLocation);

    AutoAdjustColumnWidth(&m_lvwFileInfo);
}

/////////////////////////////////////////���ļ���Ϣ�������////////////////////////////////////////////////////
// �Ҽ��������Ʋ˵�
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

// ʵ�ָ��ƹ���
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
                wcscpy( pszData, strText );
                GlobalUnlock( hClipboardData );
                SetClipboardData( CF_UNICODETEXT, hClipboardData);//������Ӧ������ѡ���һ����������CF_TEXT��  
            }
            CloseClipboard();
        }
    }
}

BOOL CFileInfoTabItem::PreTranslateMessage(MSG* pMsg)
{
    // TODO: �ڴ����ר�ô����/����û���
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

    // TODO: �ڴ���ӿؼ�֪ͨ����������
    NMCUSTOMDRAW nmCustomDraw = pNMCD->nmcd;
    switch(nmCustomDraw.dwDrawStage)
    {
    case CDDS_ITEMPREPAINT:
        {
            if (COLOR_RED == nmCustomDraw.lItemlParam)
            {
                pNMCD->clrTextBk = RGB(255, 0, 0);		    //������ɫ
                pNMCD->clrText = RGB(255, 255, 255);		//������ɫ
            }
           
            break;
        }
    default:
            break;	
    }

    *pResult = 0;
    *pResult |= CDRF_NOTIFYPOSTPAINT;		//�����У���Ȼ��û��Ч��
    *pResult |= CDRF_NOTIFYITEMDRAW;		//�����У���Ȼ��û��Ч��
}

void CFileInfoTabItem::SetFileSuccessUpdate(DWORD dwIndex)
{
    m_lvwFileInfo.SetItemText(dwIndex, 5, L"��");
}
