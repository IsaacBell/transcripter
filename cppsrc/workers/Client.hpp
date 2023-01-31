#include <iostream>
#include <thread>
#include <chrono>

#include "PocoHandler.hpp"

namespace Workers
{
  template <class WorkPolicy>
  class Client
  {
  public:
    static void run()
    {
      PocoHandler handler("127.0.0.1", 5672);

      // TODO - add env var for login credentials
      AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
      AMQP::Channel channel(&connection);

      channel.onReady([&]()
                      { std::cout << "Worker is ready!" << std::endl; });

      channel.declareExchange("ts-exchange", AMQP::direct);
      channel.declareQueue("ts-generic-response");
      channel.bindQueue("ts-exchange", "ts-generic-response", "generic-response");
      channel
          .consume("ts-generic-request", AMQP::noack)
          .onReceived(
              [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
              { WorkPolicy.execute(channel, message, deliveryTag, redelivered); });

      handler.loop();
    }

  private:
    Poco::Net::HTTPCredentials auth_;
  };
}
