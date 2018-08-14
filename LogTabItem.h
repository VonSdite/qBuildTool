#pragma once
#include "Resource.h"

// CLogTab dialog
class CLogTabItem : public CDialog
{
	DECLARE_DYNAMIC(CLogTabItem)

public:
	CLogTabItem(CWnd* pParent = NULL);   // standard constructor
	virtual         ~CLogTabItem();
    void            ShowLogUrlInfo(CString strUrl, BOOL fSuccess);
	void			ShowLogGitInfo(CString strGitResult);
	afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CString         m_strLog;
	BOOL			m_fFirstLogUrlInfo;
	BOOL			m_fFirstLogGitInfo;


// Dialog Data
	enum { IDD = IDD_LOG };

protected:
	virtual void    DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
    CEdit           m_cbEditLog;

};
