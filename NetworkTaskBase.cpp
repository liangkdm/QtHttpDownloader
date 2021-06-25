#include "ThreadTimer.h"
#include "NetworkTaskBase.h"
#include "NetworkTaskFunctions.h"
#include <QNetworkAccessManager>
#include <memory>
const int kTimeOutInterval = 15000;   //³¬Ê±Ê±¼ä(ºÁÃë)

namespace NetworkTask
{
    std::shared_ptr<CNetworkTaskToolBase> g_tool;
    std::shared_ptr<QNetworkAccessManager>g_accessManager;
}

void INetworkTaskBase::setNetworkManagerInstance(std::shared_ptr<QNetworkAccessManager> manager)
{
    NetworkTask::g_accessManager = manager;
}

QNetworkAccessManager* INetworkTaskBase::networkManagerInstance()
{
    return NetworkTask::g_accessManager.get();
}
void INetworkTaskBase::setTool(std::shared_ptr<CNetworkTaskToolBase> toolBase)
{
    NetworkTask::g_tool = toolBase;
}

std::shared_ptr<CNetworkTaskToolBase>  INetworkTaskBase::tool()
{
    return NetworkTask::g_tool;
}

INetworkTaskBase::INetworkTaskBase(int type, const NetworkTask::stNetworkParamBase* param, QObject* parent)
    : QObject(parent)
    , m_info(type)
    , m_reply(nullptr)
    , m_taskPriority(NetworkTask::Normal)
    , m_taskState(NetworkTask::Ready)
    , m_heartBreakTimer(nullptr)
{
    if (param)
    {
        m_taskPriority = param->taskPriority;
    }

    connect(this, &INetworkTaskBase::startTimerSignal, this, &INetworkTaskBase::startTimeoutTimer, Qt::QueuedConnection);
}

INetworkTaskBase::INetworkTaskBase(const QString& taskID, int type, const  NetworkTask::stNetworkParamBase* param, QObject* parent)
    : QObject(parent)
    , m_info(taskID, type)
    , m_reply(nullptr)
    , m_taskPriority(NetworkTask::Normal)
    , m_taskState(NetworkTask::Ready)
    , m_heartBreakTimer(nullptr)
{
    if (param)
    {
        m_taskPriority = param->taskPriority;
    }

    connect(this, &INetworkTaskBase::startTimerSignal, this, &INetworkTaskBase::startTimeoutTimer, Qt::QueuedConnection);
}

const NetworkTask::stNetworkResult& INetworkTaskBase::taskResult() const
{
    return m_result;
}

const NetworkTask::stNetworkTaskInfo& INetworkTaskBase::taskInfo() const
{
    return m_info;
}

void INetworkTaskBase::setCurrentReply(QNetworkReply* reply)
{
    if (reply) {
        connect(reply, &QNetworkReply::sslErrors, this, [=](const QList<QSslError> &errors) {
            if (!errors.isEmpty())
            {
                resolveWhenHttpError(reply->error());
            }
        });
    }
    if (reply && QNetworkReply::NoError != reply->error())
    {
        //处理异常
        resolveWhenHttpError(reply->error());
        return;
    }

    if (m_reply)
    {
        disconnect(m_reply, &QNetworkReply::finished, this, &INetworkTaskBase::onReplyFinished);
        disconnect(m_reply, &QNetworkReply::uploadProgress, this, &INetworkTaskBase::onUploadProgress);
        disconnect(m_reply, &QNetworkReply::downloadProgress, this, &INetworkTaskBase::onDownloadProgress);
        disconnect(m_reply, &QIODevice::readyRead, this, &INetworkTaskBase::OnReadyRead);
    }

    if (reply)
    {
        m_reply = reply;
        //保证该reply对象上的信号链接唯一
        connect(reply, &QNetworkReply::finished, this, &INetworkTaskBase::onReplyFinished, Qt::UniqueConnection);
        connect(reply, &QNetworkReply::uploadProgress, this, &INetworkTaskBase::onUploadProgress, Qt::UniqueConnection);
        connect(reply, &QNetworkReply::downloadProgress, this, &INetworkTaskBase::onDownloadProgress, Qt::UniqueConnection);
        connect(reply, &QIODevice::readyRead, this, &INetworkTaskBase::OnReadyRead, Qt::UniqueConnection);
    }
}

