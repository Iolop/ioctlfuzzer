#include "stdafx.h"
//--------------------------------------------------------------------------------------
/** 
* �������� � ������� xml ���������
* @param data ����� ������������ xml ����������
* @return TRUE ���� �� ��, FALSE � ������ ������
*/
BOOL XmlLoad(PWSTR lpwcData, IXMLDOMDocument **pXMLDoc, IXMLDOMNode **pIDOMRootNode, PWSTR lpwcRootNodeName)
{
    BOOL bOk = FALSE;
    VARIANT_BOOL status;

    // initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        DbgMsg(__FILE__, __LINE__, "CoInitializeEx() ERROR 0x%.8x\n", hr);
        return FALSE;
    }    

    // create new msxml document instance
    hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
        IID_IXMLDOMDocument, (void **)pXMLDoc);
    if (FAILED(hr)) 
    {
        DbgMsg(__FILE__, __LINE__, "CoCreateInstance() ERROR 0x%.8x\n", hr);
        return FALSE;
    }    

    hr = (*pXMLDoc)->loadXML(lpwcData, &status);
    if (status != VARIANT_TRUE)
    {
        DbgMsg(__FILE__, __LINE__, "pXMLDoc->load() ERROR 0x%.8x\n", hr);
        goto end;
    }

    // ���� xml ��������, �������� ������ �������� �����
    // �� �������� �������� ������� ������� 'logger'
    IXMLDOMNodeList *pIDOMRootNodeList;
    hr = (*pXMLDoc)->get_childNodes(&pIDOMRootNodeList);
    if (SUCCEEDED(hr))
    {
        *pIDOMRootNode = ConfGetListNodeByName(lpwcRootNodeName, pIDOMRootNodeList);
        if (*pIDOMRootNode)
        {
            bOk = TRUE;
        }            

        pIDOMRootNodeList->Release();        
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "pXMLDoc->get_childNodes() ERROR 0x%.8x\n", hr);
    }    

end:

    if (!bOk)
    {
        // ��������� ������
        // ����������� ���������� ����������
        (*pXMLDoc)->Release();
        *pXMLDoc = NULL;
    }

    return bOk;
}
//--------------------------------------------------------------------------------------
/** 
 * ��������� xml-���� �� ������ �� ��� �����
 * @param NodeName ��� �������� ����
 * @param pIDOMNodeList ���������� ������
 * @return ���������� ������� ����, ��� NULL � ������ �������
 * @see ConfGetNodeByName() 
 * @see ConfGetNodeText() 
 * @see ConfGetTextByName()
 */
IXMLDOMNode * ConfGetListNodeByName(BSTR NodeName, IXMLDOMNodeList *pIDOMNodeList)
{    
    IXMLDOMNode *Ret = NULL;
    LONG len = 0;
    
    if (pIDOMNodeList == NULL)
    {
        return NULL;
    }

    HRESULT hr = pIDOMNodeList->get_length(&len);
    if (SUCCEEDED(hr))
    {
        pIDOMNodeList->reset();
        for (int i = 0; i < len; i++)
        {
            IXMLDOMNode *pIDOMChildNode = NULL;
            hr = pIDOMNodeList->get_item(i, &pIDOMChildNode);
            if (SUCCEEDED(hr))
            {
                BSTR ChildNodeName = NULL;
                hr = pIDOMChildNode->get_nodeName(&ChildNodeName);
                if (SUCCEEDED(hr))
                {
                    if (!wcscmp(NodeName, ChildNodeName))
                    {
                        Ret = pIDOMChildNode;
                    }
                }                

                if (ChildNodeName)
                {
                    SysFreeString(ChildNodeName);                    
                }

                if (Ret)
                {
                    return Ret;
                }

                pIDOMChildNode->Release();
                pIDOMChildNode = NULL;                
            } 
            else 
            {
                DbgMsg(__FILE__, __LINE__, "pIDOMNodeList->get_item() ERROR 0x%.8x\n", hr);
            }
        }
    } 
    else 
    {
        DbgMsg(__FILE__, __LINE__, "pIDOMNodeList->get_length() ERROR 0x%.8x\n", hr);
    }

    return NULL;
}
//--------------------------------------------------------------------------------------
/** 
 * ��������� ������� �� ��� �����
 * @param NodeName ��� �������� ����
 * @param pIDOMNode ���������� ������������� ����
 * @return ���������� ������� ����, ��� NULL � ������ �������
 * @see ConfGetListNodeByName()  
 * @see ConfGetNodeText() 
 * @see ConfGetTextByName()
 */
