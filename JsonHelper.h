#pragma once

class CJsonHelper
{
public:
	static bool LoadJson(LPCTSTR lpFile, Json::Value& jvRoot);
	static bool LoadJson(std::wstring &strDoc, Json::Value& jvRoot);
};

