#include "stdafx.h"
#include "CParseWorker.h"
#include "Function.h"

CString            DownloadSaveFile(CString url, CString strSavePath);		
std::set<CString>  unzip(CString strSaveFile);  
void               GetFileInfo(CString strFilePath, FILE_INFO &fileInfo, CString &strBranch, const Json::Value &jvRoot);
void               GetFileName(const CString &strFilePath, CString &strFileName);
void               GetSignDateTime(const CString &strFilePath, CString &strSignDate);
void               GetFileSizeAndMd5(LPCWSTR FileDirectory, DWORD &dwFileSize, CString &strMd5);
bool               QueryValue(const CString& ValueName, const CString& strFilePath, CString& RetStr);
void               GetFileVersion(const CString& strFilePath, CString& strFileVersion);
void               GetFileLocation(CString strFilePath, CString &strBranch, const Json::Value &jvRoot, CString &strFileLocation);
void               DeleteFileByConfig(const Json::Value &jvRoot, CString strGitPath, CString strBranch);
void               UpdateFileByConfig(std::set<FILE_INFO *>&setFileInfo, CString strGitPath, CString strBranch);

CDownloadTask::CDownloadTask(HWND hWnd, CString strUrl, CString strSavePath) : m_hWnd(hWnd), m_strUrl(strUrl), m_strSavePath(strSavePath) 
{
}

void CDownloadTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
	
	CString *strDownloadFileName; 
	strDownloadFileName = new CString(DownloadSaveFile(m_strUrl, m_strSavePath));
	// 下载成功
	if (!strDownloadFileName->IsEmpty())
	{
		PostMessage(m_hWnd, WM_COMPLETE_DOWNLOAD, (WPARAM)new CString(m_strUrl), TRUE);
		PostMessage(m_hWnd, WM_SUCCESS_DOWNLOAD, (WPARAM)strDownloadFileName, NULL);
	}
	// 下载失败
	else 
	{
		PostMessage(m_hWnd, WM_COMPLETE_DOWNLOAD, (WPARAM)new CString(m_strUrl), FALSE);
	}

	
}

CUnzipTask::CUnzipTask(HWND hWnd, CString strFilePath, const CString strBranch, const Json::Value &jvRoot) : m_hWnd(hWnd), m_strFilePath(strFilePath), m_strBranch(strBranch), m_jvRoot(jvRoot)
{
}

void CUnzipTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    std::set<CString> strFiles = unzip(m_strFilePath);
    std::set<CString>::iterator iterFile = strFiles.begin();
    std::set<CString>::iterator iterFileEnd = strFiles.end();
    
    for (; iterFile != iterFileEnd; ++iterFile)
    {
        FILE_INFO *fileInfo = new FILE_INFO;
        GetFileInfo(*iterFile, *fileInfo, m_strBranch, m_jvRoot);
        PostMessage(m_hWnd, WM_SHOW_FILE_INFO, (WPARAM)fileInfo, NULL);
    }
}

