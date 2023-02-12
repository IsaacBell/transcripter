#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../Transcripter/Workers/Client.hpp"
#include "../Transcripter/Workers/MessageBus.hpp"

class TestClient : public testing::Test
{
protected:
  void SetUp() override
  {
    client = std::make_unique<Workers::Client<Workers::WorkPolicy>>();
  }

  std::unique_ptr<Workers::Client<Workers::WorkPolicy>> client;
};

// Testing the Client connect function
TEST_F(TestClient, ConnectTest)
{
  EXPECT_NO_THROW(client->connect());
}

// Testing the Client publish function
TEST_F(TestClient, PublishTest)
{
  const std::string msg = "Testing the Client publish function";
  Workers::MessageBus::getInstance().publish("exchange", "routing_key", msg);
  EXPECT_NO_THROW(client->publish(msg));
}

// Testing the Client run function
TEST_F(TestClient, RunTest)
{
  EXPECT_NO_THROW(client->run());
}
