#include "stdafx.h"
#include "IDataObjectImpl.h"


// The following function, DuplicateMedium, is derived from WCDataObject.cpp
// in the WebKit source code. This is the license information for the file:
/*
* Copyright (C) 2007 Apple Inc.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
static void DuplicateMedium(CLIPFORMAT source_clipformat,
    STGMEDIUM* source,
    STGMEDIUM* destination) {
        switch (source->tymed) {
        case TYMED_HGLOBAL:
            destination->hGlobal =
                static_cast<HGLOBAL>(OleDuplicateData(
                source->hGlobal, source_clipformat, 0));
            break;
        case TYMED_MFPICT:
            destination->hMetaFilePict =
                static_cast<HMETAFILEPICT>(OleDuplicateData(
                source->hMetaFilePict, source_clipformat, 0));
            break;
        case TYMED_GDI:
            destination->hBitmap =
                static_cast<HBITMAP>(OleDuplicateData(
                source->hBitmap, source_clipformat, 0));
            break;
        case TYMED_ENHMF:
            destination->hEnhMetaFile =
                static_cast<HENHMETAFILE>(OleDuplicateData(
                source->hEnhMetaFile, source_clipformat, 0));
            break;
        case TYMED_FILE:
            destination->lpszFileName =
                static_cast<LPOLESTR>(OleDuplicateData(
                source->lpszFileName, source_clipformat, 0));
            break;
        case TYMED_ISTREAM:
            destination->pstm = source->pstm;
            destination->pstm->AddRef();
            break;
        case TYMED_ISTORAGE:
            destination->pstg = source->pstg;
            destination->pstg->AddRef();
            break;
        }

        destination->tymed = source->tymed;
        destination->pUnkForRelease = source->pUnkForRelease;
        if (destination->pUnkForRelease)
            destination->pUnkForRelease->AddRef();
}

DataObjectImpl::DataObjectImpl() : m_isAsyncOperationStarted(false), m_isInAsyncMode(false)
{

}

DataObjectImpl::~DataObjectImpl()
{
    // Free all contents (pointers)
    m_contents.for_each([] (StoredDataInfo *pInfo) -> void { delete pInfo; });
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    if (!pformatetcIn) return E_INVALIDARG;
    if (!pmedium) return E_POINTER;

    unsigned count = m_contents.get_count();

    for (unsigned i = 0; i < count; i++)
    {
        StoredDataInfo *pInfo = m_contents[i];

        if (pInfo->formatEtc.cfFormat == pformatetcIn->cfFormat &&
            pInfo->formatEtc.lindex == pformatetcIn->lindex &&
            (pInfo->formatEtc.tymed & pformatetcIn->tymed)) 
        {
            // If medium is NULL, delay-rendering will be used.
            if (pInfo->pMedium) {
                DuplicateMedium(pInfo->formatEtc.cfFormat, pInfo->pMedium, pmedium);
            } else {
                // Check if the left button is down.
                bool isLeftButtonDown = (GetKeyState(VK_LBUTTON) & 0x8000) != 0;
                bool waitForData = false;

                if (pInfo->isInDelayRendering) 
                {
                    // Make sure the left button is up. Sometimes the drop target, like
                    // Shell, might be too aggresive in calling GetData when the left
                    // button is not released.
                    if (isLeftButtonDown) return DV_E_FORMATETC;

                    // In async mode, we do not want to start waiting for the data before
                    // the async operation is started. This is because we want to postpone
                    // until Shell kicks off a background thread to do the work so that
                    // we do not block the UI thread.
                    if (!m_isInAsyncMode || m_isAsyncOperationStarted)
                        waitForData = true;
                } else {
                    // If the left button is up and the target has not requested the data
                    // yet, it probably means that the target does not support delay-
                    // rendering. So instead, we wait for the data.
                    if (isLeftButtonDown) {
                        pInfo->isInDelayRendering = true;
                        memset(pmedium, 0, sizeof(STGMEDIUM));
                    } else {
                        waitForData = true;
                    }
                }

                if (!waitForData) return DV_E_FORMATETC;

                // The stored data should have been updated with the final version.
                // So we just need to call this function again to retrieve it.
                return GetData(pformatetcIn, pmedium);
            }
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    return DATA_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::QueryGetData(FORMATETC *pformatetc)
{
    if (!pformatetc) return E_INVALIDARG;

    unsigned count = m_contents.get_count();
    for (unsigned i = 0; i < count; i++)
    {
        StoredDataInfo *pInfo = m_contents[i];
        if (pInfo->formatEtc.cfFormat == pformatetc->cfFormat)
            return S_OK;
    }

    return DATA_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut)
{
    if (!pformatectIn) return E_INVALIDARG;
    pformatectIn->ptd = NULL;
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    // Remove data of the same type first
    if (!pformatetc->ptd)
    {
        unsigned count = m_contents.get_count();
        for (unsigned i = 0; i < count; i++)
        {
            StoredDataInfo* ptr = m_contents[i];
            if (pformatetc->ptd == NULL && 
                pformatetc->cfFormat == ptr->formatEtc.cfFormat &&
                pformatetc->dwAspect == ptr->formatEtc.dwAspect &&
                pformatetc->lindex == ptr->formatEtc.lindex &&
                pformatetc->tymed == ptr->formatEtc.tymed)
            {
                delete ptr;
                m_contents.remove_by_idx(i);
                break;
            }
        }
    }

    STGMEDIUM *pLocalMedium = new STGMEDIUM;
    if (fRelease)
    {
        *pLocalMedium = *pmedium;
    }
    else
    {
        DuplicateMedium(pformatetc->cfFormat, pmedium, pLocalMedium);
    }

    StoredDataInfo *pInfo = new StoredDataInfo(pformatetc->cfFormat, pLocalMedium);
    pInfo->pMedium->tymed = pformatetc->tymed;
    pInfo->isManaged = !!fRelease;
    m_contents.add_item(pInfo);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
{
    if (dwDirection == DATADIR_GET)
    {
        unsigned count = m_contents.get_count();
        pfc::array_staticsize_t<FORMATETC> fmt(count);
        for (unsigned i = 0; i < count; i++)
        {
            fmt[i] = m_contents[i]->formatEtc;
        }
        return SHCreateStdEnumFmtEtc(m_contents.get_count(), fmt.get_ptr(), ppenumFormatEtc);
    }

    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::SetAsyncMode(BOOL fDoOpAsync)
{
    m_isInAsyncMode = (fDoOpAsync == TRUE);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::GetAsyncMode(BOOL *pfIsOpAsync)
{
    if (!pfIsOpAsync) return E_POINTER;
    (*pfIsOpAsync) = m_isInAsyncMode ? TRUE : FALSE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::StartOperation(IBindCtx *pbcReserved)
{
    m_isAsyncOperationStarted = TRUE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::InOperation(BOOL *pfInAsyncOp)
{
    if (!pfInAsyncOp) return E_POINTER;
    (*pfInAsyncOp) = m_isAsyncOperationStarted ? TRUE : FALSE;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE DataObjectImpl::EndOperation(HRESULT hResult, IBindCtx *pbcReserved, DWORD dwEffects)
{
    m_isAsyncOperationStarted = false;
    return S_OK;
}
