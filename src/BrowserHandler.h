#ifndef BROWSER_HANDLER_H_
#define BROWSER_HANDLER_H_

#include "include/cef_client.h"
#include "include/cef_browser.h"

#include <list>

class BrowserHandler : public CefClient,
                       public CefLifeSpanHandler,
                       public CefLoadHandler
{
    public:

    BrowserHandler(const CefString& pdfOutput, CefPdfPrintSettings pdfSettings);

    // CefClient methods:
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE;
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE;

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

    // CefLoadHandler methods:
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) OVERRIDE;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;
    virtual void OnLoadError(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl
    ) OVERRIDE;

    private:

    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList m_browsers;

    CefRefPtr<CefRenderHandler> m_renderHandler;

    CefString m_pdfOutput;
    CefPdfPrintSettings m_pdfSettings;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(BrowserHandler);
};

#endif // BROWSER_HANDLER_H_