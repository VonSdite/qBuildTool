// CLogTab.cpp : implementation file
//

#include "stdafx.h"
#include "QbuildAutoTool.h"
#include "LogTabItem.h"


// CLogTab dialog

IMPLEMENT_DYNAMIC(CLogTabItem, CDialog)

CLogTabItem::CLogTabItem(CWnd* pParent /*=NULL*/)
	: CDialog(CLogTabItem::IDD, pParent)
{

}

CLogTabItem::~CLogTabItem()
{
}

void CLogTabItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
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
