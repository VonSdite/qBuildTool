#pragma once
#include "stdafx.h"

#define RECVPACK_SIZE 2048


class CFunction
{
public:
	CFunction(void);
	~CFunction(void);

public:
	static CString ExeCmd(CString pszCmd, CString szPath);							// ִ��������
	static std::set<CString> SplitCString(CString strSrc, CString strGap);			// �ָ�CString�ַ���
	static CString GetDefaultBranch(CString strSrc);								// ��ȡĬ�Ϸ�֧�ַ���
};
