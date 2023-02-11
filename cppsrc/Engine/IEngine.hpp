#include "Workers/Test.hpp"
#include "Workers/Client.hpp"
#include "Workers/MessageBus.hpp"

namespace Transcripter
{
  class IEngine
  {
    using ClientPool = std::vector<std::unique_ptr<Workers::Client>>;

  public:
    IEngine()
    {
      configureClients();
      // configureThreadPool();
    }

    void runTestWorker()
    {
      Workers::Client<Test>::getInstance().publish("test msg");
    }

  private:
    ClientPool clients_;
    std::vector<std::thread> threads_;

    void configureClients()
    {
      Workers::Client<Test>::getInstance().run();
      Workers::Client<SendEmail>::getInstance().run();
      Workers::Client<TranscribeVideo>::getInstance().run();
      Workers::Client<CreateJiraTicket>::getInstance().run();
      Workers::Client<ScheduleGoogleMeeting>::getInstance().run();
    }

    void configureThreadPool()
    {
      size_t numThreads = std::thread::hardware_concurrency();

      for (size_t i = 0; i < numThreads; ++i)
      {
        threads_.emplace_back([=]
                              {
      while (true) {
        // Thread operations go here.
      } });
      }
    }
  };
}
