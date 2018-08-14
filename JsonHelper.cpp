#include "stdafx.h"
#include "JsonHelper.h"

bool CJsonHelper::LoadJson(LPCTSTR lpFile, Json::Value& jvRoot)
{
    bool bLoadSucc = false;

    if (PathFileExists(lpFile) == FALSE)
    {
        return bLoadSucc;
    }

    try
    {
        Json::Reader _reader;

        jvRoot = Json::Value(Json::nullValue);

        std::ifstream _config;

        _config.open(lpFile, std::ios::in);
        bLoadSucc = _reader.parse(_config, jvRoot);
    }
    catch(...)
    {
        return false;
    }

    return bLoadSucc;
}

bool CJsonHelper::LoadJson(std::wstring &strDoc, Json::Value& jvRoot)
{
    bool bLoadSucc = false;

    std::string strTmp = CFunction::ws2s(strDoc);

    if (strDoc.empty())
    {
        return bLoadSucc;
    }

    try
    {
        Json::Reader _reader;

        jvRoot = Json::Value(Json::nullValue);

        bLoadSucc = _reader.parse(strTmp, jvRoot);
        strDoc = CFunction::s2ws(strTmp);
    }
    catch(...)
    {
        return false;
    }

    return bLoadSucc;
}