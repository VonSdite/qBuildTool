#pragma once

#include "resource.h"

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
    DWORD	    m_dwExecs;
    LONG	    m_lId;
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
    CString     m_strSavePath;
    HWND        m_hWnd;
    CString     m_strUrl;

};

class CUnzipTask : public CTaskBase
{
public:
    CUnzipTask(HWND hWnd, CString strFilePath, const CString strBranch, const Json::Value &jvRoot);

    void DoTask(void *pvParam, OVERLAPPED *pOverlapped);

private:
    CString         m_strFilePath;
    HWND            m_hWnd;
    CString         m_strBranch;
    Json::Value     m_jvRoot;
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

class CPushTask : public CTaskBase
{
public:
    CPushTask(HWND hWnd, const Json::Value &jvRoot, const std::set<FILE_INFO *> &setFileInfo, const CString &strGitPath, const CString &strBranch, const CString &strNote);
    void DoTask(void *pvParam, OVERLAPPED *pOverlapped);
private:
	HWND					m_hWnd;
    Json::Value				m_jvRoot;
    std::set<FILE_INFO *>	m_setFileInfo;
    CString					m_strBranch;
    CString					m_strGitPath;
    CString					m_strNote;
};
