#include "Workers/Client.hpp"
#include "Workers/MessageBus.hpp"
#include "Workers/policies/Test.hpp"
#include "Workers/policies/SendEmail.hpp"
#include "Workers/policies/TranscribeVideo.hpp"
#include "Workers/policies/jira/CreateJiraTicket.hpp"
#include "Workers/policies/google/ScheduleGoogleMeeting.hpp"

namespace Transcripter
{
  class IEngine
  {
    using ClientPool = std::vector<std::unique_ptr<Workers::Client>>;

  public:
    ClientPool clients_;

    IEngine() : clients_({Workers::Client<Test>::getInstance(),
                          Workers::Client<SendEmail>::getInstance(),
                          Workers::Client<TranscribeVideo>::getInstance(),
                          Workers::Client<CreateJiraTicket>::getInstance(),
                          Workers::Client<ScheduleGoogleMeeting>::getInstance()})
    {
      configureClients();
    }

    void runTestWorker()
    {
      Workers::Client<Test>::getInstance().publish("test msg");
    }

  private:
    void configureClients()
    {
      // connect to the MessageBus, automatically consume messages from the queue
      for (const auto &client : clients_)
        client->run();
    }
  };
}
