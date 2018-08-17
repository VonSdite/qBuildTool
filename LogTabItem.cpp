// CLogTab.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "LogTabItem.h"


// CLogTab dialog

IMPLEMENT_DYNAMIC(CLogTabItem, CDialog)

CLogTabItem::CLogTabItem(CWnd* pParent /*=NULL*/)
	: CDialog(CLogTabItem::IDD, pParent)
	, m_strLog(_T(""))
{

}

CLogTabItem::~CLogTabItem()
{
}

void CLogTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LOG, m_strLog);
	DDX_Control(pDX, IDC_EDIT_LOG, m_cbEditLog);
}

BEGIN_MESSAGE_MAP(CLogTabItem, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CLogTab message handlers
HBRUSH CLogTabItem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CLogTabItem::ShowLogUrlInfo(CString strUrl, BOOL fSuccess)
{
	if (fSuccess)
    {
        m_strLog += strUrl + L" --> 下载成功\r\n";
    }
    else 
    {
        m_strLog += strUrl + L" --> 下载失败\r\n";
    }
    UpdateData(FALSE);
}

void CLogTabItem::ShowLogGitInfo(CString strGitResult)
{
	strGitResult.Replace(L"\n", L"\r\n");
	m_strLog += strGitResult;
	UpdateData(FALSE);
}



BOOL CLogTabItem::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}
