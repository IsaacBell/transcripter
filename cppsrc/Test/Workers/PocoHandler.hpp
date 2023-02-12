#include <gtest/gtest.h>
#include "PocoHandler.h"

TEST(PocoHandlerTest, Loop)
{
  PocoHandler handler("localhost", 8888);
  handler.loop();

  ASSERT_TRUE(handler.connected());
}

TEST(PocoHandlerTest, Quit)
{
  PocoHandler handler("localhost", 8888);
  handler.quit();

  ASSERT_FALSE(handler.connected());
}

TEST(PocoHandlerTest, OnData)
{
  PocoHandler handler("localhost", 8888);
  const char *data = "This is some test data";
  size_t size = strlen(data);

  handler.onData(nullptr, data, size);
  ASSERT_EQ(handler.outBuffer.available(), size);
}

TEST(PocoHandlerTest, OnConnected)
{
  PocoHandler handler("localhost", 8888);
  handler.onConnected(nullptr);
  ASSERT_TRUE(handler.connected());
}

TEST(PocoHandlerTest, OnError)
{
  PocoHandler handler("localhost", 8888);
  const char *message = "This is an error";
  handler.onError(nullptr, message);
  ASSERT_STREQ(spdlog::get("error")->last_msg().c_str(), "AMQPConnectionError: This is an error");
}

TEST(PocoHandlerTest, OnClosed)
{
  PocoHandler handler("localhost", 8888);
  handler.onClosed(nullptr);
  ASSERT_TRUE(handler.quit());
}
