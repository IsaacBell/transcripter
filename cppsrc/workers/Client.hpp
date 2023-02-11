#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "MessageBus.hpp"

/*
  Workers::Client<SendEmail>::getInstance().run();
*/

namespace Workers
{
  template <typename WorkPolicy>
  class ClientBase
  {
  public:
    void execute(AMQP::Channel &channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
      static_cast<WorkPolicy *>(this)->execute(channel, message, deliveryTag, redelivered);
    }
  };

  template <typename WorkPolicy>
  class Client : public ClientBase<WorkPolicy>
  {
  public:
    void run()
    {
      if (!isRunning_)
      {
        PocoHandler handler("127.0.0.1", 5672);
        auto bus = MessageBus::getInstance();
        bus->connect();

        AMQP::Connection connection(
            &handler,
            AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
            "/");
        AMQP::Channel channel(&connection);

        channel.onReady([&]() {
          spdlog::info("Client is connected to the bus!"); }

        channel.declareExchange(WorkPolicy::Exchange(), AMQP::direct);
        channel.declareQueue(WorkPolicy::Queue());
        channel.bindQueue(WorkPolicy::Exchange(), WorkPolicy::Queue(), WorkPolicy::RoutingKey);
        channel
            .consume(WorkPolicy::Queue(), AMQP::noack)
            .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                        {
          WorkPolicy::execute(channel, message, deliveryTag, redelivered); });

        handler.loop();
        isRunning_ = true;
      }
    }

    void publish(const std::string &message)
    {
      Workers::MessageBus::getInstance().publish(Exchange, RoutingKey, message);
    }

  private:
    bool isRunning_ = false;
    Poco::Net::HTTPCredentials auth_;
  };

  class WorkPolicy : public Client<WorkPolicy>
  {
  public:
    static WorkPolicy &getInstance()
    {
      static WorkPolicy instance;
      return instance;
    }

    void execute(AMQP::Channel &channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
      static_assert(false, "Base WorkPolicy class shouldn't be called!")
    }
  };
}
