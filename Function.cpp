#include "StdAfx.h"
#include "Function.h"

CFunction::CFunction(void)
{
}

CFunction::~CFunction(void)
{
}

// 执行cmd命令，获取返回值
CString CFunction::ExeCmd(CString pszCmd, CString szPath)
{
	//创建匿名管道
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return TEXT(" ");
	}

	//设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite)
	STARTUPINFO si = {sizeof(STARTUPINFO)};
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;

	//启动命令行
	PROCESS_INFORMATION pi;
	TCHAR *pszCmdLine = pszCmd.GetBuffer(pszCmd.GetLength());
	TCHAR *pszPath = szPath.GetBuffer(szPath.GetLength());
	if (!CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE, NULL, NULL, pszPath, &si, &pi))
	{
		return TEXT("cannot create process");
	}

	//立即关闭hWrite
	CloseHandle(hWrite);

	//读取命令行返回值
	CString strRet;
	char buff[1024] = {0};
	DWORD dwRead = 0;
	strRet = buff;
	while (ReadFile(hRead, buff, 1024, &dwRead, NULL))
	{
		strRet+=buff;
	}
	CloseHandle(hRead);

	return strRet;
}

// 分割CString字符串
std::set<CString> CFunction::SplitCString(CString strSrc, CString strGap)
{
	std::set<CString> strResult;
	int nPos;

	while ((nPos = strSrc.Find(strGap)) != -1)
	{
		CString tmp = strSrc.Left(nPos);
		if (!tmp.IsEmpty()) 
			strResult.insert(tmp);
		strSrc = strSrc.Right(strSrc.GetLength() - nPos - strGap.GetLength());
	}

	if (nPos == -1 && !strSrc.IsEmpty())
		strResult.insert(strSrc);
	return strResult;
}

// 获取默认分支字符串
CString CFunction::GetDefaultBranch(CString strSrc)
{
	// 查找*，赋值默认分支
	CString strDefaultBranch;
	int nPos;
	nPos = strSrc.Find('*');
	strDefaultBranch = strSrc.Mid(nPos+2, strSrc.GetLength()-nPos-1);
	nPos = strDefaultBranch.Find(L"\n");
	if (nPos != 0)
	{
		strDefaultBranch = strDefaultBranch.Left(nPos);
	}

	return strDefaultBranch;
}
