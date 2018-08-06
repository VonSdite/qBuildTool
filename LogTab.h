#pragma once


// CLogTab dialog

class CLogTab : public CDialog
{
	DECLARE_DYNAMIC(CLogTab)

public:
	CLogTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogTab();

// Dialog Data
	enum { IDD = IDD_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
