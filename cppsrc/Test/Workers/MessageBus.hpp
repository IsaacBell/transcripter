#include <gtest/gtest.h>
#include "PocoHandler.hpp"
#include "policies/SendEmail.hpp"
#include "policies/Test.hpp"
#include "policies/TranscribeVideo.hpp"
#include "policies/jira/CreateTicket.hpp"
#include "policies/google/ScheduleGoogleMeeting.hpp"

namespace Workers
{
  TEST(MessageBus, Publish)
  {
    PocoHandler handler("127.0.0.1", 5672);
    AMQP::Connection connection(
        &handler,
        AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
        "/");
    MessageBus bus;
    bus.publish("Clients", "Client1", "Test message");
    EXPECT_TRUE(bus.isRunning());
  }

  TEST(MessageBus, Run)
  {
    PocoHandler handler("127.0.0.1", 5672);
    AMQP::Connection connection(
        &handler,
        AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
        "/");
    MessageBus bus;
    bus.run();
    EXPECT_TRUE(bus.isRunning());
  }

  TEST(MessageBus, ProcessMessage)
  {
    PocoHandler handler("127.0.0.1", 5672);
    AMQP::Connection connection(
        &handler,
        AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
        "/");
    MessageBus bus;
    bus.run();
    AMQP::Message message = AMQP::Message("Test message");
    bus.processMessage(bus.channel, message, 0, false);
    EXPECT_TRUE(bus.isRunning());
  }
}
