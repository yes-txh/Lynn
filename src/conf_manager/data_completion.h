#ifndef CONF_MANAGER_DATA_COMPLETION_H
#define CONF_MANAGER_DATA_COMPLETION_H

#include <string>
#include <zookeeper/zookeeper.h>

std::string GetNodeFromPath(const std::string &path);
/// declare DataCompletion
void DataCompletion(int rc, const char *value, int value_len,
                    const Stat* stat, const void *data);

#endif // CONF_MANAGER_DATA_COMPLETION_H
