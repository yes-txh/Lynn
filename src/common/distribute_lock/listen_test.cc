#include "common/distribute_lock/distribute_lock.h"
// ¼àÌıÆ÷
class Listener : public distribute_lock::EventListener{
    public:
        virtual void LockAcquired(const std::string & node)
        {
            LOG(INFO) << "listen acquire lock, node = " << node;
        }

        virtual void LockReleased(const std::string& node)
        {
            LOG(INFO) << "listen lose lock, node = " << node;
        }

    private:
};

int main(int argc, char *argv[])
{
    std::string node = "/ts";
    std::string zk_ip = "xaec.zk.oa.com:2181/zk/xaec/xcube/dist_lock";
    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "./";
    FLAGS_per_log_size = 1000;
    FLAGS_max_log_size = 20000;
    FLAGS_stderrthreshold = 3;
    FLAGS_logbuflevel = -1;
    distribute_lock::DistributeLock dist_lock;
    Listener* listen = new Listener;
    int ret = dist_lock.Init(zk_ip, listen, 20000);
    if (ret == 0)
    {
        ret = dist_lock.Exists("/ts");
        if (ret != 0)
        {
            ret = dist_lock.CreateNode("/ts");
        }
        // ÉèÖÃ¼àÌı
        ret = dist_lock.SetWatcher("/ts", distribute_lock::EVENT_MASK_CHILD_CHANGED);
        if (ret == 0) LOG(INFO) << "set child success";
        else LOG(INFO) << "set child failed";
    }
    while (true)
    {
        sleep(2);
    }
    return 0;
}
