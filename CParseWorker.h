#pragma once

#include <atlutil.h>

class CTaskBase;
class CTask;

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

class CTask : public CTaskBase
{
public:
    CTask(CString url, CString strSavePath);

    void DoTask(void *pvParam, OVERLAPPED *pOverlapped);

private:
    CString m_strUrl;
    CString m_strSavePath;
};