//下载文件
CString DownloadSaveFile(CString strUrl, CString strSavePath) 
{
    static BOOL isSetSeed = FALSE;
    if (!isSetSeed)
    {
        srand((unsigned)time(NULL));
        isSetSeed = TRUE;
    }
    CString strSaveFile;
    strSaveFile.Format(L"%04x%04x%04x.zip", rand() % 0x10000, rand() % 0x10000, rand() % 0x10000);
    strSaveFile = strSavePath + strSaveFile;

    bool ret=false;
    //CInternetSession Sess(TEXT("lpload"));

	CInternetSession Sess;
    Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT     , 2000); //2秒的连接超时
    Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT        , 2000); //2秒的发送超时
    Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT     , 2000); //2秒的接收超时
    Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT   , 2000); //2秒的发送超时
    Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2秒的接收超时
    DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_RELOAD ;

    CHttpFile* cFile   = NULL;
    char      *pBuf    = NULL;
    int        nBufLen = 0   ;
    try{
        cFile = (CHttpFile*)Sess.OpenURL(strUrl, 1, dwFlag);
        DWORD dwStatusCode;
        cFile->QueryInfoStatusCode(dwStatusCode);
        if (dwStatusCode == HTTP_STATUS_OK) {
            //查询文件长度
            DWORD nLen = 0;
            cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
            nBufLen = nLen;
            if (nLen <= 0) return L"";

            //分配接收数据缓存
            pBuf = (char*)malloc(nLen+8);
            ZeroMemory(pBuf,nLen+8);

            char *p=pBuf;
            while (nLen>0) {
                //每次下载8K
                int n = cFile->Read(p, (nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
                //接收完成退出循环
                if (n <= 0) return L"";
                //接收缓存后移
                p+= n ;
                //剩余长度递减
                nLen -= n ;
            }

            //如果未接收完中断退出
            if (nLen != 0) return L"";

            //接收成功保存到文件

            CFile file(strSaveFile, CFile::modeCreate | CFile::modeWrite);
            file.Write(pBuf, nBufLen);
            file.Close();
            ret = true;
        }
    } catch(...) {
        return L"";
    }

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
    return strSaveFile;
}

// 解压文件
std::set<CString> unzip(CString strSaveFile)
{
    std::set<CString> setFileName;
    unzFile zFile;
    // Unicode转为ANSI
    char filePath[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, strSaveFile, -1, filePath, MAX_PATH, NULL, NULL);
    // 打开压缩文件
    zFile = unzOpen64(filePath);
    if (zFile == NULL)
    {
        return setFileName;
    }

    // 获取压缩文件的全局消息
    unz_global_info64 zGlobalInfo; // 重要成员变量是压缩文件内所有文件的数量（不包括目录）
    if (UNZ_OK != unzGetGlobalInfo64(zFile, &zGlobalInfo))
    {
        return setFileName;
    }

    // 循环遍历所有文件
    unz_file_info64 zFileInfo;
    unsigned int num = 512;
    char *fileName = new char[num];
    char *fileData = new char[5000*1024];

    for(int i=0; i < zGlobalInfo.number_entry; i++)
    {
        //遍历所有文件（fileName是全路径名）
        if(UNZ_OK != unzGetCurrentFileInfo64(zFile,&zFileInfo,fileName,num,NULL,0,NULL,0))
        {
            return setFileName;
        }
        if(UNZ_OK != unzOpenCurrentFile(zFile))
        {
            return setFileName;
        }
        int fileLength = (int)zFileInfo.uncompressed_size;
        int len;
        //解压缩文件
        len = unzReadCurrentFile(zFile,(voidp)fileData,fileLength);
        fileData[len] = '\0';
        //写入到解压缩后的文件中 
        CString strFileName(fileName);
        strFileName = strFileName.Right(strFileName.GetLength() - strFileName.ReverseFind(L'/') - 1);
        CString strTmpSaveFileName = strSaveFile.Left(strSaveFile.ReverseFind(L'\\') + 1) + strFileName;
        CFile file(strTmpSaveFileName, CFile::modeCreate | CFile::modeWrite);
        setFileName.insert(strTmpSaveFileName);
        file.Write(fileData,len);
        file.Close();
        unzCloseCurrentFile(zFile);
        unzGoToNextFile(zFile);
    }

    unzClose(zFile);
    delete[] fileName;
    delete[] fileData;
    return setFileName;
}

LONG g_lCurrId = -1;

CParseWorker::CParseWorker() : m_dwExecs( 0 )
{
    m_lId = InterlockedIncrement( &g_lCurrId );
}

BOOL CParseWorker::Initialize(void *pvParam)
{
    return TRUE;
}

void CParseWorker::Terminate(void* /*pvParam*/)
{
}

void CParseWorker::Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw()
{
    CTaskBase* pTask = (CTaskBase*)(DWORD_PTR)dw;
    pTask->DoTask(pvParam, pOverlapped);
    m_dwExecs++;
    delete pTask;
}

BOOL CParseWorker::GetWorkerData(DWORD /*dwParam*/, void ** /*ppvData*/)
{
    return FALSE;
}


void GetFileInfo(CString strFilePath, FILE_INFO &fileInfo, CString &strBranch, const Json::Value &jvRoot)
{
    // 获取文件名字
    GetFileName(strFilePath, fileInfo.strFileName);
    
    // 获取文件大小和Md5
    GetFileSizeAndMd5(strFilePath, fileInfo.dwSize, fileInfo.strMd5);
    
    // 获取签名时间
    GetSignDateTime(strFilePath, fileInfo.strSignTime);

    // 获取文件版本号
    GetFileVersion(strFilePath, fileInfo.strVersion);

    // 根据json配置文件中获取文件存放位置
    GetFileLocation(strFilePath, strBranch, jvRoot, fileInfo.strFileLocation);

    std::set<CString> location = CFunction::SplitCString(fileInfo.strFileLocation, L";");
    std::set<CString>::iterator it = location.begin();
    std::set<CString>::iterator itEnd = location.end();

    fileInfo.fUpdate = TRUE;
    for (; it != itEnd; ++it)
    {
        DWORD size;
        CString md5;
        GetFileSizeAndMd5(*it, size, md5);

        if (md5.IsEmpty()) continue;

        if (md5 != fileInfo.strMd5)
        {
            fileInfo.fUpdate = TRUE;
            break;
        }
        else 
        {
            fileInfo.fUpdate = FALSE;
            break;
        }
    }
}

// 获取文件名字
void GetFileName(const CString &strFilePath, CString &strFileName)
{
	strFileName = PathFindFileName((LPCTSTR)strFilePath);
}

// 获取文件大小和Md5
void GetFileSizeAndMd5(LPCWSTR strFilePath, DWORD &dwFileSize, CString &strMd5)
{
	HANDLE hFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)                                        	
    {     
		// 如果CreateFile调用失败
		CloseHandle(hFile);
		return ;
	}
	HCRYPTPROV hProv = NULL;
	if( CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)     
	{
		// 获得CSP中一个密钥容器的句柄
		return ;
	}
	HCRYPTPROV hHash=NULL;
	if(CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)     
	{
		// 初始化对数据流的hash，创建并返回一个与CSP的hash对象相关的句柄。这个句柄接下来将被CryptHashData调用。
		return ;
	}

	dwFileSize = GetFileSize(hFile, NULL);			// 获取文件的大小
	if (dwFileSize==0xFFFFFFFF)                     // 如果获取文件大小失败
	{
		return ;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)       
	{
		// 读取文件
		return ;
	}
	if(CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)     
	{
		//hash文件
		return ;
	}

	delete[] lpReadFileBuffer;
	CloseHandle(hFile);          //关闭文件句柄
	BYTE *pbHash;
	DWORD dwHashLen=sizeof(DWORD);

	if (CryptGetHashParam(hHash,HP_HASHVAL,NULL,&dwHashLen,0))     
	{
	}
	else
	{
		return ;
	}
	if(pbHash=(byte*)malloc(dwHashLen))
	{

	}
	else
	{
		return ;
	}
	if(CryptGetHashParam(hHash,HP_HASHVAL,pbHash,&dwHashLen,0)) //获得md5值
	{
		for(DWORD i = 0; i < dwHashLen; i++)     //输出md5值
		{
			TCHAR tmp[10];
			swprintf_s(tmp, 10, L"%02x", pbHash[i]);
			strMd5 += tmp;
		}
	}
	//善后工作
	if(CryptDestroyHash(hHash)==FALSE)          //销毁hash对象
	{
		return ;
	}
	if(CryptReleaseContext(hProv,0)==FALSE)
	{
		return ;
	}
	return ;
}

