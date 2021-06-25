#include "NetworkTaskManager.h"
#include "NetworkTaskBase.h"
#include <QDebug>
#include "NetworkDownloadTask.h"


const int kNormalCount = 25;
const int kLowCount = 2;



CNetworkTaskManager::CNetworkTaskManager(QObject* parent)
    : QObject(parent)
    , m_networkAccessManager(std::make_shared<QNetworkAccessManager>())
{ 
    //qRegisterMetaType(std::shared_ptr<INetworkTaskBase>);
    //qRegisterMetaType<std::shared_ptr<INetworkTaskBase> >();
    INetworkTaskBase::setNetworkManagerInstance(m_networkAccessManager);
    m_taskWorkThread = std::make_unique<std::thread>(&CNetworkTaskManager::TaskWorkHandle, this);
    qDebug() << m_networkAccessManager->supportedSchemes();
 
    connect(this, &CNetworkTaskManager::taskFinished, this, &CNetworkTaskManager::onTaskFinished);
    connect(this, &CNetworkTaskManager::startRequest, this, &CNetworkTaskManager::onStartRequest, Qt::QueuedConnection);
}
CNetworkTaskManager::~CNetworkTaskManager()
{
    //等待工作线程退出
    m_bTaskWorkThreadStop = true;
    m_taskWorkThread->join();
}

bool CNetworkTaskManager::checkTaskExist(const QString& taskID) const
{
    if (!taskID.isEmpty())
    {
        for (auto i = 0; i < m_listAllTask.size(); i++)
        {
            NetworkTaskWeakPtr weakTask(m_listAllTask[i]);
            if (weakTask.expired()) return false;

            if (weakTask.lock()->taskInfo().taskID == taskID)
            {
                return  weakTask.lock()->taskState() == NetworkTask::ETaskStatus::Processed ? false : true;
            }
        }
    }
    return false;
}

void CNetworkTaskManager::checkTaskPriority(const NetworkTask::stNetworkParamBase* param, const QString& taskID)
{
    if (param && !taskID.isEmpty())
    {
        std::weak_ptr<INetworkTaskBase> taskBase;
        for (int i = 0; i < m_listAllTask.size(); i++)
        {
            if (m_listAllTask[i]->taskInfo().taskID == taskID) taskBase = m_listAllTask[i];
        }

        //观察shared_ptr包装的task对象是否被析构
        if (false == taskBase.expired())
        {
            if (taskBase.lock()->taskState() == NetworkTask::Processed)
            {
                emit taskFinished(taskBase.lock()->taskInfo(), taskBase.lock()->taskResult());
            }
            else
            {
                if (param->taskPriority == NetworkTask::High && taskBase.lock()->priority() != NetworkTask::High)
                {
                    taskBase.lock()->startTask();
                }
            }
        }
    }
}

QString CNetworkTaskManager::addTaskToMap(std::shared_ptr<INetworkTaskBase> task)
{
    //生产消费数据
    if (task == nullptr)
    {
        return "";
    }
    QMutexLocker locker(&m_mutex);
    auto taskItor = qFind(m_listAllTask.begin(), m_listAllTask.end(), task);
    if (m_listAllTask.end() == taskItor)
    {
        connect(task.get(), &INetworkTaskBase::taskUploadProgress, this, &CNetworkTaskManager::taskUploadProgress, Qt::QueuedConnection);
        connect(task.get(), &INetworkTaskBase::taskDownloadProgress, this, &CNetworkTaskManager::taskDownloadProgress, Qt::QueuedConnection);
        connect(task.get(), &INetworkTaskBase::taskFinished, this, &CNetworkTaskManager::taskFinished, Qt::QueuedConnection);

        m_listAllTask.push_back(task);

        switch (task->priority())
        {
            case NetworkTask::High:   { m_listHighPriority.push_back(task); break; }
            case NetworkTask::Normal: { m_listNormalPriority.push_back(task);break; }
            case NetworkTask::Low:    { m_listLowPriority.push_back(task); break; }
            default: break;
        }
    }
    return task->taskInfo().taskID;
}


