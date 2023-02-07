#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "MessageBus.hpp"

/*
  Workers::Client<SendEmail> clientA;
  clientA.run();

  Workers::Client<CreateJiraTicket> clientB;
  clientB.run();
*/

namespace Workers
{
  template <typename WorkPolicy>
  class ClientBase
  {
  public:
    void execute(AMQP::Channel& channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
      static_cast<WorkPolicy*>(this)->execute(channel, message, deliveryTag, redelivered);
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
        auto bus = MessageBus::getInstance();
        bus->connect();

        AMQP::Connection connection(
          &handler,
          AMQP::Login(std::getenv("AMQP_USERNAME"), std::getenv("AMQP_PASSWORD")),
          "/"
        );
        AMQP::Channel channel(&connection);

        channel.onReady([&]()
                        { std::cout << "Client is connected to the bus!\n"; });

        channel.declareExchange(bus->exchange(), AMQP::direct);
        channel.declareQueue(bus->queue());
        channel.bindQueue(bus->exchange(), bus->queue(), WorkPolicy::routingKey);
        channel
            .consume(bus->queue(), AMQP::noack)
            .onReceived([this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                        { WorkPolicy::execute(channel, message, deliveryTag, redelivered); });

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
    static WorkPolicy& getInstance()
    {
      static WorkPolicy instance;
      return instance;
    }

    void execute(AMQP::Channel& channel, const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
    {
      // Implementation specific to this work policy
      throw("Base WorkPolicy class shouldn't be called!")
    }
  };
}