// 获取签名时间
void GetSignDateTime(const CString &strFilePath, CString &strSignDate)
{
	GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_FILE_INFO sWintrustFileInfo;
	WINTRUST_DATA      sWintrustData;
	HRESULT            hr;

	memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));
	memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));

	sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
	sWintrustFileInfo.pcwszFilePath = strFilePath;
	sWintrustFileInfo.hFile = NULL;

	sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
	sWintrustData.dwUIChoice          = WTD_UI_NONE;
	sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
	sWintrustData.pFile               = &sWintrustFileInfo;
	sWintrustData.dwStateAction       = WTD_STATEACTION_VERIFY;

	hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

	if (TRUST_E_NOSIGNATURE == hr)
	{
		strSignDate = _T("No signature found on the file");
	}
	else if (TRUST_E_BAD_DIGEST == hr)
	{
		strSignDate = _T("The signature of the file is invalid");
	}
	else if (TRUST_E_PROVIDER_UNKNOWN == hr)
	{
		strSignDate = _T("No trust provider on this machine can verify this type of files");
	}
	else if (S_OK != hr)
	{
		strSignDate = _T("WinVerifyTrust failed with error");
	}
	else
	{
		// retreive the signer certificate and display its information
		CRYPT_PROVIDER_DATA const *psProvData     = NULL;
		CRYPT_PROVIDER_SGNR       *psProvSigner   = NULL;
		CRYPT_PROVIDER_CERT       *psProvCert     = NULL;
		FILETIME                   localFt;
		SYSTEMTIME                 sysTime;

		psProvData = WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
		if (psProvData)
		{
			psProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0, FALSE, 0);
			if (psProvSigner)
			{
				FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
				FileTimeToSystemTime(&localFt, &sysTime);

				strSignDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), sysTime.wYear, sysTime.wMonth,sysTime.wDay, sysTime.wHour, sysTime.wMinute,sysTime.wSecond);
			}
		}
	}

	sWintrustData.dwUIChoice = WTD_UI_NONE;
	sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
	WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);
}

