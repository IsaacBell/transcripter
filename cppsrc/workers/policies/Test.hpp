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
        bool redeliveredtask_body,
        Test &worker)
    {
      std::cout << "[x] Received " << message.body() << "\n";

      std::string msg = message.body();
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
        channel.publish("ts-exchange", "generic-response", ":::test response:::");
      }
      else
      {
        // TODO: re-queue the worker
        std::cout << "Can't publish, channel unavailable"
                  << "\n";
      }
    };
  };
}; // namespace Workers
