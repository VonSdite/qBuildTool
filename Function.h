#pragma once

#include "MyMacro.h"

class CFunction
{
public:
	CFunction(void);
	~CFunction(void);

public:
    // ִ��������
	static CString			    ExeCmd(CString pszCmd, CString szPath);
    
    // �ָ�CString�ַ���
    static std::set<CString>    SplitCString(CString strSrc, CString strGap);			
    static std::vector<CString> SplitString(CString strSrc, CString strGap);

    // ��url�л�ȡ���ص�ѹ�����ļ���
    static CString GetNameFromUrl(CString strUrl);

    // ��ȡĬ�Ϸ�֧�ַ���
	static CString			    GetDefaultBranch(CString strSrc);

    // ɾ���ǿ��ļ���
	static BOOL				    RemoveDir(CString strFileDir);

    // wstringתstring
    static std::string          ws2s(const std::wstring& ws);

    // stringתwstring
    static std::wstring         s2ws(const std::string& s);

    static std::string&         stolower(std::string& str);

};
