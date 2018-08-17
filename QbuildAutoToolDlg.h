
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
	CQbuildAutoToolDlg(CWnd* pParent = NULL);	           // standard constructor
    ~CQbuildAutoToolDlg();
	BOOL PreTranslateMessage(MSG* pMsg);

// Dialog Data
	enum { IDD = IDD_QbuildAutoTool_DIALOG };

protected:
	virtual void        DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

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
    afx_msg void        OnEnKillfocusEditGitPath();
    afx_msg void        OnEnChangeEditGitPath();
    afx_msg void        OnCbnSelchangeComboBranch();
    afx_msg void        OnEnChangeEditUrlList();
    afx_msg void        OnEnChangeEditNote();
    afx_msg void        OnTimer(UINT_PTR nIDEvent);
    afx_msg void        OnTcnSelchangeTabInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT     OnLogDownloadStatus(WPARAM, LPARAM);
    afx_msg LRESULT     OnShowFileInfo(WPARAM, LPARAM);
	afx_msg LRESULT		OnLogGitInfo(WPARAM, LPARAM);
    afx_msg LRESULT     OnDownLoadFinished(WPARAM, LPARAM);
    afx_msg LRESULT     OnFileUpdateSuccess(WPARAM, LPARAM);

    // 显示进度
    void ShowProgress(const CString &strProgressStatus = L"");

public:
    CString                     m_strNote;
    CButton                     m_btnPushFile;
    CButton                     m_btnGetFile;
    CString                     m_strUrls;

private:
    CString                     m_strGitPath;
    CString                     m_strBranch;
    CComboBox			        m_cmbBranch;
    CTabCtrl			        m_tabInfo;
    CFileInfoTabItem	        m_tabItemFileInfo;
    CLogTabItem			        m_tabItemLog;
    CThreadPool<CParseWorker>   m_thrdpoolParse;
    Json::Value                 m_jvRoot;
    std::vector<FILE_INFO*>     m_vecFileInfo;
    HICON                       m_hIconLoading[9];

    // 标志文件是否全部下载成功 
    BOOL                        m_fCompleteDownload;    
    // 标志git仓库路径发生改变
    BOOL                        m_fIsEditGitPathChange;
	BOOL						m_fIsDownLoading;
    BOOL                        m_fCanPush;

    // 从json文件读取配置信息
    BOOL GetFileLocationFromJson();
    // 清空文件信息列表和日志内容
    void Clear();
};
