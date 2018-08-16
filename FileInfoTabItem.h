#pragma once
#include "CParseWorker.h"

// CFileInfoTab dialog

class Test : public CListCtrl
{

};

class CFileInfoTabItem : public CDialog
{
	DECLARE_DYNAMIC(CFileInfoTabItem)

public:
	CFileInfoTabItem(CWnd* pParent = NULL);             // standard constructor
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
    afx_msg HRESULT OnCopy(WPARAM, LPARAM);
    virtual BOOL OnInitDialog();
    void InsertFileInfo(FILE_INFO *fileInfo);

    // 对话框最大化
	//afx_msg void OnSize(UINT nType, int cx, int cy);

	CListCtrl   m_lvwFileInfo;

private:
    DWORD       m_dwRow;
    HACCEL      m_hAcc;

    // 列宽自适应
    void AutoAdjustColumnWidth(CListCtrl *pListCtrl);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
