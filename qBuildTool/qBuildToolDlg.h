
// qBuildToolDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"


// CqBuildToolDlg �Ի���
class CqBuildToolDlg : public CDialog
{
// ����
public:
	CqBuildToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_QBUILDTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
    CListCtrl m_lvwUrlList;
    CListCtrl m_lvwFileInfo;
};
