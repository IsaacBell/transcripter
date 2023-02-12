#include <gtest/gtest.h>
#include <vector>
#include <thread>

#include "../Transcripter/Workers/Message.hpp"
#include "../Transcripter/Engine.hpp"

TEST(Transcripter_Engine, start)
{
  Engine engine;
  engine.start();

  EXPECT_TRUE(Workers::MessageBus::isInstantiated());
  EXPECT_TRUE(Workers::MessageBus::getInstance().isRunning());
}
