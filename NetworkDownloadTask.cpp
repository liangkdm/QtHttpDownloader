#include "NetworkDownloadTask.h"
#include "NetworkTaskFunctions.h"
#include <QFile>
//#include "TestNetworkTaskMacroDefine.h"

CNetworkDownloadTask::CNetworkDownloadTask(const NetworkTask::stDownloadParam& param, QObject* parent)
    : INetworkTaskBase(param.taskType(), &param, parent)
    , m_param(param)
{
    if (param.localSaveFullPath != "") {
        m_saveFile = std::make_unique<QFile>(param.localSaveFullPath);
    }
}

CNetworkDownloadTask::CNetworkDownloadTask(const NetworkTask::stDownloadParam& param, const QString& taskID, QObject* parent)
    : INetworkTaskBase(taskID, param.taskType(), &param, parent)
    , m_param(param)
{
    if (param.localSaveFullPath != "") {
        m_saveFile = std::make_unique<QFile>(param.localSaveFullPath);
    }
}

CNetworkDownloadTask::~CNetworkDownloadTask()
{
    if (m_saveFile && m_saveFile->isOpen()) {
        m_saveFile->flush();
        m_saveFile->close();
        m_saveFile.release();
    }
}

bool CNetworkDownloadTask::startTask()
{
    Q_ASSERT(!currentReply());
    Q_ASSERT(INetworkTaskBase::networkManagerInstance());

    m_saveFile->open(QIODevice::WriteOnly);

    if (!checkParam(&m_param))
    {
        m_saveFile->close();
        endTask();
        return false;
    }

    INetworkTaskBase::startTask();

    const bool isHttps = (QUrl(m_param.downloadUrl).scheme().toLower() == "https");

    QNetworkRequest request;

    if (isHttps)
    {
        NetworkTask::createSSLRequest(request, m_param.downloadUrl);
    }
    else
    {
        request.setUrl(m_param.downloadUrl);
    }

    setCurrentReply(INetworkTaskBase::networkManagerInstance()->get(request));

    return true;
}

void CNetworkDownloadTask::downloadDataReady(const QVariant& data)
{
    if (m_saveFile && m_saveFile->isOpen())
    {
        m_saveFile->write(data.toByteArray());
    }
}



void CNetworkDownloadTask::onReplyFinished()
{
    if (m_saveFile != nullptr && m_saveFile->isOpen()) {
        m_saveFile->flush();
        m_saveFile->close();
    }

    Q_ASSERT(currentReply());

    INetworkTaskBase::onReplyFinished();
    if (isTimeOut())
    {
        return;
    }
    QNetworkReply::NetworkError httpCode = currentReply()->error();

    if(QNetworkReply::NoError != httpCode)
    {
        resolveWhenHttpError(httpCode);
        return;
    }

    taskResult().success = true;

    AFTER_TASK_FINISHED();

    currentReply()->deleteLater();
}