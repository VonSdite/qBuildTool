#pragma once
#include "stdafx.h"

#define RECVPACK_SIZE 2048


class CFunction
{
public:
	CFunction(void);
	~CFunction(void);

public:
	static CString ExeCmd(CString pszCmd, CString szPath);							// 执行命令行
	static std::set<CString> SplitCString(CString strSrc, CString strGap);			// 分割CString字符串
	static CString GetDefaultBranch(CString strSrc);								// 获取默认分支字符串
};
