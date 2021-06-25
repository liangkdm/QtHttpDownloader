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
    //����״̬
    NetworkTask::ETaskStatus taskState() const
    {
        return m_taskState;
    }

    //������Ϣ
    const NetworkTask::stNetworkTaskInfo& taskInfo() const;

    //������
    const NetworkTask::stNetworkResult& taskResult() const;

    //��ʼ����
    virtual bool startTask();

    //��ֹ����
    virtual void endTask();

    //����Toolʵ��
    static void                                    setTool( std::shared_ptr< CNetworkTaskToolBase > toolBase );
    static std::shared_ptr< CNetworkTaskToolBase > tool();

    //����������ʶ���
    static void                   setNetworkManagerInstance( std::shared_ptr< QNetworkAccessManager > manager );
    static QNetworkAccessManager* networkManagerInstance();

    //�������ȼ�
    NetworkTask::ETaskPriority priority() const
    {
        return m_taskPriority;
    }
    void setPriority( NetworkTask::ETaskPriority priority )
    {
        m_taskPriority = priority;
    }

    //������get��������ʱ��ͨ���ú�����ȡ���ص�����
    virtual void downloadDataReady( const QVariant& data ) = 0;

protected:
    //����http���ش���
    bool resolveWhenHttpError( QNetworkReply::NetworkError httpCode );

    //������
    bool checkParam( const NetworkTask::stNetworkParamBase* param );

    //������
    NetworkTask::stNetworkResult& taskResult()
    {
        return m_result;
    }

    //��Ӧ����
    QNetworkReply* currentReply()
    {
        return m_reply;
    }
    void setCurrentReply( QNetworkReply* reply );

    //��������״̬
    void setTaskState( NetworkTask::ETaskStatus state );

    //�Ƿ��ѳ�ʱ
    bool isTimeOut() const
    {
        return m_isTimeOut;
    }
    void setTimeOut( bool timeout )
    {
        m_isTimeOut = timeout;
    }

protected Q_SLOTS:
    //��̬+�� ��̬�滻QT http�ź���Ӧ
    virtual void onReplyFinished();
    virtual void onUploadProgress( qint64 bytesSent, qint64 bytesTotal );
    virtual void onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal );
    virtual void onTimeOut();

    //��ʱ��ʱ��
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
    //˽���ź�
    void startTimerSignal();

private:
    NetworkTask::ETaskStatus        m_taskState;                  //����״̬
    NetworkTask::stNetworkTaskInfo  m_info;                       //������Ϣ
    NetworkTask::stNetworkResult    m_result;                     //������
    NetworkTask::ETaskPriority      m_taskPriority;               //�������ȼ�
    std::unique_ptr< CThreadTimer > m_heartBreakTimer = nullptr;  //����ʱ��ʱ��
    QNetworkReply*                  m_reply;
    bool                            m_isTimeOut;
};

//�ڲ���
#define AFTER_TASK_FINISHED()                            \
    {                                                    \
        setTaskState( NetworkTask::Processed );          \
        Q_EMIT taskFinished( taskInfo(), taskResult() ); \
    }
#define TASK_UPLOAD_PROGRESS( bytesSent, bytesTotal ) ( Q_EMIT taskUploadProgress( taskInfo(), bytesSent, bytesTotal ) )
#define TASK_DOWNLOAD_PROGRESS( bytesReceived, bytesTotal ) ( Q_EMIT taskDownloadProgress( taskInfo(), bytesReceived, bytesTotal ) )

#endif  // NetworkTaskBase_h__
