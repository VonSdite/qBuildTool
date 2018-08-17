#include "stdafx.h"
#include "CParseWorker.h"
#include "Function.h"

std::map<CString, BOOL> CUnzipTask::mapRecordExist;

CDownloadTask::CDownloadTask(HWND hWnd, std::set<CString> urls) 
    : m_hWnd(hWnd)
    , m_strUrls(urls)
{
}

CUnzipTask::CUnzipTask(HWND hWnd, CString strFilePath, const CString strGitPath, const CString strBranch, const Json::Value &jvRoot)
    : m_hWnd(hWnd)
    , m_strFilePath(strFilePath)
    , m_strBranch(strBranch)
    , m_jvRoot(jvRoot)
    , m_strGitPath(strGitPath)
{
}

void CDownloadTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    std::set<CString>::iterator iterUrl		= m_strUrls.begin();
    std::set<CString>::iterator iterUrlEnd	= m_strUrls.end();

    CString *strDownloadFileName; 
    for (; iterUrl!= iterUrlEnd; iterUrl++)
    {
        strDownloadFileName = new CString(DownloadSaveFile(*iterUrl));
        // ���سɹ�
        if (!strDownloadFileName->IsEmpty())
        {
            PostMessage(m_hWnd, WM_COMPLETE_DOWNLOAD, (WPARAM)new CString(*iterUrl), TRUE);
        }
        else
        {
            PostMessage(m_hWnd, WM_COMPLETE_DOWNLOAD, (WPARAM)new CString(*iterUrl), FALSE);
        }
    }
    PostMessage(m_hWnd, WM_DOWNLOAD_FINISHED, NULL, NULL);
}

void CUnzipTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    std::set<CString> strFiles = unzip(m_strFilePath);
    std::set<CString>::iterator iterFile = strFiles.begin();
    std::set<CString>::iterator iterFileEnd = strFiles.end();
    
    for (; iterFile != iterFileEnd; ++iterFile)
    {
        FILE_INFO *fileInfo = new FILE_INFO;
        GetFileInfo(*iterFile, *fileInfo);

        BOOL fHadAppeared = FALSE;
        CString strTmp = fileInfo->strFileName;
        if (mapRecordExist[strTmp.MakeLower()])
            fHadAppeared = TRUE;

        mapRecordExist[strTmp.MakeLower()] = TRUE;

        PostMessage(m_hWnd, WM_SHOW_FILE_INFO, (WPARAM)fileInfo, fHadAppeared);
    }
}

//�����ļ�
CString CDownloadTask::DownloadSaveFile(CString strUrl) 
{
    CString strSaveFile = CFunction::GetNameFromUrl(strUrl);

    bool ret=false;
    CInternetSession Sess;

    Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT     , 2000); //2������ӳ�ʱ
    Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT        , 2000); //2��ķ��ͳ�ʱ
    Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT     , 2000); //2��Ľ��ճ�ʱ
    Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT   , 2000); //2��ķ��ͳ�ʱ
    Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2��Ľ��ճ�ʱ
    DWORD dwFlag = INTERNET_FLAG_TRANSFER_BINARY|INTERNET_FLAG_DONT_CACHE|INTERNET_FLAG_RELOAD ;

    CHttpFile *cFile   = NULL;
    char      *pBuf    = NULL;
    int        nBufLen = 0   ;

    try{
        cFile = (CHttpFile*)Sess.OpenURL(strUrl, 1, dwFlag);
        DWORD dwStatusCode;
        cFile->QueryInfoStatusCode(dwStatusCode);
        if (dwStatusCode == HTTP_STATUS_OK) {
            // ��ѯ�ļ�����
            CString strDescription;
            cFile->QueryInfo(HTTP_QUERY_CONTENT_DESCRIPTION, strDescription);
            if (strDescription!=L"File Transfer") return L"";

            //��ѯ�ļ�����
            DWORD nLen = 0;
            cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);

            nBufLen = nLen;
            if (nLen <= 0) return L"";

            //����������ݻ���
            pBuf = (char*)malloc(nLen+8);
            ZeroMemory(pBuf,nLen+8);

            char *p=pBuf;
            while (nLen>0) {
                //ÿ������8K
                int n = cFile->Read(p, (nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
                //��������˳�ѭ��
                if (n <= 0) return L"";
                //���ջ������
                p+= n ;
                //ʣ�೤�ȵݼ�
                nLen -= n ;
            }

            //���δ�������ж��˳�
            if (nLen != 0) return L"";

            //���ճɹ����浽�ļ�

            CFile file(strSaveFile, CFile::modeCreate | CFile::modeWrite);
            file.Write(pBuf, nBufLen);
            file.Close();
            ret = true;
        }
    } catch(...) {
        return L"";
    }

    //�ͷŻ���
    if (pBuf) {
        free(pBuf);
        pBuf=NULL;
        nBufLen = 0 ;
    }

    //�ر���������
    if (cFile) {
        cFile->Close();
        Sess.Close();
        delete cFile;
    }
    return strSaveFile;
}

