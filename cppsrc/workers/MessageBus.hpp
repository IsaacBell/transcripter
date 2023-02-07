#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"

namespace MessageBus
{
  class MessageBus
  {
  public:
    static MessageBus &getInstance()
    {
      static MessageBus instance;
      return instance;
    }

    void run()
    {
      if (!isRunning_)
      {
        PocoHandler handler("127.0.0.1", 5672);

        AMQP::Connection connection(
            &handler,
            AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
            "/");
        AMQP::Channel channel(&connection);

        channel.onReady([&]()
                        { std::cout << "MessageBus is ready!" << std::endl; });

        channel.declareExchange("ts-exchange", AMQP::direct);
        channel.declareQueue("ts-generic-response");
        channel.bindQueue("ts-exchange", "ts-generic-response", "generic-response");
        channel
            .consume("ts-generic-request", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { workPolicy.execute(channel, message, deliveryTag, redelivered); });

        handler.loop();
        isRunning_ = true;
      }
    }

    void setWorkPolicy(const WorkPolicy &policy)
    {
      workPolicy = policy;
    }

  private:
    // Singleton
    MessageBus() = default;
    MessageBus(const MessageBus &) = delete;
    MessageBus &operator=(const MessageBus &) = delete;

    bool isRunning_ = false;
    WorkPolicy workPolicy;
  };
}
