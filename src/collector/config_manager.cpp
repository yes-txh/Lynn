#include "collector/config_manager.h"

bool CollectorConf::Init() {
    return m_dynamic_config.Init("collecotr");
}
