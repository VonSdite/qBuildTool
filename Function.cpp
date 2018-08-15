#include "stdafx.h"
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
	si.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow	= SW_HIDE;
	si.hStdError	= hWrite;
	si.hStdOutput	= hWrite;

	//启动命令行
	PROCESS_INFORMATION pi;
	TCHAR *pszCmdLine = pszCmd.GetBuffer(pszCmd.GetLength());
    TCHAR *pszPath	  = szPath.IsEmpty()?NULL:szPath.GetBuffer(szPath.GetLength());
	if (!CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE, NULL, NULL, pszPath, &si, &pi))
	{
        DWORD tmp = GetLastError();
		return TEXT("cannot create process");
	}

	//立即关闭hWrite
	CloseHandle(hWrite);

	//读取命令行返回值
	CString strRet;
	char	buff[1024] = {0};
	DWORD	dwRead = 0;
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
	int		nPos;

	nPos			 = strSrc.Find('*');
	strDefaultBranch = strSrc.Mid(nPos+2, strSrc.GetLength()-nPos-1);
	nPos			 = strDefaultBranch.Find(L"\n");
	if (nPos != 0)
	{
		strDefaultBranch = strDefaultBranch.Left(nPos);
	}

	return strDefaultBranch;
}

// 删除非空文件夹
BOOL CFunction::RemoveDir(CString strFileDir)
{
	if(strFileDir.GetAt(strFileDir.GetLength()-1)!=L'\\')
		strFileDir += L'\\';
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((strFileDir + L"*.*"),&wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_wcsicmp(wfd.cFileName,L".") != 0 &&
				_wcsicmp(wfd.cFileName,L"..") != 0)
				RemoveDir( (strFileDir + wfd.cFileName));
		}
		else
		{
			DeleteFile( (strFileDir + wfd.cFileName));
		}
	}
	while (FindNextFile(hFind,&wfd));
	FindClose(hFind);
	RemoveDirectory(strFileDir);
	return TRUE;
}

// wstring转string
std::string CFunction::ws2s(const std::wstring& ws)
{
    std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
    setlocale(LC_ALL, "chs");
    const wchar_t* _Source = ws.c_str();
    size_t _Dsize = 2 * ws.size() + 1;
    char *_Dest = new char[_Dsize];
    memset(_Dest,0,_Dsize);
    wcstombs(_Dest,_Source,_Dsize);
    std::string result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, curLocale.c_str());
    return result;
}

// string转wstring
std::wstring CFunction::s2ws(const std::string& s)
{
    setlocale(LC_ALL, "chs"); 
    const char* _Source = s.c_str();
    size_t _Dsize = s.size() + 1;
    wchar_t *_Dest = new wchar_t[_Dsize];
    wmemset(_Dest, 0, _Dsize);
    mbstowcs(_Dest,_Source,_Dsize);
    std::wstring result = _Dest;
    delete []_Dest;
    setlocale(LC_ALL, "C");
    return result;
}


std::vector<CString> CFunction::SplitString(CString strSrc, CString strGap)
{
    std::vector<CString> strResult;
    int nPos;

    while ((nPos = strSrc.Find(strGap)) != -1)
    {
        CString tmp = strSrc.Left(nPos);
        if (!tmp.IsEmpty()) 
            strResult.push_back(tmp);
        strSrc = strSrc.Right(strSrc.GetLength() - nPos - strGap.GetLength());
    }

    if (nPos == -1 && !strSrc.IsEmpty())
        strResult.push_back(strSrc);
    return strResult;
}

CString CFunction::GetNameFromUrl(CString strUrl)
{
    CString strTmpUrl = strUrl;
    strTmpUrl.Remove(L'\\');
    strTmpUrl.Remove(L'/');
    strTmpUrl.Remove(L':');
    strTmpUrl.Remove(L'.');
    strTmpUrl.Remove(L'?');

    std::vector<CString> vecSplitString = CFunction::SplitString(strTmpUrl, L"=");

    CString strSaveFile = vecSplitString[vecSplitString.size() - 1] + L".zip";

    strSaveFile = SAVE_PATH + strSaveFile;
    return strSaveFile;
}
