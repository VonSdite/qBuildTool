#pragma once


// CLogTab dialog

class CLogTabItem : public CDialog
{
	DECLARE_DYNAMIC(CLogTabItem)

public:
	CLogTabItem(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogTabItem();

// Dialog Data
	enum { IDD = IDD_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
