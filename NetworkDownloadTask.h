//下载网络任务
#ifndef NetworkDownloadTask_h__
#define NetworkDownloadTask_h__

#include "NetworkTaskBase.h"
#include "NetworkTaskStructs.h"
#include <memory>
#include <QFile>

class CNetworkDownloadTask : public INetworkTaskBase
{
    Q_OBJECT

public:
    CNetworkDownloadTask(const NetworkTask::stDownloadParam& param, QObject* parent = 0);
    CNetworkDownloadTask(const NetworkTask::stDownloadParam& param, const QString& taskID, QObject* parent = 0);
    ~CNetworkDownloadTask();
public:
    //开始任务
    virtual bool startTask();
    virtual void downloadDataReady(const QVariant& data);
private Q_SLOTS:
    void onReplyFinished();
private:
    const NetworkTask::stDownloadParam	m_param;
    std::unique_ptr<QFile> m_saveFile = nullptr;
};
#endif // NetworkDownloadTask_h__