// 获取文件版本号
bool QueryValue(const CString& ValueName, const CString& szFileName, CString& RetStr)  
{  
    bool	bSuccess = FALSE;  
    BYTE	*m_lpVersionData = NULL;  
    DWORD   m_dwLangCharset = 0;  
    CString tmpstr;
    do  
    {  
        if (!ValueName.GetLength() || !szFileName.GetLength())  
            break;  
        DWORD dwHandle;  
        // 判断系统能否检索到指定文件的版本信息，并判断容纳文件版本信息需要一个多大的缓冲区
        DWORD dwDataSize = ::GetFileVersionInfoSize(szFileName, &dwHandle);  
        if (dwDataSize == 0)  
            break;  
		// 分配缓冲区  
        m_lpVersionData = new BYTE[dwDataSize];
        if ( NULL == m_lpVersionData)  
            break;  
        // 检索信息  
        //从支持版本标记的一个模块里获取文件版本信息
        if (!::GetFileVersionInfo(szFileName, dwHandle, dwDataSize,  
            (void*)m_lpVersionData))  
            break;  
        UINT nQuerySize;  
        DWORD* pTransTable;  
        // 设置语言  
        if (!::VerQueryValue(m_lpVersionData, L"\\VarFileInfo\\Translation", (void **)&pTransTable, &nQuerySize))  
            break;  
        //MAKELONG 将两个16位的数联合成一个无符号的32位数
        m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));  
        if (m_lpVersionData == NULL)  
            break;  
        tmpstr.Format(L"\\StringFileInfo\\%08lx\\%s", m_dwLangCharset, ValueName);

        LPVOID lpData;  
        // 调用此函数查询前需要先依次调用函数GetFileVersionInfoSize和GetFileVersionInfo  
        if (::VerQueryValue((void *)m_lpVersionData, tmpstr, &lpData, &nQuerySize))  
            RetStr = (LPCTSTR)lpData, RetStr.Replace(L',', L'.'), RetStr.Remove(L' ');
        bSuccess = TRUE;  
    } while (FALSE);  

    // 销毁缓冲区  
    if (m_lpVersionData)  
    {  
        delete[] m_lpVersionData;  
        m_lpVersionData = NULL;  
    }  
    return bSuccess;  
}  

void GetFileVersion(const CString& szFileName, CString& strFileVersion)
{
    QueryValue(L"FileVersion", szFileName, strFileVersion);
}

