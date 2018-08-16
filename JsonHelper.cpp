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
