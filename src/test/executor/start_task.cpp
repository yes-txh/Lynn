#include <iostream>
#include <string>
#include <string.h>

#include <classad/classad.h>
#include <classad/classad_distribution.h>

#include "common/clynn/rpc.h"
#include "include/proxy.h"
#include "include/classad_attr.h"

using namespace std;

static string usage = "./start_task task_id vnc_port";

int main(int argc, char ** argv)
{
    if(argc != 3)
    {
        cout << "Usage is wrong." << endl;
        cout << "Usage is: " << usage << endl;
        return -1;
    }
    int32_t task_id = atoi(argv[1]);
    if (task_id <= 0)
    {    
        cout << "Usage is wrong." << endl;
        cout << "Usage is: " << usage << endl;
        return -1;
    }

    int32_t vnc_port = atoi(argv[2]);
    if (vnc_port < 0)
    {
        cout << "Usage is wrong." << endl;
        cout << "Usage is: " << usage << endl;
        return -1;
    }

    cout << "start task " << task_id << endl;
    string endpoint = "127.0.0.1:9997";

    // build task    
    ClassAd ad;
    ad.InsertAttr(ATTR_ID, task_id);
    ad.InsertAttr(ATTR_JOB_ID, 1);
    ad.InsertAttr(ATTR_VMTYPE, 1);
    ad.InsertAttr(ATTR_IS_RUN, false);
    ad.InsertAttr(ATTR_MEMORY, 1024);
    ad.InsertAttr(ATTR_VCPU, 1);
    ad.InsertAttr(ATTR_IP, "192.168.0.2");
    ad.InsertAttr(ATTR_PORT, 9991);
    ad.InsertAttr(ATTR_OS, "ubuntu");
    ad.InsertAttr(ATTR_IMG, "ubuntu.qco");
    ad.InsertAttr(ATTR_ISO, ".");
    ad.InsertAttr(ATTR_SIZE, 1);
    ad.InsertAttr(ATTR_VNC_PORT, vnc_port);

    // classad -> string
    ClassAdUnParser unparser;
    string str_ad;
    // Serialization, convert ClassAd into string str_ad
    unparser.Unparse(str_ad, &ad);


    // build a task
    /*TaskInfo task_info;
    task_info.framework_name = "Hadoop1.0.4";
    task_info.id = task_id;
    task_info.cmd = "bash";
    task_info.arguments = "/home/wm/work/hadoop/hadoop-1.0.4/bin/hadoop --config /home/wm/work/hadoop/hadoop-1.0.4/conf tasktracker";
    task_info.need_cpu = 0.5;
    task_info.need_memory = 500;*/

    try {
        Proxy<ExecutorClient> proxy = Rpc<ExecutorClient, ExecutorClient>::GetProxy(endpoint);
        proxy().Helloworld();
        proxy().StartTask(str_ad);
    } catch (TException &tx) {
        cout<<"error"<<endl;
        return -1;
    }

    return 0;
}
