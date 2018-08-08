#pragma once

#include <atlutil.h>

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

private:
    CString m_strUrl;
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
