#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "MessageBus.hpp"

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
        auto bus = MessageBus::getInstance();
        bus->connect();

        // TODO - add env var for login credentials
        AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
        AMQP::Channel channel(&connection);

        channel.onReady([&]()
                        { std::cout << "Client is connected to the bus!" << std::endl; });

        channel.declareExchange(bus->exchange(), AMQP::direct);
        channel.declareQueue(bus->queue());
        channel.bindQueue(bus->exchange(), bus->queue(), "generic-response");
        channel
            .consume(bus->queue(), AMQP::noack)
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
