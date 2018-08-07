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

bool CJsonHelper::LoadJson(const std::string &strDoc, Json::Value& jvRoot)
{
    bool bLoadSucc = false;

    if (strDoc.empty())
    {
        return bLoadSucc;
    }

    try
    {
        Json::Reader _reader;

        jvRoot = Json::Value(Json::nullValue);

        bLoadSucc = _reader.parse(strDoc, jvRoot);
    }
    catch(...)
    {
        return false;
    }

    return bLoadSucc;
}

bool CJsonHelper::WriteJson(LPCTSTR lpFile, const Json::Value &jvRoot)
{
    if (lpFile == NULL)
        return false;

    try
    {
        Json::StyledWriter writer;
        std::ofstream _config;

        std::string strBuff = writer.write(jvRoot);

        _config.open(lpFile, std::ios::out);
        _config.write(strBuff.c_str(), strBuff.size());
        _config.flush();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CJsonHelper::WriteJson(const Json::Value &jvRoot,CStringA& strJsonA)
{
    try
    {
        Json::StyledWriter writer;

        std::string strBuff = writer.write(jvRoot);
        strJsonA = strBuff.c_str();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

INT CJsonHelper::GetJsonValueINT(LPCTSTR lpKey, const Json::Value& jvRoot, DWORD dwDefault/* = 0*/)
{
    try
    {
        if (jvRoot.isNull() || !jvRoot.isObject())
        {
            return dwDefault;
        }

        if (lpKey == NULL ||  (!jvRoot.isMember(CStringA(lpKey))))
        {
            return dwDefault;
        }

        Json::Value _value = jvRoot[CStringA(lpKey)];
        if (!_value.isInt())
        {
            return dwDefault;
        }

        return _value.asInt();
    }
    catch(...)
    {
        return dwDefault;
    }
}


CString CJsonHelper::GetJsonValueString(LPCTSTR lpKey, const Json::Value& jvRoot, CString strDefault /*= CString()*/)
{
    try
    {
        if (jvRoot.isNull() || !jvRoot.isObject())
        {
            return strDefault;
        }

        if (lpKey == NULL || (!jvRoot.isMember(CStringA(lpKey))))
        {
            return strDefault;
        }	

        Json::Value _value = jvRoot[CStringA(lpKey)];
        if (_value.isNull() || !_value.isString())
        {
            return strDefault;
        }

        return CString(CA2T(_value.asCString(),CP_UTF8));
    }
    catch(...) 
    {
        return strDefault;
    }
}

CStringA CJsonHelper::GetJsonValueStringA(LPCSTR lpKey, const Json::Value& jvRoot, CStringA strDefault /*= CStringA()*/)
{
    try
    {
        if (jvRoot.isNull() || !jvRoot.isObject())
        {
            return strDefault;
        }

        if (lpKey == NULL || (!jvRoot.isMember(lpKey)))
        {
            return strDefault;
        }

        Json::Value _value = jvRoot[lpKey];
        if (_value.isNull() || !_value.isString())
        {
            return strDefault;
        }

        LPCSTR lpText = _value.asCString();
        return CStringA(lpText);
    }
    catch(...)
    {
        return strDefault;
    }
}

BOOL CJsonHelper::GetJsonValueArray(LPCSTR lpKey, const Json::Value& jvRoot, Json::Value& jvArray)
{
    try
    {
        if (jvRoot.isNull() || !jvRoot.isObject())
        {
            return FALSE;
        }

        if (lpKey == NULL || (!jvRoot.isMember(lpKey)))
        {
            return FALSE;
        }

        jvArray = jvRoot[lpKey];
        if (jvArray.isNull() || !jvArray.isArray())
        {
            return FALSE;
        }

        return TRUE;
    }
    catch(...)
    {
        return FALSE;
    }
}

bool CJsonHelper::WriteJsonValueInt(LPCTSTR lpKey, Json::Value& jvRoot, INT nValue)
{
    try
    {
        if (lpKey == NULL)
            return false;

        //if (!jvRoot[CStringA(lpKey)].isArray())
        jvRoot[CStringA(lpKey)] = nValue;


        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CJsonHelper::WriteJsonValueString(LPCTSTR lpKey, Json::Value& jvRoot, LPCTSTR lpValue)
{
    try
    {
        if (lpKey == NULL)
            return false;

        //if (!jvRoot[CStringA(lpKey)].isArray())
        jvRoot[CStringA(lpKey)] = (LPCSTR)CStringA(lpValue);

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool CJsonHelper::GetJsonObject(const Json::Value &jsParent, LPCSTR lpKey, Json::Value &jsObject)
{
    try
    {
        if (jsParent.isNull() || !jsParent.isObject() || lpKey==NULL)
        {
            return false;
        }

        jsObject = Json::nullValue;

        if (jsParent.isMember(lpKey))
        {
            jsObject = jsParent[lpKey];
        }

        return !jsObject.isNull();
    }
    catch(...)
    {
        return false;
    }
}
