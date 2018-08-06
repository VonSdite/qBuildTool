#pragma once
#include "afxcmn.h"


// CFileInfoTab dialog

class CFileInfoTab : public CDialog
{
	DECLARE_DYNAMIC(CFileInfoTab)

public:
	CFileInfoTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileInfoTab();

// Dialog Data
	enum { IDD = IDD_FILE_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	CListCtrl m_listCtrlFileInfo;
	afx_msg void OnRclickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCopyMenu();

	// 对话框最大化
	//afx_msg void OnSize(UINT nType, int cx, int cy);
};
