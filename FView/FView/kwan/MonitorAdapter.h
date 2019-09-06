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

//将当前所有Monitor坐标push到out_vec内
//并将EDID中包含HAIER的显示器标记为isGCmonitor = true
int XDD_GetActiveAttachedMonitor(std::vector<GCinfo> &out_vec);
} // namespace GCmointor
#endif
