#include "StdAfx.h"
#include "Function.h"

CFunction::CFunction(void)
{
}

CFunction::~CFunction(void)
{
}

// ִ��cmd�����ȡ����ֵ
CString CFunction::ExeCmd(CString pszCmd, CString szPath)
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

// �ָ�CString�ַ���
std::set<CString> CFunction::SplitCString(CString strSrc, CString strGap)
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

// ��ȡĬ�Ϸ�֧�ַ���
CString CFunction::GetDefaultBranch(CString strSrc)
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
