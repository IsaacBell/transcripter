#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "MessageBus.hpp"

/*
  Usage:

  Workers::Client<SendEmail>::getInstance().run();
*/

namespace Workers
{
  template <typename WorkPolicy>
    requires std::is_base_of_v<ClientBase<WorkPolicy>, WorkPolicy> &&
             std::is_invocable_v<decltype(WorkPolicy::execute), AMQP::Channel &, const AMQP::Message &, uint64_t, bool>
  class ClientBase
  {
  public:
    static void execute(AMQP::Channel &channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) const
    {
      static_cast<WorkPolicy *>(this)->execute(channel, message, deliveryTag, redelivered);
    }
  };

  template <typename WorkPolicy>
  class Client : public ClientBase<WorkPolicy>
  {
  public:
    void connect(const std::string client1 = "Client1", const std::string client2 = "Client2") const
    {
      channel.consume(client1, AMQP::noack).onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                                                       { onMessageReceived(message, deliveryTag, redelivered, client1); });
      channel.consume(client2, AMQP::noack).onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                                                       { onMessageReceived(message, deliveryTag, redelivered, client2); });
      bus->connect();
    }

    void onMessageReceived(const AMQP::Message &message,
                           uint64_t deliveryTag, bool redelivered, const std::string &client) const
    {
      WorkPolicy::execute(message, deliveryTag, redelivered, client);
    }

    void publish(const std::string &message) const
    {
      Workers::MessageBus::getInstance().publish(Exchange, RoutingKey, message);
    }

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

    static void execute(AMQP::Channel &channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
      static_assert(false, "Base WorkPolicy class shouldn't be called!")
    }
  };
}
