
// QbuildAutoToolDlg.h : header file
//

#pragma once
#include "FileInfoTabItem.h"
#include "LogTabItem.h"
#include "CParseWorker.h"
#include "MyMacro.h"
#include "afxwin.h"

// CQbuildAutoToolDlg dialog
class CQbuildAutoToolDlg : public CDialog
{
// Construction
public:
	CQbuildAutoToolDlg(CWnd* pParent = NULL);	        // standard constructor
    ~CQbuildAutoToolDlg();
	BOOL PreTranslateMessage(MSG* pMsg);

// Dialog Data
	enum { IDD = IDD_QbuildAutoTool_DIALOG };

protected:
	virtual void        DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL        OnInitDialog();
	afx_msg void        OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void        OnPaint();
	afx_msg HCURSOR     OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
    afx_msg void        OnBnClickedExit();
    afx_msg void        OnBnClickedPushFile();
    afx_msg void        OnBnClickedGetFile();
    afx_msg void        OnBnClickedBrowse();
    afx_msg void        OnTcnSelchangeTabInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void        OnEnKillfocusEditGitPath();
    afx_msg void        OnEnChangeEditGitPath();
    afx_msg void        OnCbnSelchangeComboBranch();
    afx_msg LRESULT     OnLogDownloadStatus(WPARAM, LPARAM);
    afx_msg LRESULT     OnShowFileInfo(WPARAM, LPARAM);
	afx_msg LRESULT		OnLogGitInfo(WPARAM, LPARAM);
    afx_msg LRESULT     OnDownLoadFinished(WPARAM, LPARAM);

    //// 对话框最大化
    //afx_msg void OnSize(UINT nType, int cx, int cy);
    //void ReSize();
    
private:
    CString                     m_strGitPath;
    CString                     m_strBranch;
    CComboBox			        m_cmbBranch;
    CTabCtrl			        m_tabInfo;
    CFileInfoTabItem	        m_tabItemFileInfo;
    CLogTabItem			        m_tabItemLog;
    CThreadPool<CParseWorker>   m_thrdpoolParse;
    Json::Value                 m_jvRoot;
    BOOL                        m_fIsEditGitPathChange;
	BOOL						m_fEnableGitPush;
    std::set<FILE_INFO*>        m_setFileInfo;

    BOOL GetFileLocationFromJson();
    void Clear();
public:
	afx_msg void OnEnChangeEditUrlList();
    afx_msg void OnEnChangeEditNote();

	// 显示进度
	void ShowProgress(const CString &strProgressStatus = L"");
	CString m_strNote;
	CButton m_btnPushFile;
	CButton m_btnGetFile;
	CString m_strUrls;
};
