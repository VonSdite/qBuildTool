#pragma once

#define RECVPACK_SIZE 2048

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

    // ��ȡĬ�Ϸ�֧�ַ���
	static CString			    GetDefaultBranch(CString strSrc);

    // ɾ���ǿ��ļ���
	static BOOL				    RemoveDir(CString strFileDir);

    // wstringתstring
    static std::string          ws2s(const std::wstring& ws);

    // stringתwstring
    static std::wstring         s2ws(const std::string& s);
};