// ��ѹ�ļ�
std::set<CString> CUnzipTask::unzip(CString strSaveFile)
{
    std::set<CString> setFileName;
    unzFile zFile;
    // UnicodeתΪANSI
    char filePath[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, strSaveFile, -1, filePath, MAX_PATH, NULL, NULL);
    // ��ѹ���ļ�
    zFile = unzOpen64(filePath);
    if (zFile == NULL)
    {
        return setFileName;
    }

    // ��ȡѹ���ļ���ȫ����Ϣ
    unz_global_info64 zGlobalInfo; // ��Ҫ��Ա������ѹ���ļ��������ļ���������������Ŀ¼��
    if (UNZ_OK != unzGetGlobalInfo64(zFile, &zGlobalInfo))
    {
        return setFileName;
    }

    // ѭ�����������ļ�
    unz_file_info64 zFileInfo;
    unsigned int num = 512;
    char *fileName = new char[num];
    char *fileData = new char[5000*1024];

    for(int i=0; i < zGlobalInfo.number_entry; i++)
    {
        //���������ļ���fileName��ȫ·������
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
        //��ѹ���ļ�
        len = unzReadCurrentFile(zFile,(voidp)fileData,fileLength);
        fileData[len] = '\0';
        //д�뵽��ѹ������ļ��� 
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


void CUnzipTask::GetFileInfo(CString strFileName, FILE_INFO &fileInfo)
{
    // ��ȡ�ļ�����
    GetFileName(strFileName, fileInfo.strFileName);
    
    // ��ȡ�ļ���С��Md5
    GetFileSizeAndMd5(strFileName, fileInfo.dwSize, fileInfo.strMd5);
    
    // ��ȡǩ��ʱ��
    GetSignDateTime(strFileName, fileInfo.strSignTime);

    // ��ȡ�ļ��汾��
    GetFileVersion(strFileName, fileInfo.strVersion);

    // ����json�����ļ��л�ȡ�ļ����λ��
    GetFileLocation(strFileName,  fileInfo.strFileLocation);

    // ��ȡ�ļ�����״̬
    GetUpdateStatus(fileInfo);
}

// ��ȡ�ļ�����
void CUnzipTask::GetFileName(const CString &strFilePath, CString &strFileName)
{
	strFileName = PathFindFileName((LPCTSTR)strFilePath);
}

// ��ȡ�ļ���С��Md5
void CUnzipTask::GetFileSizeAndMd5(LPCWSTR strFilePath, DWORD &dwFileSize, CString &strMd5)
{
	HANDLE hFile = CreateFile(strFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)                                        	
    {     
		// ���CreateFile����ʧ��
		CloseHandle(hFile);
		return ;
	}
	HCRYPTPROV hProv = NULL;
	if( CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)     
	{
		// ���CSP��һ����Կ�����ľ��
		return ;
	}
	HCRYPTPROV hHash=NULL;
	if(CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)     
	{
		// ��ʼ������������hash������������һ����CSP��hash������صľ��������������������CryptHashData���á�
		return ;
	}

	dwFileSize = GetFileSize(hFile, NULL);			// ��ȡ�ļ��Ĵ�С
	if (dwFileSize==0xFFFFFFFF)                     // �����ȡ�ļ���Сʧ��
	{
		return ;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)       
	{
		// ��ȡ�ļ�
		return ;
	}
	if(CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)     
	{
		//hash�ļ�
		return ;
	}

	delete[] lpReadFileBuffer;
	CloseHandle(hFile);          //�ر��ļ����
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
	if(CryptGetHashParam(hHash,HP_HASHVAL,pbHash,&dwHashLen,0)) //���md5ֵ
	{
		for(DWORD i = 0; i < dwHashLen; i++)     //���md5ֵ
		{
			TCHAR tmp[10];
			swprintf_s(tmp, 10, L"%02x", pbHash[i]);
			strMd5 += tmp;
		}
	}
	//�ƺ���
	if(CryptDestroyHash(hHash)==FALSE)          //����hash����
	{
		return ;
	}
	if(CryptReleaseContext(hProv,0)==FALSE)
	{
		return ;
	}
	return ;
}

// ��ȡǩ��ʱ��
void CUnzipTask::GetSignDateTime(const CString &strFilePath, CString &strSignDate)
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

// ��ȡ�ļ��汾��
bool CUnzipTask::QueryValue(const CString& ValueName, const CString& szFileName, CString& RetStr)  
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
        // �ж�ϵͳ�ܷ������ָ���ļ��İ汾��Ϣ�����ж������ļ��汾��Ϣ��Ҫһ�����Ļ�����
        DWORD dwDataSize = ::GetFileVersionInfoSize(szFileName, &dwHandle);  
        if (dwDataSize == 0)  
            break;  
		// ���仺����  
        m_lpVersionData = new BYTE[dwDataSize];
        if ( NULL == m_lpVersionData)  
            break;  
        // ������Ϣ  
        //��֧�ְ汾��ǵ�һ��ģ�����ȡ�ļ��汾��Ϣ
        if (!::GetFileVersionInfo(szFileName, dwHandle, dwDataSize,  
            (void*)m_lpVersionData))  
            break;  
        UINT nQuerySize;  
        DWORD* pTransTable;  
        // ��������  
        if (!::VerQueryValue(m_lpVersionData, L"\\VarFileInfo\\Translation", (void **)&pTransTable, &nQuerySize))  
            break;  
        //MAKELONG ������16λ�������ϳ�һ���޷��ŵ�32λ��
        m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));  
        if (m_lpVersionData == NULL)  
            break;  
        tmpstr.Format(L"\\StringFileInfo\\%08lx\\%s", m_dwLangCharset, ValueName);

        LPVOID lpData;  
        // ���ô˺�����ѯǰ��Ҫ�����ε��ú���GetFileVersionInfoSize��GetFileVersionInfo  
        if (::VerQueryValue((void *)m_lpVersionData, tmpstr, &lpData, &nQuerySize))  
            RetStr = (LPCTSTR)lpData, RetStr.Replace(L',', L'.'), RetStr.Remove(L' ');
        bSuccess = TRUE;  
    } while (FALSE);  

    // ���ٻ�����  
    if (m_lpVersionData)  
    {  
        delete[] m_lpVersionData;  
        m_lpVersionData = NULL;  
    }  
    return bSuccess;  
}  

