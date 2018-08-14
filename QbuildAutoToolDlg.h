
// QbuildAutoToolDlg.h : header file
//

#pragma once
#include "FileInfoTabItem.h"
#include "LogTabItem.h"
#include "CParseWorker.h"

#define WM_FILE_INFO_UPADTE     (WM_USER+1)
#define WM_SUCCESS_DOWNLOAD     (WM_USER+2)
#define WM_COMPLETE_DOWNLOAD    (WM_USER+4)
#define WM_SHOW_FILE_INFO       (WM_USER+5)

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
    afx_msg LRESULT     OnSuccessDownloadFile(WPARAM, LPARAM);
    afx_msg LRESULT     OnLogDownloadStatus(WPARAM, LPARAM);
    afx_msg LRESULT     OnShowFileInfo(WPARAM, LPARAM);
	afx_msg LRESULT		OnLogGitInfo(WPARAM, LPARAM);

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
    BOOL                        m_fEditGitPathChange;
	BOOL						m_fIsStartGetFile;
	BOOL						m_fISStartGit;
	BOOL						m_fClear;
    std::set<FILE_INFO*>        m_setFileInfo;

    BOOL GetFileLocationFromJson();
public:
	afx_msg void OnEnChangeEditUrlList();
    afx_msg void OnEnChangeEditNote();
	CString m_strNote;
};
