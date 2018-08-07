
// QbuildAutoToolDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "FileInfoTabItem.h"
#include "LogTabItem.h"
#include "CParseWorker.h"

// CQbuildAutoToolDlg dialog
class CQbuildAutoToolDlg : public CDialog
{
// Construction
public:
	CQbuildAutoToolDlg(CWnd* pParent = NULL);	// standard constructor
	void SendLogInfo(CString strLogInfo);

// Dialog Data
	enum { IDD = IDD_QbuildAutoTool_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox			m_cmbBranch;
	CTabCtrl			m_tabFileInfo;
    CFileInfoTabItem	m_tabItemFileInfo;
    CLogTabItem			m_tabItemLog;
    CThreadPool<CParseWorker> m_thrdpoolParse;
    BOOL isEditGitPathChange;

public:
    afx_msg void OnBnClickedExit();
    afx_msg void OnBnClickedPushFile();
    afx_msg void OnBnClickedGetFile();
    afx_msg void OnBnClickedBrowse();
    //afx_msg void OnCbnSelchangeComboBranch();
    afx_msg void OnTcnSelchangeTab2(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEnKillfocusEditGitPath();

    //// 对话框最大化
    //afx_msg void OnSize(UINT nType, int cx, int cy);
    //void ReSize();
    afx_msg void OnEnChangeEditGitPath();
	afx_msg void OnCbnSelchangeComboBranch();
};