IXMLDOMNode * ConfGetNodeByName(BSTR NodeName, IXMLDOMNode *pIDOMNode)
{
    IXMLDOMNode *pIDOMRetNode = NULL;
    IXMLDOMNodeList *pIDOMNodeList = NULL;

    if (pIDOMNode == NULL)
    {
        return NULL;
    }

    HRESULT hr = pIDOMNode->get_childNodes(&pIDOMNodeList);
    if (SUCCEEDED(hr) && pIDOMNodeList)
    {
        pIDOMRetNode = ConfGetListNodeByName(NodeName, pIDOMNodeList);
        pIDOMNodeList->Release();        
    } 
    else 
    {
        DbgMsg(__FILE__, __LINE__, "pIDOMNodeList->get_length() ERROR 0x%.8x\n", hr);
    }

    return pIDOMRetNode;
} 
//--------------------------------------------------------------------------------------
/** 
 * ��������� �������� ����
 * @param pIDOMNode ���������� ����
 * @param str ������ unicode-������, � ������� ����� �������� ��������
 * @return TRUE ���� �� ��, FALSE � ������ ������
 * @see ConfGetListNodeByName() 
 * @see ConfGetNodeByName() 
 * @see ConfGetTextByName()
 */
BOOL ConfGetNodeTextW(IXMLDOMNode *pIDOMNode, PWSTR *str)
{
    BOOL bRet = FALSE;
    BSTR val = NULL;

    if (pIDOMNode == NULL)
    {
        return FALSE;
    }

    HRESULT hr = pIDOMNode->get_text(&val);
    if (FAILED(hr))
    {
        DbgMsg(__FILE__, __LINE__, "pIDOMNode->get_text() ERROR 0x%.8x\n", hr);
        return FALSE;
    }

    DWORD Len = (wcslen((PWSTR)val) + 1) * sizeof(WCHAR);
    if (*str = (PWSTR)M_ALLOC(Len))
    {
        ZeroMemory(*str, Len);
        wcscpy_s(*str, Len / sizeof(wchar_t), (PWSTR)val);
        bRet = TRUE;
    }
    else
    {
        DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
    }

    if (val)
    {
        SysFreeString(val);        
    }            

    return bRet;
}
//--------------------------------------------------------------------------------------
/** 
 * ��������� �������� ����
 * @param pIDOMNode ���������� ����
 * @param str ������ unicode-������, � ������� ����� �������� ��������
 * @return TRUE ���� �� ��, FALSE � ������ ������
 * @see ConfGetListNodeByName() 
 * @see ConfGetNodeByName() 
 * @see ConfGetTextByName()
 */
