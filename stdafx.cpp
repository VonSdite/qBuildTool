
// stdafx.cpp : source file that includes just the standard includes
// QbuildAutoTool.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// ִ��cmd�����ȡ����ֵ
CString ExeCmd(CString pszCmd, CString szPath)
{
    //���������ܵ�
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    {
        return TEXT(" ");
    }

    //���������н���������Ϣ(�����ط�ʽ���������λ�������hWrite)
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    GetStartupInfo(&si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;

    //����������
    PROCESS_INFORMATION pi;
    TCHAR *pszCmdLine = pszCmd.GetBuffer(pszCmd.GetLength());
    TCHAR *pszPath = szPath.GetBuffer(szPath.GetLength());
    if (!CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE, NULL, NULL, pszPath, &si, &pi))
    {
        return TEXT("cannot create process");
    }

    //�����ر�hWrite
    CloseHandle(hWrite);

    //��ȡ�����з���ֵ
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

// ��ȡĬ�Ϸ�֧�ַ���
CString GetDefaultBranch(CString strSrc)
{
    // ����*����ֵĬ�Ϸ�֧
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

// �ָ�CString�ַ���
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

//�����ļ�������Ϊ���ļ���
bool DownloadSaveFiles(LPCTSTR url,LPCTSTR strSaveFile) {
	bool ret=false;
	CInternetSession Sess(TEXT("lpload"));
	Sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT     , 2000); //2������ӳ�ʱ
	Sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT        , 2000); //2��ķ��ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT     , 2000); //2��Ľ��ճ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT   , 2000); //2��ķ��ͳ�ʱ
	Sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 2000); //2��Ľ��ճ�ʱ
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
				//��ѯ�ļ�����
				DWORD nLen=0;
				cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
				//CString strFilename = GetFileName(url,TRUE);
				nBufLen=nLen;
				if (nLen <= 0) break;//

				//����������ݻ���
				pBuf = (char*)malloc(nLen+8);
				ZeroMemory(pBuf,nLen+8);

				char *p=pBuf;
				while (nLen>0) {
					//ÿ������8K
					int n = cFile->Read(p,(nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
					//��������˳�ѭ��
					if (n <= 0) break;//
					//���ջ������
					p+= n ;
					//ʣ�೤�ȵݼ�
					nLen -= n ;
				}

				//���δ�������ж��˳�
				if (nLen != 0) break;

				//���ճɹ����浽�ļ�

				CFile file(strSaveFile, CFile::modeCreate | CFile::modeWrite);
				file.Write(pBuf,nBufLen);
				file.Close();
				ret = true;
			}
		} catch(...) {
			break;//
		}
	} while(0);

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
	return ret;
}