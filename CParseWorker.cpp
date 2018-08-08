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
    strSaveFile = strSavePath + L"\\" + strSaveFile;

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
    try{
        cFile = (CHttpFile*)Sess.OpenURL(strUrl, 1, dwFlag);
        DWORD dwStatusCode;
        cFile->QueryInfoStatusCode(dwStatusCode);
        if (dwStatusCode == HTTP_STATUS_OK) {
            //查询文件长度
            DWORD nLen=0;
            cFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, nLen);
            //CString strFilename = GetFileName(url,TRUE);
            nBufLen=nLen;
            if (nLen <= 0) return L"";//

            //分配接收数据缓存
            pBuf = (char*)malloc(nLen+8);
            ZeroMemory(pBuf,nLen+8);

            char *p=pBuf;
            while (nLen>0) {
                //每次下载8K
                int n = cFile->Read(p, (nLen<RECVPACK_SIZE)?nLen:RECVPACK_SIZE);
                //接收完成退出循环
                if (n <= 0) return L"";//
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
        return L"";//
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
    char *fileData = new char[500*1024];

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