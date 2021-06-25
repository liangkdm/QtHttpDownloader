#ifndef NetworkTaskManager_h__
#define NetworkTaskManager_h__

#include <QMap>
#include <QNetworkAccessManager>
#include "NetworkTaskStructs.h"
#include <memory>
#include <thread>
#include <QMutex>
#include <QMutexLocker>


class INetworkTaskBase;
class CNetworkTaskManager : public QObject
{
    Q_OBJECT
public:
    CNetworkTaskManager(QObject* parent = 0);
    virtual ~CNetworkTaskManager();

    using MapPriorityToTaskList = QMap< NetworkTask::ETaskPriority, QList<INetworkTaskBase*> >;
    using NetworkTaskWeakPtr = std::weak_ptr<INetworkTaskBase>;
    using NetworkTaskIter = QList<std::shared_ptr<INetworkTaskBase> >::iterator;
public:
    //添加下载任务
    QString addTask(const NetworkTask::stDownloadParam& param, const QString& taskID = "");
    //停止指定任务
    bool stopTaskByTaskId(const QString& taskID);
    //停止所有任务
    bool stopAllTask();   
protected:
    bool checkTaskExist(const QString& taskID) const;
    void checkTaskPriority(const NetworkTask::stNetworkParamBase* param, const QString& taskID);

private:
    QString addTaskToMap(std::shared_ptr<INetworkTaskBase> task);
    int processingTask(NetworkTask::ETaskPriority priority) const;
    void TaskWorkHandle();
signals:
    void taskUploadProgress(const NetworkTask::stNetworkTaskInfo& taskInfo, qint64 bytesSent, qint64 bytesTotal);
    void taskDownloadProgress(const NetworkTask::stNetworkTaskInfo& taskInfo, qint64 bytesReceived, qint64 bytesTotal);
    void taskFinished(const NetworkTask::stNetworkTaskInfo& taskInfo, const NetworkTask::stNetworkResult& result);
private:
Q_SIGNALS:
    void startRequest(std::shared_ptr<INetworkTaskBase> task);
private slots:
    void onTaskFinished(const NetworkTask::stNetworkTaskInfo& taskInfo, const NetworkTask::stNetworkResult& result);
    void onStartRequest(std::shared_ptr<INetworkTaskBase> task);
private:
    QList<std::shared_ptr<INetworkTaskBase> >   m_listAllTask;
    QList<std::shared_ptr<INetworkTaskBase> >   m_listHighPriority;			//高优先级队列
    QList<std::shared_ptr<INetworkTaskBase> >   m_listNormalPriority;		//正常优先级队列
    QList<std::shared_ptr<INetworkTaskBase> >   m_listLowPriority;			//低优先级队列
    std::shared_ptr<QNetworkAccessManager>      m_networkAccessManager;

    std::unique_ptr<std::thread> m_taskWorkThread = nullptr;
    bool m_bTaskWorkThreadStop = false;
    QMutex m_mutex;
};
Q_DECLARE_METATYPE(std::shared_ptr<INetworkTaskBase>);
#endif // NetworkTaskManager_h__
