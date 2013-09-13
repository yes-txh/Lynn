#include <iostream>
#include <string>
#include <string.h>
#include "common/clynn/rpc.h"
#include "include/proxy.h"

using namespace std;

static string usage = "./kill_task task_id";

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout << "Usage is wrong." << endl;
        cout << "Usage is: " << usage << endl;
        return -1;
    }
    uint64_t task_id = atoi(argv[1]);
    if (task_id <= 0)
    {   
        cout << "Usage is wrong." << endl;
        cout << "Usage is: " << usage << endl;
        return -1;
    }

    cout<<"kill task 1"<<endl;
    string endpoint = "127.0.0.1:9999";

    try
    {
        Proxy<SlaveClient> proxy = Rpc<SlaveClient, SlaveClient>::GetProxy(endpoint);
        proxy().Hello("abc");
        proxy().KillTask(task_id);
    }
    catch (TException &tx)
    {
        cout<<"error"<<endl;
        return -1;
    }
    return 0;
}
