#ifndef NetworkTaskToolBase_h__
#define NetworkTaskToolBase_h__

#include <QString>
#include <QObject>



namespace NetworkTask
{
    const QString kHttpErrorCode = QObject::tr("HTTP error code");
    const QString kNoError = QObject::tr("no error");
    const QString kUnknownError = QObject::tr("unknown error");
    const QString kInvalidParameter = QObject::tr("invalid parameter");
    const QString kTimeOut = QObject::tr("time out");
    const QString kEmptyData = QObject::tr("empty data");
}


class CNetworkTaskToolBase
{
public:
    //error description
    virtual QString errorDescription(int errorCode);
};

#endif // NetworkTaskToolBase_h__
