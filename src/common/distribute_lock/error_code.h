#ifndef  DISTRIBUTE_LOCK__ERROR_CODE_H__
#define  DISTRIBUTE_LOCK__ERROR_CODE_H__

namespace distribute_lock
{

const char* StateString(int state);
const char* TypeString(int type);
const char* ErrorString(int ret);

} /// namespace


#endif  // DISTRIBUTE_LOCK__ERROR_CODE_H__
