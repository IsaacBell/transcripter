#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <amqpcpp.h>

namespace Workers
{
  class Test : public WorkPolicy<Test>
  {
    static const std::string Queue = "ts-test-queue";
    static const std::string RoutingKey = "ts-test*";
    static const std::string Exchange = "ts-test-exchange";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      const std::string msg = message.body();
      spdlog::debug("[x] Test Worker Received:{}", msg.body());

      if (channel.ready())
      {
        channel.ack(deliveryTag); // acknowledge the message as processed
      }
      else
      {
        channel.reject(deliveryTag, false); // notify but don't re-queue test workers if failed
        spdlog::error("AMQP::Can't publish, channel unavailable:{}:{}",
                      channel.getPeerAddress(), channel.getPeerPort());
      }
    };
  };
}; // namespace Workers
