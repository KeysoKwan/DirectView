//�򵥵��Զ�������
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