BOOL ConfGetNodeTextA(IXMLDOMNode *pIDOMNode, PCHAR *str)
{
    BOOL bRet = FALSE;
    PWSTR str_w;

    if (ConfGetNodeTextW(pIDOMNode, &str_w))
    {
        int len = wcslen(str_w);
        if (*str = (PCHAR)M_ALLOC(len + 1))
        {
            ZeroMemory(*str, len + 1);
            WideCharToMultiByte(CP_ACP, 0, str_w, -1, *str, len, NULL, NULL);    
            bRet = TRUE;
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
        }

        M_FREE(str_w);
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
/** 
 * ��������� �������� ������� �� ��� �����
 * @param pIDOMNode ���������� ������������� ����
 * @param name ��� ��������� ����, �������� �������� ���������� ��������
 * @param val ������ ��������� �� unicode-������, � ������� ����� �������� ��������
 * @return TRUE ���� �� ��, FALSE � ������ ������
 * @see ConfGetListNodeByNameA() 
 * @see ConfGetListNodeByName() 
 * @see ConfGetNodeByName() 
 * @see ConfGetNodeText() 
 * @see ConfGetTextByName()
 */
BOOL ConfAllocGetTextByNameW(IXMLDOMNode *pIDOMNode, PWSTR name, PWSTR *value)
{
    BOOL bRet = FALSE;
    
    IXMLDOMNode *pIDOMChildNode = ConfGetNodeByName(name, pIDOMNode);
    if (pIDOMChildNode)
    {
        bRet = ConfGetNodeTextW(pIDOMChildNode, value);        
    
        pIDOMChildNode->Release();
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
/** 
 * ��������� �������� ������� �� ��� �����
 * @param pIDOMNode ���������� ������������� ����
 * @param name ��� ��������� ����, �������� �������� ���������� ��������
 * @param val ������ ��������� �� unicode-������, � ������� ����� �������� ��������
 * @return TRUE ���� �� ��, FALSE � ������ ������
 * @see ConfGetListNodeByNameW() 
 * @see ConfGetListNodeByName() 
 * @see ConfGetNodeByName() 
 * @see ConfGetNodeText() 
 * @see ConfGetTextByName()
 */
BOOL ConfAllocGetTextByNameA(IXMLDOMNode *pIDOMNode, PWSTR name, PCHAR *value)
{
    BOOL bRet = FALSE;
    PWSTR value_w;

    if (ConfAllocGetTextByNameW(pIDOMNode, name, &value_w))
    {
        int len = wcslen(value_w);
        if (*value = (PCHAR)M_ALLOC(len + 1))
        {
            ZeroMemory(*value, len + 1);
            WideCharToMultiByte(CP_ACP, 0, value_w, -1, *value, len, NULL, NULL);    
            bRet = TRUE;
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
        }

        M_FREE(value_w);
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
BOOL ConfGetNodeAttributeW(IXMLDOMNode *pIDOMNode, PWSTR name, PWSTR *value)
{
    BOOL bRet = FALSE;
    IXMLDOMNamedNodeMap *pIXMLDOMNamedNodeMap = NULL;

    // query attributes map
    HRESULT hr = pIDOMNode->get_attributes(&pIXMLDOMNamedNodeMap);
    if (SUCCEEDED(hr) && pIXMLDOMNamedNodeMap)
    {
        IXMLDOMNode *pIDOMAttrNode = NULL;

        // query attribute node
        hr = pIXMLDOMNamedNodeMap->getNamedItem(name, &pIDOMAttrNode);
        if (SUCCEEDED(hr) && pIDOMAttrNode)
        {
            VARIANT varValue;
            hr = pIDOMAttrNode->get_nodeValue(&varValue);
            if (FAILED(hr))
            {
                DbgMsg(__FILE__, __LINE__, "pIDOMAttrNode->get_nodeValue() ERROR 0x%.8x\n", hr);
                goto free;
            }

            BSTR val = _bstr_t(varValue);
            DWORD Len = (wcslen((PWSTR)val) + 1) * sizeof(WCHAR);
            if (*value = (PWSTR)M_ALLOC(Len))
            {
                ZeroMemory(*value, Len);
                wcscpy(*value, (PWSTR)val);
                bRet = TRUE;
            }
            else
            {
                DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
            }
free:
            pIDOMAttrNode->Release();
            pIDOMAttrNode = NULL;
        }

        pIXMLDOMNamedNodeMap->Release();
        pIXMLDOMNamedNodeMap = NULL;
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
BOOL ConfGetNodeAttributeA(IXMLDOMNode *pIDOMNode, PWSTR name, PCHAR *value)
{
    BOOL bRet = FALSE;
    PWSTR value_w;

    if (ConfGetNodeAttributeW(pIDOMNode, name, &value_w))
    {
        int len = wcslen(value_w);
        if (*value = (PCHAR)M_ALLOC(len + 1))
        {
            ZeroMemory(*value, len + 1);
            WideCharToMultiByte(CP_ACP, 0, value_w, -1, *value, len, NULL, NULL);    
            bRet = TRUE;
        }
        else
        {
            DbgMsg(__FILE__, __LINE__, "M_ALLOC() ERROR %d\n", GetLastError());
        }

        M_FREE(value_w);
    }

    return bRet;
}
//--------------------------------------------------------------------------------------
// EoF
