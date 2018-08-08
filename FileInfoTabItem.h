#pragma once
#include "afxcmn.h"


// CFileInfoTab dialog

class CFileInfoTabItem : public CDialog
{
	DECLARE_DYNAMIC(CFileInfoTabItem)

public:
	CFileInfoTabItem(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileInfoTabItem();

// Dialog Data
	enum { IDD = IDD_FILE_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRclickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCopyMenu();
    virtual BOOL OnInitDialog();

    // 对话框最大化
	//afx_msg void OnSize(UINT nType, int cx, int cy);

public:
    CListCtrl m_lvwFileInfo;
};
