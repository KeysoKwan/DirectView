//简单的自动互斥类
#include <mutex>
using namespace std;

class Rlock
{
  public:
    Rlock(mutex* mu);
    ~Rlock();

  private:
    Rlock(){};
    mutex* m_Mutex;
};
