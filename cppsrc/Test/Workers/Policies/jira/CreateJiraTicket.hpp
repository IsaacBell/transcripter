#include <gtest/gtest.h>

#include <amqpcpp.h>

#include "../Transcripter/Workers/Client.hpp"
#include "../Transcripter/Workers/jira/CreateJiraTicket.hpp"

namespace Workers
{
  class CreateJiraTicketTest : public ::testing::Test
  {
  protected:
    const std::string RoutingKey = "ts-create-jira-ticket";
    const std::string Queue = "ts-jira-queue";
    const std::string Exchange = "ts-jira-exchange";

    class MockChannel : public AMQP::Channel
    {
    public:
      bool ready()
      {
        return true;
      }

      void ack(uint64_t deliveryTag) {}

      void nack() {}

      void reject(uint64_t deliveryTag, bool reQueue) {}

      void setQos(int prefetchCount) {}
    };

    class MockMessage : public AMQP::Message
    {
    public:
      std::string body()
      {
        Json::Value root;
        root["projectKey"] = "TST";
        root["summary"] = "This is a test ticket";
        root["description"] = "This is a test description";

        Json::FastWriter writer;
        std::string msg = writer.write(root);
        return msg;
      }
    };

    MockChannel mockChannel;
    MockMessage mockMessage;
  };

  TEST_F(CreateJiraTicketTest, ShouldCallAckWhenValidResponse)
  {
    bool redelivered = false;
    uint64_t deliveryTag = 0;
    Workers::CreateJiraTicket::execute(mockChannel, mockMessage, deliveryTag, redelivered);

    ASSERT_TRUE(mockChannel.ready());
  }

  TEST_F(CreateJiraTicketTest, ShouldCallNackWhenInvalidResponse)
  {
    // Arrange
    bool redelivered = false;
    uint64_t deliveryTag = 0;
    // Set the status to an invalid one
    Poco::Net::HTTPResponse response;
    response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);

    Workers::CreateJiraTicket::execute(mockChannel, mockMessage, deliveryTag, redelivered);

    ASSERT_FALSE(mockChannel.ready());
  }
} // namespace Workers
