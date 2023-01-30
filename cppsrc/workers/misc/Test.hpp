#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <amqpcpp.h>

#include "../Base.hpp"

namespace Workers
{
  class Test
  {
    void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
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

      int id = std::stoi(tokens[0]);
      int n = std::stoi(tokens[1]);

      std::cout << "Request id is " << id << " and n is " << n << "\n";

      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      n = n * 2;

      std::string response = std::to_string(id) + " " + std::to_string(n) + " | ";
      std::cout << "Response is " << response << "\n";

      if (channel.ready())
      {
        channel.publish("ts-exchange", "generic-response", response);
      }
      else
      {
        // TODO: re-queue the worker
        std::cout << "Can't publish, channel unavailable"
                  << "\n";
      }
    };
  }
}; // namespace Workers
