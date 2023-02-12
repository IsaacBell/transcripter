#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/MailRecipient.h>

#include "../Transcriber/Mail.hpp"

class MockSMTPClientSession : public Poco::Net::SMTPClientSession
{
public:
  MOCK_METHOD2(login, void(const std::string &user, const std::string &password));
  MOCK_METHOD1(sendMessage, void(Poco::Net::MailMessage &message));
  MOCK_METHOD0(close, void());
};

class SendEmailTest : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    mockSMTPClientSession = std::make_shared<MockSMTPClientSession>();
    Poco::Net::SMTPClientSession::instance_ = mockSMTPClientSession;
  }

  virtual void TearDown()
  {
    Poco::Net::SMTPClientSession::instance_ = nullptr;
  }

  std::shared_ptr<MockSMTPClientSession> mockSMTPClientSession;
};

TEST_F(SendEmailTest, SendSuccess)
{
  EXPECT_CALL(*mockSMTPClientSession, login(_, _)).Times(1);
  EXPECT_CALL(*mockSMTPClientSession, sendMessage(_)).Times(1);
  EXPECT_CALL(*mockSMTPClientSession, close()).Times(1);

  const std::string input = R"(email foo@example.com subject: Test Email `This is a test email.` to: bar@example.com, baz@example.com)";
  const bool result = Mail::send(input);

  ASSERT_TRUE(result);
}

TEST_F(SendEmailTest, SendFailure)
{
  EXPECT_CALL(*mockSMTPClientSession, login(_, _)).Times(0);
  EXPECT_CALL(*mockSMTPClientSession, sendMessage(_)).Times(0);
  EXPECT_CALL(*mockSMTPClientSession, close()).Times(0);

  const std::string input = R"(foo bar)";
  const bool result = Mail::send(input);

  ASSERT_FALSE(result);
}
