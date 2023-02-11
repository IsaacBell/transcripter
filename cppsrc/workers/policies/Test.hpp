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
    static const std::string routingKey = "ts-test*";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      const std::string msg = message.body();
      spdlog::debug("[x] Test Worker Received:{}", msg);

      std::vector<std::string> tokens;
      std::stringstream ss(msg);
      std::string token;
      while (std::getline(ss, token, ' '))
      {
        tokens.push_back(token);
      }

      // int id = std::stoi(tokens[0]);
      // int n = std::stoi(tokens[1]);

      // std::cout << "Request id is " << id << " and n is " << n << "\n";

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
