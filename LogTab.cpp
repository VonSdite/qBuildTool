// CLogTab.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "LogTab.h"


// CLogTab dialog

IMPLEMENT_DYNAMIC(CLogTab, CDialog)

CLogTab::CLogTab(CWnd* pParent /*=NULL*/)
	: CDialog(CLogTab::IDD, pParent)
{

}

CLogTab::~CLogTab()
{
}

void CLogTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLogTab, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CLogTab message handlers

HBRUSH CLogTab::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}
