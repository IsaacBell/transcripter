#include <vector>
#include <thread>

#include "../Workers/Test.hpp"
#include "../Workers/Client.hpp"
#include "../Workers/MessageBus.hpp"

namespace Transcripter
{
  class Engine : public IEngine
  {
  public:
    void start()
    {
      Workers::MessageBus::getInstance().run();
    }
  }
};
