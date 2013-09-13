#include <iostream>
#include "common/builder/test/src/d/a.h"
#include "common/base/string/string_number.hpp"
#include "common/system/time/timestamp.hpp"

using namespace std;

int main()
{
    cout << "Hello!" << endl;
    ThisIsATestFunction();
    cout << IntegerToString(123456) << endl;
    cout << GetTimeStamp() << endl;
    return 0;
}

