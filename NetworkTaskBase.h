#ifndef NetworkTaskBase_h__
#define NetworkTaskBase_h__

#include "NetworkTaskStructs.h"
#include "NetworkTaskToolBase.h"
#include "ThreadTimer.h"
#include <QNetworkReply>
#include <memory>

class CNetworkTaskToolBase;
class QNetworkAccessManager;
class INetworkTaskBase : public QObject
{
    Q_OBJECT
    friend class CNetworkTaskManager;

public:
    INetworkTaskBase( int type, const NetworkTask::stNetworkParamBase* param, QObject* parent = 0 );
    INetworkTaskBase( const QString& taskID, int type, const NetworkTask::stNetworkParamBase* param, QObject* parent = 0 );
    virtual ~INetworkTaskBase();

public:
    //任务状态
    NetworkTask::ETaskStatus taskState() const
    {
        return m_taskState;
    }

    //任务信息
    const NetworkTask::stNetworkTaskInfo& taskInfo() const;

    //任务结果
    const NetworkTask::stNetworkResult& taskResult() const;

    //开始任务
    virtual bool startTask();

    //终止任务
    virtual void endTask();

    //设置Tool实例
    static void                                    setTool( std::shared_ptr< CNetworkTaskToolBase > toolBase );
    static std::shared_ptr< CNetworkTaskToolBase > tool();

    //设置网络访问对象
    static void                   setNetworkManagerInstance( std::shared_ptr< QNetworkAccessManager > manager );
    static QNetworkAccessManager* networkManagerInstance();

    //任务优先级
    NetworkTask::ETaskPriority priority() const
    {
        return m_taskPriority;
    }
    void setPriority( NetworkTask::ETaskPriority priority )
    {
        m_taskPriority = priority;
    }

    //当发起get下载请求时，通过该函数获取下载的数据
    virtual void downloadDataReady( const QVariant& data ) = 0;

protected:
    //解析http返回错误
    bool resolveWhenHttpError( QNetworkReply::NetworkError httpCode );

    //检测参数
    bool checkParam( const NetworkTask::stNetworkParamBase* param );

    //网络结果
    NetworkTask::stNetworkResult& taskResult()
    {
        return m_result;
    }

    //响应对象
    QNetworkReply* currentReply()
    {
        return m_reply;
    }
    void setCurrentReply( QNetworkReply* reply );

    //设置任务状态
    void setTaskState( NetworkTask::ETaskStatus state );

    //是否已超时
    bool isTimeOut() const
    {
        return m_isTimeOut;
    }
    void setTimeOut( bool timeout )
    {
        m_isTimeOut = timeout;
    }

protected Q_SLOTS:
    //多态+槽 动态替换QT http信号响应
    virtual void onReplyFinished();
    virtual void onUploadProgress( qint64 bytesSent, qint64 bytesTotal );
    virtual void onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
    virtual void onTimeOut();

    //超时定时器
    void startTimeoutTimer();
    void resetTimeoutTimer();
    void stopTimeoutTimer();
    void OnReadyRead();

Q_SIGNALS:
    void taskUploadProgress( const NetworkTask::stNetworkTaskInfo& taskInfo, qint64 bytesSent, qint64 bytesTotal );
    void taskDownloadProgress( const NetworkTask::stNetworkTaskInfo& taskInfo, qint64 bytesReceived, qint64 bytesTotal );
    void taskFinished( const NetworkTask::stNetworkTaskInfo& taskInfo, const NetworkTask::stNetworkResult& result );

private:
Q_SIGNALS:
    //私有信号
    void startTimerSignal();

private:
    NetworkTask::ETaskStatus        m_taskState;                  //任务状态
    NetworkTask::stNetworkTaskInfo  m_info;                       //任务信息
    NetworkTask::stNetworkResult    m_result;                     //任务结果
    NetworkTask::ETaskPriority      m_taskPriority;               //任务优先级
    std::unique_ptr< CThreadTimer > m_heartBreakTimer = nullptr;  //任务超时定时器
    QNetworkReply*                  m_reply;
    bool                            m_isTimeOut;
};

//内部宏
#define AFTER_TASK_FINISHED()                            \
    {                                                    \
        setTaskState( NetworkTask::Processed );          \
        Q_EMIT taskFinished( taskInfo(), taskResult() ); \
    }
#define TASK_UPLOAD_PROGRESS( bytesSent, bytesTotal ) ( Q_EMIT taskUploadProgress( taskInfo(), bytesSent, bytesTotal ) )
#define TASK_DOWNLOAD_PROGRESS( bytesReceived, bytesTotal ) ( Q_EMIT taskDownloadProgress( taskInfo(), bytesReceived, bytesTotal ) )

#endif  // NetworkTaskBase_h__
