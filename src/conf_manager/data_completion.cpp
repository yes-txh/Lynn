#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "conf_manager/data_completion.h"

std::string GetNodeFromPath(const std::string &path) {
    std::string node;
    std::vector<std::string> items;

    //SplitString(path, "/", &items);
    boost::split(items, path, boost::is_any_of("/"));

    if (items.size() != 0) {
        //node = StringTrim(*(--items.end()));
        node = *(--items.end());
        boost::trim(node);
    }
    return node;
}

// DataCompletion for aget
void DataCompletion(int rc, const char *value, int value_len,
                    const Stat* stat, const void *data) {
    printf("[%s]: rc = %d\n", reinterpret_cast<const char*>
            (data == 0 ? "null" : data), rc);
    std::string path = reinterpret_cast<const char*>(data == 0 ? "null" : data);

    if (value) {
        printf("value_len = %d\n", value_len);
    }
}

