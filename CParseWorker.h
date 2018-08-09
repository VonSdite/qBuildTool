#pragma once

#include <atlutil.h>

#define WM_FILE_INFO_UPADTE     (WM_USER+1)
#define WM_SUCCESS_DOWNLOAD     (WM_USER+2)
#define WM_COMPLETE_DOWNLOAD    (WM_USER+4)
#define WM_SHOW_FILE_INFO       (WM_USER+5)

class CTaskBase;
class CDownloadTask;

class CParseWorker
{
public:
    typedef DWORD_PTR RequestType;

    CParseWorker();

    virtual BOOL Initialize(void *pvParam);

    virtual void Terminate(void* /*pvParam*/);

    void Execute(RequestType dw, void *pvParam, OVERLAPPED* pOverlapped) throw();

    virtual BOOL GetWorkerData(DWORD /*dwParam*/, void ** /*ppvData*/);

protected:
    DWORD	m_dwExecs;
    LONG	m_lId;
}; // CParseWorker

class CTaskBase
{
public:
    virtual void DoTask(void *pvParam, OVERLAPPED *pOverlapped)=0;
};

class CDownloadTask : public CTaskBase
{
public:
    CDownloadTask(HWND hWnd, CString url, CString strSavePath);

    void DoTask(void *pvParam, OVERLAPPED *pOverlapped);

    CString m_strUrl;
private:
    CString m_strSavePath;
    HWND m_hWnd;
};

class CUnzipTask : public CTaskBase
{
public:
    CUnzipTask(HWND hWnd, CString strFilePath);

    void DoTask(void *pvParam, OVERLAPPED *pOverlapped);

private:
    CString m_strFilePath;
    HWND m_hWnd;
};

struct FILE_INFO
{
    CString         strFileName;
    CString         strMd5;
    CString         strVersion;
    DWORD           dwSize;
    CString         strSignTime;
    BOOL            fUpdate;
    CString         strFileLocation;
};