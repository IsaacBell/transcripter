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
    // using ClientPool = std::vector<std::unique_ptr<Workers::Client>>;

  public:
    IEngine()
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
      Workers::Client<Test>::getInstance().run();
      Workers::Client<SendEmail>::getInstance().run();
      Workers::Client<TranscribeVideo>::getInstance().run();
      Workers::Client<CreateJiraTicket>::getInstance().run();
      Workers::Client<ScheduleGoogleMeeting>::getInstance().run();
    }
  };
}
