#ifndef MonitorAdapter_H
#define MonitorAdapter_H

#include <vector>

namespace GCmointor {

struct GCinfo
{
    bool isGCmonitor;
    int RCleft;
    int RCright;
    int RCtop;
    int RCbottom;
    char DeviceName[18];
};
static std::vector<GCinfo> StackGcinfo;
// 获取当前正在使用的Monitor
int XDD_GetActiveAttachedMonitor(std::vector<GCinfo> &out_vec);
} // namespace GCmointor
#endif
