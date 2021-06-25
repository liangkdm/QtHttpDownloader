#ifndef NetworkTaskFunctions_h__
#define NetworkTaskFunctions_h__

#include <QNetworkRequest>

namespace NetworkTask
{
    //create ssl request
    void createSSLRequest(QNetworkRequest& request, const QUrl& url);
    QNetworkRequest createSSLRequest(const QString& strUrl);
}

#endif // NetworkTaskFunctions_h__
