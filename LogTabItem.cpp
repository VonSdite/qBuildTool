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
	, m_fFirstLogUrlInfo(TRUE)
	, m_fFirstLogGitInfo(TRUE)
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
	if (m_fFirstLogUrlInfo)
	{
		m_strLog += L"1. 下载文件\r\n";
		UpdateData(FALSE);
		m_fFirstLogUrlInfo = FALSE;
	}
	if (fSuccess)
    {
        m_strLog += strUrl + L" --> 下载成功\r\n";
        UpdateData(FALSE);
    }
    else 
    {
        m_strLog += strUrl + L" --> 下载失败\r\n";
        UpdateData(FALSE);
    }
}

void CLogTabItem::ShowLogGitInfo(CString strGitResult)
{
	if (m_fFirstLogGitInfo)
	{
		m_strLog += L"\r\n2. 文件入库\r\n";
		UpdateData(FALSE);
		m_fFirstLogGitInfo = FALSE;
	}
	strGitResult.Replace(L"\n", L"\r\n");
	m_strLog += strGitResult;
	UpdateData(FALSE);
}