void CNetworkTaskManager::onTaskFinished(const NetworkTask::stNetworkTaskInfo& taskInfo, const NetworkTask::stNetworkResult& result)
{
    (void)(result);
    for (auto task : m_listAllTask)
    {
        NetworkTaskWeakPtr weak(task);
        if (weak.expired()) continue;
    
        if (weak.lock()->taskInfo().taskID == taskInfo.taskID) {
            auto taskItor = qFind(m_listAllTask.begin(), m_listAllTask.end(), task);
            m_listAllTask.erase(taskItor);
        }
    }
}

void CNetworkTaskManager::onStartRequest(std::shared_ptr<INetworkTaskBase> task)
{
    if (task) {
        task->startTask();
    }
}

int CNetworkTaskManager::processingTask(NetworkTask::ETaskPriority priority) const
{
    int i = 0;
    for (NetworkTaskWeakPtr task : m_listAllTask)
    {
        if (task.expired()) continue;
      
        if (task.lock()->priority() == priority && task.lock()->taskState() == NetworkTask::Processing)
        {
            i++;
        }
    }
    return i;
}

void CNetworkTaskManager::TaskWorkHandle()
{
    //消费队列
    //notes:QT 网络请求 QNetworkAccessManager的使用必须保证在同一线程，否则无法收到reply 
    while ( !m_bTaskWorkThreadStop )
    {
        QMutexLocker locker(&m_mutex);
       
        while (!m_listHighPriority.isEmpty())
        {
            emit startRequest(m_listHighPriority.front());
            m_listHighPriority.pop_front();
        }

        //m_listHighPriority.clear();

        while (!m_listNormalPriority.isEmpty() && processingTask(NetworkTask::Normal) < kNormalCount)
        {
            NetworkTaskWeakPtr task = m_listNormalPriority[0];
            if (false == task.expired() && task.lock()->taskState() == NetworkTask::Ready)
            {
                emit startRequest(m_listNormalPriority[0]);
            }
            m_listNormalPriority.pop_front();
        }

        while (!m_listLowPriority.isEmpty() && processingTask(NetworkTask::Low) < kLowCount)
        {
            NetworkTaskWeakPtr task = m_listLowPriority[0];
            if (false == task.expired() && task.lock()->taskState() == NetworkTask::Ready)
            {
                emit startRequest(m_listLowPriority[0]);
            }
            m_listLowPriority.pop_front();
        }
    }
    
}

QString CNetworkTaskManager::addTask(const NetworkTask::stDownloadParam& param, const QString& taskID /*= ""*/)
{
    if (checkTaskExist(taskID))
    {
        checkTaskPriority(&param, taskID);
        return taskID;
    }

    return addTaskToMap(std::make_shared<CNetworkDownloadTask>(param, taskID));
}

bool CNetworkTaskManager::stopTaskByTaskId(const QString &taskID)
{
    QMutexLocker locker(&m_mutex);
    for (auto task : m_listAllTask)
    {
        NetworkTaskWeakPtr weak(task);
        if(weak.expired())
        {
            continue;
        }
        if(weak.lock()->taskInfo().taskID == taskID && weak.lock()->taskState() == NetworkTask::Processing)
        {
            weak.lock()->endTask();
            NetworkTaskIter taskItor = qFind(m_listAllTask.begin(), m_listAllTask.end(), task);
            m_listAllTask.erase(taskItor);
            break;
        }
        else
        {
            continue;
        }
    }
    return  true;
}

bool CNetworkTaskManager::stopAllTask()
{
    QMutexLocker locker(&m_mutex);
    for (NetworkTaskWeakPtr task : m_listAllTask)
    {
        if (task.expired())
        {
            continue;
        }
        if (task.lock()->taskState() == NetworkTask::Processing)
        {
            task.lock()->endTask();
        }
    }
    m_listAllTask.clear();
    return true;
}