#pragma once

#include "MyMacro.h"

class CFunction
{
public:
	CFunction(void);
	~CFunction(void);

public:
    // 执行命令行
	static CString			    ExeCmd(CString pszCmd, CString szPath);
    
    // 分割CString字符串
    static std::set<CString>    SplitCString(CString strSrc, CString strGap);			
    static std::vector<CString> SplitString(CString strSrc, CString strGap);

    // 从url中获取下载的压缩包文件名
    static CString GetNameFromUrl(CString strUrl);

    // 获取默认分支字符串
	static CString			    GetDefaultBranch(CString strSrc);

    // 删除非空文件夹
	static BOOL				    RemoveDir(CString strFileDir);

    // wstring转string
    static std::string          ws2s(const std::wstring& ws);

    // string转wstring
    static std::wstring         s2ws(const std::string& s);

    static std::string&         stolower(std::string& str);

};
