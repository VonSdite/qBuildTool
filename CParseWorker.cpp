#include "stdafx.h"
#include "CParseWorker.h"

CString DownloadSaveFile(CString url, CString strSavePath);		
std::set<CString> unzip(CString strSaveFile);  

CDownloadTask::CDownloadTask(HWND hWnd, CString strUrl, CString strSavePath) : m_hWnd(hWnd), m_strUrl(strUrl), m_strSavePath(strSavePath) 
{
}

void CDownloadTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    CString *strDownloadFileName; 
    strDownloadFileName = new CString(DownloadSaveFile(m_strUrl, m_strSavePath));
    if (strDownloadFileName->IsEmpty())
    {

        delete strDownloadFileName;
    }
    else
    {
        PostMessage(m_hWnd, WM_SUCCESS_DOWNLOAD, (WPARAM)strDownloadFileName, NULL);
    }
}

CUnzipTask::CUnzipTask(HWND hWnd, CString strFilePath) : m_hWnd(hWnd), m_strFilePath(strFilePath)
{
}

void CUnzipTask::DoTask(void *pvParam, OVERLAPPED *pOverlapped)
{
    unzip(m_strFilePath);
}

#include <stdlib.h>
#include <time.h>

//�����ļ�
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
    strSaveFile = strSavePath + L"\\" + strSaveFile;

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
    try{
        cFile = (CHttpFile*)Sess.OpenURL(strUrl, 1, dwFlag);
        DWORD dwStatusCode;
        cFile->QueryInfoStatusCode(dwStatusCode);
        if (dwStatusCode == HTTP_STATUS_OK) {
            //��ѯ�ļ�����
            DWORD nLen=0;
            cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
            //CString strFilename = GetFileName(url,TRUE);
            nBufLen=nLen;
            if (nLen <= 0) return L"";//

            //����������ݻ���
            pBuf = (char*)malloc(nLen+8);
            ZeroMemory(pBuf,nLen+8);

            char *p=pBuf;
            while (nLen>0) {
                //ÿ������8K
                int n = cFile->Read(p, (nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
                //��������˳�ѭ��
                if (n <= 0) return L"";//
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
        return L"";//
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
std::set<CString> unzip(CString strSaveFile)
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
    char *fileData = new char[500*1024];

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
        setFileName.insert(strFileName);
        CString strTmpSaveFileName = strSaveFile.Left(strSaveFile.ReverseFind(L'\\') + 1) + strFileName;
        CFile file(strTmpSaveFileName, CFile::modeCreate | CFile::modeWrite);
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