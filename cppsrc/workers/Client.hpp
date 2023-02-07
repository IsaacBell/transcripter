#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"

namespace Workers
{
  // template <class WorkPolicy>
  class Client
  {
  public:
    static Client& getInstance()
    {
      static Client instance;
      return instance;
    }

    void run()
    {
      if (!isRunning_)
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
            .consume("ts-email-send", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { WorkPolicy.execute(channel, message, deliveryTag, redelivered); });

        channel
            .consume("ts-generic-request", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { WorkPolicy.execute(channel, message, deliveryTag, redelivered); });

        handler.loop();
        isRunning_ = true;
      }
    }

  private:
    // Singleton
    Client() = default;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool isRunning_ = false;
    Poco::Net::HTTPCredentials auth_;
  };
}
