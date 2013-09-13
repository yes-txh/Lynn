#ifndef  LISTENER_H_
#define  LISTENER_H_

#include <string>
#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <common/distribute_lock/event_listener.h>

///@brief   监听三类事件: 结点的添加、锁的失效、属性更改三种事件
class Listener : public distribute_lock::EventListener
{
public:
	Listener()
    {
    }
	virtual ~Listener() {}
public:
	//////////////////////////////////////////////////////////////////////////
	///@brief  zk发过来的结点增加事件(用以监听目录)
	///@param  目录路径
	///@param  增加的结点名称
    virtual void ChildChange(const std::string & res, const std::string & child)
    {
        RAW_LOG(INFO, "new tablet server:%s join.", child.c_str());
    }

	//////////////////////////////////////////////////////////////////////////
	///@brief zk发过来的文件属性发生更改的事件
	///@param 文件名称
	///@param 更改的属性
	virtual void AttrChange(const std::string & res, const std::string  & attr)
    {
        RAW_LOG(INFO, "file %s  changed attr: %s.", res.c_str(), attr.c_str());
    }

	//////////////////////////////////////////////////////////////////////////
	///@brief zk发过来的文件锁丢失事件
	///@param 文件名称
	void LockReleased( const std::string  & res)
    {
        RAW_LOG(INFO, "file %s lost lock. ", res.c_str());
    }
};


#endif // LISTENER_H_
