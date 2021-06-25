#ifndef NetworkTaskStructDefine_h__
#define NetworkTaskStructDefine_h__

#include <QString>
#include <QVariant>
#include "NetworkTaskMacros.h"

namespace NetworkTask
{
    //�����������
    struct stNetworkParamBase
    {
        stNetworkParamBase()
        {
            taskPriority = NetworkTask::Normal;
        }

        //��������
        virtual int  taskType() const { return -1; }

        //�Ƿ���Ч
        virtual bool isValid() const = 0;

        NetworkTask::ETaskPriority				taskPriority;  //�������ȼ�
    };

    //������Ϣ
    struct stNetworkTaskInfo
    {
        stNetworkTaskInfo();

        stNetworkTaskInfo(int type);

        stNetworkTaskInfo(const QString& taskID, int type);

        const int			taskType;			//��������
        const QString		taskID;				//����ID
    };

    //���緵�ؽ��
    struct stNetworkResult
    {
        stNetworkResult()
        {
            errorCode = NetworkTask::NoError;
            success = false;
        }

        stNetworkResult(bool suc, EReturnCode code)
        {
            errorCode = code;
            success = suc;
        }

        bool success;						//������
        int	errorCode;						//ʧ�ܴ������
        QString errorMsg;					//������Ϣ
    };

    //�����������
    struct stDownloadParam : public stNetworkParamBase
    {
        stDownloadParam()
        {
            taskPriority = NetworkTask::Normal;
            localSaveFullPath = "temp.temp";
        }
        //��������
        virtual int taskType()  const { return NetworkTask::TaskType_Donwload; }

        //�Ƿ���Ч
        virtual bool isValid() const { return (!downloadUrl.isEmpty()); }

        QString downloadUrl;				//���صĵ�ַ
        QString localSaveFullPath;          //�ļ�Ŀ¼��ַ(����Ӻ�׺��)
    };
}

Q_DECLARE_METATYPE(NetworkTask::stNetworkResult);
Q_DECLARE_METATYPE(NetworkTask::stNetworkTaskInfo);
#endif // NetworkTaskStructDefine_h__
