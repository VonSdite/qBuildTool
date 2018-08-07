#pragma once

/// 注意下面两文件的路径，必须在工程包含
#include <fstream>
#include <string>

#include "json/json.h"

class CJsonHelper
{
public:

	static bool LoadJson(LPCTSTR lpFile, Json::Value& jvRoot);

	static bool LoadJson(const std::string &strDoc, Json::Value& jvRoot);

	static bool WriteJson(LPCTSTR lpFile, const Json::Value &jvRoot);

	static bool WriteJson(const Json::Value &jvRoot,CStringA& strJsonA);

	static INT GetJsonValueINT(LPCTSTR lpKey, const Json::Value& jvRoot, DWORD dwDefault = 0);
	
	static CString GetJsonValueString(LPCTSTR lpKey, const Json::Value& jvRoot, CString strDefault = CString());

	static CStringA GetJsonValueStringA(LPCSTR lpKey, const Json::Value& jvRoot, CStringA strDefault = CStringA());
	
	static BOOL GetJsonValueArray(LPCSTR lpKey, const Json::Value& jvRoot, Json::Value& jvArray);

	static bool WriteJsonValueInt(LPCTSTR lpKey, Json::Value& jvRoot, INT nValue);

	static bool WriteJsonValueString(LPCTSTR lpKey, Json::Value& jvRoot, LPCTSTR lpValue);

	static bool GetJsonObject(const Json::Value &jsParent, LPCSTR lpKey, Json::Value &jsObject);

};

