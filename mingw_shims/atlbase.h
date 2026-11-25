// atlbase.h - MinGW 用シム (mingw-w64 には ATL が無い)
// ffdshow が使うのは CComPtr / CComQIPtr だけ (DXVA 系ヘッダ3枚)。
// ATL 本家と同じ意味論で最小限を実装する。
#pragma once
#include <unknwn.h>
#include <objbase.h>

template <class T>
class CComPtr
{
public:
    T *p;
    CComPtr(void) : p(NULL) {}
    CComPtr(T *lp) : p(lp) {
        if (p) {
            p->AddRef();
        }
    }
    CComPtr(const CComPtr<T> &lp) : p(lp.p) {
        if (p) {
            p->AddRef();
        }
    }
    ~CComPtr(void) {
        if (p) {
            p->Release();
        }
    }
    operator T*(void) const {
        return p;
    }
    T& operator*(void) const {
        return *p;
    }
    T** operator&(void) {
        return &p;
    }
    T* operator->(void) const {
        return p;
    }
    T* operator=(T *lp) {
        if (lp) {
            lp->AddRef();
        }
        if (p) {
            p->Release();
        }
        p = lp;
        return p;
    }
    T* operator=(const CComPtr<T> &lp) {
        return operator=(lp.p);
    }
    bool operator==(T *lp) const {
        return p == lp;
    }
    bool operator!=(T *lp) const {
        return p != lp;
    }
    bool operator<(T *lp) const {
        return p < lp;
    }
    void Release(void) {
        if (p) {
            p->Release();
            p = NULL;
        }
    }
    void Attach(T *lp) {
        if (p) {
            p->Release();
        }
        p = lp;
    }
    T* Detach(void) {
        T *lp = p;
        p = NULL;
        return lp;
    }
    HRESULT CopyTo(T **ppT) const {
        if (!ppT) {
            return E_POINTER;
        }
        *ppT = p;
        if (p) {
            p->AddRef();
        }
        return S_OK;
    }
    HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) {
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
    }
    HRESULT QueryInterface(REFIID iid, void **ppvObject) const {
        return p->QueryInterface(iid, ppvObject);
    }
    template <class Q>
    HRESULT QueryInterface(Q **pp) const {
        return p->QueryInterface(__uuidof(Q), (void**)pp);
    }
};

// 本家の CComQIPtr は第2テンプレート引数に &__uuidof(T) を取るが、
// MinGW の __uuidof は関数呼び出しに展開されるためテンプレート引数にできない。
// ここでは IID を実行時に __uuidof(T) から取る形にする (意味論は同じ)。
template <class T>
class CComQIPtr : public CComPtr<T>
{
public:
    CComQIPtr(void) {}
    CComQIPtr(T *lp) : CComPtr<T>(lp) {}
    CComQIPtr(const CComQIPtr<T> &lp) : CComPtr<T>(lp.p) {}
    CComQIPtr(IUnknown *lp) {
        if (lp) {
            lp->QueryInterface(__uuidof(T), (void**)&this->p);
        }
    }
    T* operator=(T *lp) {
        return CComPtr<T>::operator=(lp);
    }
    T* operator=(const CComQIPtr<T> &lp) {
        return CComPtr<T>::operator=(lp.p);
    }
    T* operator=(IUnknown *lp) {
        T *lp2 = NULL;
        if (lp) {
            lp->QueryInterface(__uuidof(T), (void**)&lp2);
        }
        return CComPtr<T>::operator=(lp2);
    }
};
