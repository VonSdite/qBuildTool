
// stdafx.cpp : source file that includes just the standard includes
// QbuildAutoTool.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// 执行cmd命令，获取返回值
CString ExeCmd(CString pszCmd, CString szPath)
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

// 获取默认分支字符串
CString GetDefaultBranch(CString strSrc)
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

// 分割CString字符串
std::set<CString> SplitCString(CString strSrc, CString strGap)
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

//下载文件并保存为新文件名
bool DownloadSaveFiles(LPCTSTR url,LPCTSTR strSaveFile) {
	bool ret=false;
	CInternetSession Sess(TEXT("lpload"));
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT     , 2000); //2秒的连接超时
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT        , 2000); //2秒的发送超时
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT     , 2000); //2秒的接收超时
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT   , 2000); //2秒的发送超时
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2秒的接收超时
	DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_RELOAD ;

	CHttpFile* cFile   = NULL;
	char      *pBuf    = NULL;
	int        nBufLen = 0   ;
	do {
		try{
			cFile = (CHttpFile*)Sess.OpenURL(url,1,dwFlag);
			DWORD dwStatusCode;
			cFile->QueryInfoStatusCode(dwStatusCode);
			if (dwStatusCode == HTTP_STATUS_OK) {
				//查询文件长度
				DWORD nLen=0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen=nLen;
				if (nLen <= 0) break;//

				//分配接收数据缓存
				pBuf = (char*)malloc(nLen+8);
				ZeroMemory(pBuf,nLen+8);

				char *p=pBuf;
				while (nLen>0) {
					//每次下载8K
					int n = cFile->Read(p,(nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
					//接收完成退出循环
					if (n <= 0) break;//
					//接收缓存后移
					p+= n ;
					//剩余长度递减
					nLen -= n ;
				}

				//如果未接收完中断退出
				if (nLen != 0) break;

				//接收成功保存到文件

				CFile file(strSaveFile, CFile::modeCreate | CFile::modeWrite);
				file.Write(pBuf,nBufLen);
				file.Close();
				ret = true;
			}
		} catch(...) {
			break;//
		}
	} while(0);

	//释放缓存
	if (pBuf) {
		free(pBuf);
		pBuf=NULL;
		nBufLen = 0 ;
	}

	//关闭下载连接
	if (cFile) {
		cFile->Close();
		Sess.Close();
		delete cFile;
	}
	return ret;
}