void GetFileLocation(CString strFilePath, CString &strBranch,const Json::Value &jvRoot, CString &strFileLocation)
{
     std::string szBranch;
     std::string szFilePath;
     
     strFilePath = strFilePath.Right(strFilePath.GetLength() - strFilePath.ReverseFind(L'\\') - 1);
     szBranch    = CT2A(strBranch.GetBuffer());
     szFilePath  = CT2A(strFilePath.GetBuffer());

     Json::Value arrFilePath;
     arrFilePath = jvRoot["Repository"][szBranch][szFilePath];

     if (arrFilePath.size() == 0)
        arrFilePath = jvRoot["Repository"]["Default"][szFilePath];

     std::string tmp;
     for (size_t i = 0; i < arrFilePath.size(); ++i)
     {
         tmp += arrFilePath[i].asString();
         if (i != arrFilePath.size() - 1)
             tmp += ";";
     }

     CString strFileLocationTmp(tmp.c_str());
     strFileLocation = strFileLocationTmp;
}

void DeleteFileByConfig(const Json::Value &jvRoot, CString strGitPath, CString strBranch)
{
    std::string szBranch;
    std::string szGitPath;
	Json::Value arrFilePath;

	szBranch    = CT2A(strBranch.GetBuffer());
    szGitPath   = CT2A(strGitPath.GetBuffer());
    arrFilePath = jvRoot["DeleteFile"][szBranch];

    if (arrFilePath.size() == 0)
        arrFilePath = jvRoot["DeleteFile"]["Default"];

    Json::Value::Members		   mem = arrFilePath.getMemberNames();
    Json::Value::Members::iterator it  = mem.begin();
 
    for (; it != mem.end(); ++it)
    {
        std::string szFileName = *it;
        for(size_t i = 0; i < arrFilePath[*it].size(); ++i)
        {
            std::string szPath = szGitPath + arrFilePath[*it][i].asString() + "\\" + szFileName;
            DeleteFile(CFunction::s2ws(szPath).c_str());
        }
    }
}

void UpdateFileByConfig(std::set<FILE_INFO *> &setFileInfo, CString strGitPath, CString strBranch)
{
    std::set<FILE_INFO*>::iterator iter		= setFileInfo.begin();
    std::set<FILE_INFO*>::iterator iterEnd	= setFileInfo.end();

    for (; iter != iterEnd; ++iter)
    {
        std::set<CString>			setFilePath = CFunction::SplitCString((*iter)->strFileLocation, L";");
        std::set<CString>::iterator it			= setFilePath.begin();
        for (; it != setFilePath.end(); ++it)
        {
            CString strFullPath = strGitPath + L"\\" + (*it);
            SHCreateDirectoryEx(NULL, strFullPath, NULL);
            strFullPath = strFullPath + L"\\" + (*iter)->strFileName;
            CopyFile(L"Temp\\" + (*iter)->strFileName, strFullPath, FALSE);
        }
    }
}

void CPushTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    // 删除配置文件分支指定的文件或默认指定的文件
    DeleteFileByConfig(m_jvRoot, m_strGitPath, m_strBranch);

    // 更新文件
    UpdateFileByConfig(m_setFileInfo, m_strGitPath, m_strBranch);

    // git命令入库
	CString  strLogGitResult;
	CString  strLogCommit, strLogPush;
	CFunction::ExeCmd(L"git add .", m_strGitPath);
    strLogCommit = CFunction::ExeCmd(L"git commit -m " + m_strNote, m_strGitPath);
	strLogPush	 = CFunction::ExeCmd(L"git push --set-upstream origin "+m_strBranch, m_strGitPath);
	strLogGitResult = strLogGitResult + L"Commit:\r\n" + strLogCommit + L"\r\nPush:\r\n" + strLogPush + L"\r\n\r\n";

	PostMessage(m_hWnd, WM_LOG_GIT_INFO, (WPARAM)new CString(strLogGitResult), NULL);
}

CPushTask::CPushTask(HWND hWnd, const Json::Value &jvRoot, const std::set<FILE_INFO *> &setFileInfo, const CString &strGitPath, const CString &strBranch, const CString &strNote) : m_hWnd(hWnd), m_jvRoot(jvRoot), m_setFileInfo(setFileInfo), m_strBranch(strBranch), m_strGitPath(strGitPath), m_strNote(strNote)
{

}


