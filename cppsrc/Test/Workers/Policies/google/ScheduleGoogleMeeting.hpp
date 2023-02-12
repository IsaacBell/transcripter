#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <amqpcpp.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>
#include "ScheduleGoogleMeeting.hpp"
#include <gtest/gtest.h>

// mock the AMQP library
class MockAMQPChannel : public AMQP::Channel
{
public:
  MOCK_METHOD3(publish, void(const std::string &, const std::string &, const std::string &));
  MOCK_METHOD1(ack, void(uint64_t));
  MOCK_METHOD1(nack, void(uint64_t));
  MOCK_METHOD1(reject, void(uint64_t));
  MOCK_CONST_METHOD0(ready, bool());
  MOCK_CONST_METHOD0(getPeerAddress, std::string());
  MOCK_CONST_METHOD0(getPeerPort, uint16_t());
};

class ScheduleGoogleMeetingTest : public testing::Test
{
protected:
  virtual void SetUp()
  {
    // create mock objects
    channel = new MockAMQPChannel();

    // create message
    message = new AMQP::Message();
    message->setBody("{\"text\": \"schedule meeting\"}");
  }

  virtual void TearDown()
  {
    // delete mock objects
    delete channel;
    delete message;
  }

  MockAMQPChannel *channel;
  AMQP::Message *message;
};

TEST_F(ScheduleGoogleMeetingTest, TestExecuteSuccess)
{
  EXPECT_CALL(*channel, ready())
      .Times(1)
      .WillOnce(testing::Return(true));
  EXPECT_CALL(*channel, getPeerAddress())
      .Times(1)
      .WillOnce(testing::Return("127.0.0.1"));
  EXPECT_CALL(*channel, getPeerPort())
      .Times(1)
      .WillOnce(testing::Return(8080));
  EXPECT_CALL(*channel, publish(_, _, _))
      .Times(1);
  EXPECT_CALL(*channel, ack(_))
      .Times(1);

  Workers::ScheduleGoogleMeeting::execute(*channel, *message, 0, false);
}

TEST_F(ScheduleGoogleMeetingTest, TestExecuteFail)
{
  EXPECT_CALL(*channel, ready())
      .Times(1)
      .WillOnce(testing::Return(false));
  EXPECT_CALL(*channel, getPeerAddress())
      .Times(1)
      .WillOnce(testing::Return("127.0.0.1"));
  EXPECT_CALL(*channel, getPeerPort())
      .Times(1)
      .WillOnce(testing::Return(8080));
  EXPECT_CALL(*channel, publish(_, _, _))
      .Times(0);
  EXPECT_CALL(*channel, nack())
      .Times(1);

  Workers::ScheduleGoogleMeeting::execute(*channel, *message, 0, false);
}
