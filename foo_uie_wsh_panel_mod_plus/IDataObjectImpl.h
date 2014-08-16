#pragma once

/* The IDataObjectImpl is taken partially from chromium source, following BSD style License */

#include <ObjIdl.h>
#include <ShlObj.h>

#include "com_tools.h"


class DataObjectImpl : public IDataObject, public IAsyncOperation
{
private:
    BEGIN_COM_QI_IMPL()
        COM_QI_ENTRY_MULTI(IUnknown, IDataObject)
        COM_QI_ENTRY(IAsyncOperation)
    END_COM_QI_IMPL()

protected:
    DataObjectImpl();
    virtual ~DataObjectImpl();

public:
    // IDataObject
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium);
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc);
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut);
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

    // IAsyncOperation
    HRESULT STDMETHODCALLTYPE SetAsyncMode(BOOL fDoOpAsync);
    HRESULT STDMETHODCALLTYPE GetAsyncMode(BOOL *pfIsOpAsync);
    HRESULT STDMETHODCALLTYPE StartOperation(IBindCtx *pbcReserved);
    HRESULT STDMETHODCALLTYPE InOperation(BOOL *pfInAsyncOp);
    HRESULT STDMETHODCALLTYPE EndOperation(HRESULT hResult, IBindCtx *pbcReserved, DWORD dwEffects);

private:
    struct StoredDataInfo
    {
        FORMATETC formatEtc;
        STGMEDIUM *pMedium;
        bool isManaged;
        bool isInDelayRendering;

        StoredDataInfo(CLIPFORMAT cf, STGMEDIUM *medium) 
            : pMedium(pMedium)
            , isManaged(true)
            , isInDelayRendering(false)
        {
            formatEtc.cfFormat = cf;
            formatEtc.dwAspect = DVASPECT_CONTENT;
            formatEtc.lindex = -1;
            formatEtc.ptd = NULL;
            formatEtc.tymed = pMedium ? pMedium->tymed : TYMED_HGLOBAL;
        }

        StoredDataInfo(FORMATETC *pFormatEtc, STGMEDIUM *medium)
            : pMedium(pMedium)
            , formatEtc(*pFormatEtc)
            , isManaged(true)
            , isInDelayRendering(false)
        {

        }

        ~StoredDataInfo()
        {
            if (isManaged) 
            {
                ReleaseStgMedium(pMedium);
                delete pMedium;
            }
        }
    };

    typedef pfc::list_t<StoredDataInfo*> StoredData;
    StoredData m_contents;

    bool m_isInAsyncMode;
    bool m_isAsyncOperationStarted;
};
