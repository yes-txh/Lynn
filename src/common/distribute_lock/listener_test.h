#ifndef  LISTENER_H_
#define  LISTENER_H_

#include <string>
#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <common/distribute_lock/event_listener.h>

///@brief   ���������¼�: ������ӡ�����ʧЧ�����Ը��������¼�
class Listener : public distribute_lock::EventListener
{
public:
	Listener()
    {
    }
	virtual ~Listener() {}
public:
	//////////////////////////////////////////////////////////////////////////
	///@brief  zk�������Ľ�������¼�(���Լ���Ŀ¼)
	///@param  Ŀ¼·��
	///@param  ���ӵĽ������
    virtual void ChildChange(const std::string & res, const std::string & child)
    {
        RAW_LOG(INFO, "new tablet server:%s join.", child.c_str());
    }

	//////////////////////////////////////////////////////////////////////////
	///@brief zk���������ļ����Է������ĵ��¼�
	///@param �ļ�����
	///@param ���ĵ�����
	virtual void AttrChange(const std::string & res, const std::string  & attr)
    {
        RAW_LOG(INFO, "file %s  changed attr: %s.", res.c_str(), attr.c_str());
    }

	//////////////////////////////////////////////////////////////////////////
	///@brief zk���������ļ�����ʧ�¼�
	///@param �ļ�����
	void LockReleased( const std::string  & res)
    {
        RAW_LOG(INFO, "file %s lost lock. ", res.c_str());
    }
};


#endif // LISTENER_H_
