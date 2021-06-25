#include "NetworkTaskToolBase.h"
#include <QObject>
#include "NetworkTaskMacros.h"

QString CNetworkTaskToolBase::errorDescription(int errorCode)
{
    if (errorCode > 0)
    {
        return NetworkTask::kHttpErrorCode + ": " + QString::number(errorCode);
    }

    switch (errorCode)
    {
        case NetworkTask::NoError:		    return NetworkTask::kNoError;
        case NetworkTask::InvalidParam:		return NetworkTask::kInvalidParameter;
        case NetworkTask::TimeOut:			return NetworkTask::kTimeOut;
        case NetworkTask::EmptyData:		return NetworkTask::kEmptyData;
    }

    return NetworkTask::kUnknownError;
}