void CUnzipTask::GetFileVersion(const CString& szFileName, CString& strFileVersion)
{
    QueryValue(L"FileVersion", szFileName, strFileVersion);
}

void CUnzipTask::GetFileLocation(CString strFilePath, CString &strFileLocation)
{
     std::string szBranch;
     std::string szFilePath;
     
     strFilePath = strFilePath.Right(strFilePath.GetLength() - strFilePath.ReverseFind(L'\\') - 1);
     szBranch    = CT2A(m_strBranch.GetBuffer());
     szFilePath  = CT2A(strFilePath.GetBuffer());

     Json::Value arrFilePath;
     arrFilePath = m_jvRoot["Repository"][szBranch][szFilePath];

     if (arrFilePath.size() == 0)
        arrFilePath = m_jvRoot["Repository"]["Default"][szFilePath];

     std::string tmp;
     for (size_t i = 0; i < arrFilePath.size(); ++i)
     {
         tmp += arrFilePath[i].asString();
         if (i != arrFilePath.size() - 1)
             tmp += ";";
     }

     CString strFileLocationTmp(tmp.c_str());
     strFileLocation = strFileLocationTmp;

     m_strBranch.ReleaseBuffer();
     strFilePath.ReleaseBuffer();
}

void CUnzipTask::GetUpdateStatus(FILE_INFO &fileInfo)
{
    std::set<CString> location = CFunction::SplitCString(fileInfo.strFileLocation, L";");
    std::set<CString>::iterator it = location.begin();
    std::set<CString>::iterator itEnd = location.end();

    fileInfo.fUpdate = TRUE;
    for (; it != itEnd; ++it)
    {
        DWORD size;
        CString md5;
        CString strFileName = m_strGitPath + L"\\" + *it + L"\\" + fileInfo.strFileName;
        GetFileSizeAndMd5(strFileName, size, md5);

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

void CPushTask::DeleteFileByConfig()
{
    std::string szBranch;
    std::string szGitPath;
	Json::Value arrFilePath;

	szBranch    = CT2A(m_strBranch.GetBuffer());
    szGitPath   = CT2A(m_strGitPath.GetBuffer());
    arrFilePath = m_jvRoot["DeleteFile"][szBranch];

    if (arrFilePath.size() == 0)
        arrFilePath = m_jvRoot["DeleteFile"]["Default"];

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

    m_strBranch.ReleaseBuffer();
    m_strGitPath.ReleaseBuffer();
}

void CPushTask::UpdateFileByConfig()
{
    std::vector<FILE_INFO*>::iterator iter		= m_vecFileInfo.begin();
    std::vector<FILE_INFO*>::iterator iterEnd	= m_vecFileInfo.end();

    DWORD dwIndex = 0;
    for (; iter != iterEnd; ++iter)
    {
        std::set<CString>			setFilePath = CFunction::SplitCString((*iter)->strFileLocation, L";");
        std::set<CString>::iterator it			= setFilePath.begin();
        BOOL fCopySuccess = FALSE;
        for (; it != setFilePath.end(); ++it)
        {
            CString strFullPath = m_strGitPath + L"\\" + (*it);
            SHCreateDirectoryEx(NULL, strFullPath, NULL);
            strFullPath = strFullPath + L"\\" + (*iter)->strFileName;
            fCopySuccess = CopyFile(SAVE_PATH + (*iter)->strFileName, strFullPath, FALSE);
        }
        
        if (fCopySuccess)
        {
            PostMessage(m_hWnd, WM_UPDATE_SUCCESS, dwIndex, NULL);
        }
        ++dwIndex;
    }
}

void CPushTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    // ɾ�������ļ���ָ֧�����ļ���Ĭ��ָ�����ļ�
    DeleteFileByConfig();

    // �����ļ�
    UpdateFileByConfig();

    // git�������
	CString  strLogGitResult;
	CString  strLogCommit, strLogPush;
	CFunction::ExeCmd(L"git add .", m_strGitPath);
    strLogCommit = CFunction::ExeCmd(L"git commit -m " + (L"\"" + m_strNote + L"\""), m_strGitPath);
	strLogPush	 = CFunction::ExeCmd(L"git push --set-upstream origin " + m_strBranch, m_strGitPath);
	strLogGitResult = strLogGitResult + L"Commit:\r\n" + strLogCommit + L"\r\nPush:\r\n" + strLogPush + L"\r\n\r\n";

	PostMessage(m_hWnd, WM_LOG_GIT_INFO, (WPARAM)new CString(strLogGitResult), NULL);
}

CPushTask::CPushTask(HWND hWnd, const Json::Value &jvRoot, const std::vector<FILE_INFO*> &vecFileInfo, const CString &strGitPath, const CString &strBranch, const CString &strNote) 
    : m_hWnd(hWnd)
    , m_jvRoot(jvRoot)
    , m_vecFileInfo(vecFileInfo)
    , m_strBranch(strBranch)
    , m_strGitPath(strGitPath)
    , m_strNote(strNote)
{
}


