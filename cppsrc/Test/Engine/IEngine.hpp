#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../Transcripter/Workers/Client.hpp"
#include "../Transcripter/Workers/MessageBus.hpp"
#include "../Transcripter/Workers/policies/Test.hpp"
#include "../Transcripter/Workers/policies/SendEmail.hpp"
#include "../Transcripter/Workers/policies/TranscribeVideo.hpp"
#include "../Transcripter/Workers/policies/jira/CreateJiraTicket.hpp"
#include "../Transcripter/Workers/policies/google/ScheduleGoogleMeeting.hpp"

namespace Transcripter
{
  class IEngine;
}

using ::testing::_;
using ::testing::Invoke;

class MockMessageBus : public Workers::MessageBus
{
public:
  MOCK_METHOD3(publish, void(const std::string &exchange, const std::string &routingKey, const std::string &message));
};

class EngineTest : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    engine = new Transcripter::IEngine();
    mockMessageBus = std::make_shared<MockMessageBus>();
    Workers::MessageBus::instance_ = mockMessageBus;
  }

  virtual void TearDown()
  {
    delete engine;
  }

  Transcripter::IEngine *engine;
  std::shared_ptr<MockMessageBus> mockMessageBus;
};

TEST_F(EngineTest, RunTestWorker)
{
  EXPECT_CALL(*mockMessageBus, publish(_, _, _))
      .WillOnce(Invoke([](const std::string &exchange, const std::string &routingKey, const std::string &message)
                       {
                         EXPECT_EQ(exchange, Test::Exchange);
                         EXPECT_EQ(routingKey, Test::RoutingKey);
                         EXPECT_EQ(message, "test msg"); }));

  engine->runTestWorker();
}

TEST(EngineTest, ConfigureClients)
{
  using ClientPool = std::vector<std::unique_ptr<Workers::Client>>;

  // Create a mock MessageBus object
  MockMessageBus mockMessageBus;

  // Assert that the ClientPool contains all the expected Clients
  ASSERT_EQ(engine->clients_.size(), 5u);
}