void INetworkTaskBase::endTask()
{
    if (nullptr != m_reply)
    {
        disconnect(m_reply, nullptr, nullptr, nullptr);
    }

    if (currentReply() && currentReply()->isRunning())
    {
        currentReply()->abort();
    }

    stopTimeoutTimer();
}

bool INetworkTaskBase::checkParam(const NetworkTask::stNetworkParamBase* param)
{
    if (!param || !param->isValid())
    {
        m_result.success = false;
        m_result.errorCode = NetworkTask::InvalidParam;
        m_result.errorMsg = tool()->errorDescription(NetworkTask::InvalidParam);

        AFTER_TASK_FINISHED();
        return false;
    }
    return true;
}

bool INetworkTaskBase::resolveWhenHttpError(QNetworkReply::NetworkError httpCode)
{
    if (QNetworkReply::NoError == httpCode)
    {
        return true;
    }

    m_result.success = false;
    m_result.errorCode = httpCode;
    m_result.errorMsg = tool()->errorDescription(httpCode);

    AFTER_TASK_FINISHED();
    endTask();
    return false;
}

void INetworkTaskBase::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    resetTimeoutTimer();
    if (bytesTotal == 0) {
        AFTER_TASK_FINISHED();
        endTask();
        return;
    }
    if (bytesReceived == bytesTotal) {
        disconnect(currentReply(), &QNetworkReply::downloadProgress, nullptr, nullptr);
    }
    TASK_DOWNLOAD_PROGRESS(bytesReceived, bytesTotal);
}

void INetworkTaskBase::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    TASK_UPLOAD_PROGRESS(bytesSent, bytesTotal);
    if (bytesSent == bytesTotal)
    {
        disconnect(currentReply(), &QNetworkReply::uploadProgress, nullptr, nullptr);
    }
}

void INetworkTaskBase::setTaskState(NetworkTask::ETaskStatus state)
{
    m_taskState = state;
}

void INetworkTaskBase::resetTimeoutTimer()
{
    if (m_heartBreakTimer)
    {
        setTimeOut(false);
        m_heartBreakTimer->restart();
    }
}

void INetworkTaskBase::startTimeoutTimer()
{
    if (!m_heartBreakTimer)
    {
        m_heartBreakTimer = std::make_unique<CThreadTimer>();
        connect(m_heartBreakTimer.get(), &CThreadTimer::timeout, this, &INetworkTaskBase::onTimeOut, Qt::QueuedConnection);
        m_heartBreakTimer->setInterval(kTimeOutInterval);
        m_heartBreakTimer->setSingleShot(true);
    }

    if (m_heartBreakTimer)
    {
        if (m_heartBreakTimer->isRunning())
            m_heartBreakTimer->quit();

        m_heartBreakTimer->start();
    }
}

void INetworkTaskBase::stopTimeoutTimer()
{
    if (m_heartBreakTimer)
    {
        m_heartBreakTimer->quit();
    }
}

void INetworkTaskBase::OnReadyRead()
{
    auto reply = currentReply();
    if (reply) {
        downloadDataReady(QVariant(reply->readAll()));
    }
}

void INetworkTaskBase::onTimeOut()
{
    //³¬Ê±
    setTimeOut(true);
    stopTimeoutTimer();

    m_result.success = false;
    m_result.errorCode = NetworkTask::TimeOut;
    m_result.errorMsg = tool()->errorDescription(NetworkTask::TimeOut);

    AFTER_TASK_FINISHED();
}

void INetworkTaskBase::onReplyFinished()
{
    disconnect(currentReply(), &QNetworkReply::finished, nullptr, nullptr);
    stopTimeoutTimer();
}

bool INetworkTaskBase::startTask()
{
    setTimeOut(false);
    setTaskState(NetworkTask::Processing);
    Q_EMIT startTimerSignal();
    return true;
}

INetworkTaskBase::~INetworkTaskBase()
{
    if (m_heartBreakTimer)
    {
        if (m_heartBreakTimer->isRunning())
        {
            m_heartBreakTimer->quit();
            m_heartBreakTimer->wait();
        }
    }
}
