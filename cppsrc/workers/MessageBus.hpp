#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "policies/SendEmail.hpp"
#include "policies/Test.hpp"
#include "policies/jira/CreateTicket.hpp"

namespace Workers
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
                        { std::cout << "MessageBus is ready!"
                                    << "\n"; });

        // Exchanges
        channel.declareExchange("ts-default", AMQP::direct);

        // Queues
        channel.declareQueue("ts-email-queue");
        channel.declareQueue("ts-jira-queue");
        channel.declareQueue("ts-test-queue");

        // bindQueue args: exchange, queue, routing key
        channel.bindQueue("ts-email-exchange", "ts-email-queue", "ts-send-email");
        channel
            .consume("ts-send-email", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { SendEmail::execute(channel, message, deliveryTag, redelivered); });

        channel.bindQueue("ts-jira-exchange", "ts-jira-queue", "ts-create-jira-ticket");
        channel
            .consume("ts-create-jira-ticket", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { CreateJiraTicket::execute(channel, message, deliveryTag, redelivered); });

        channel.bindQueue("ts-default", "ts-test-queue", "ts-test*");
        channel
            .consume("ts-test", AMQP::noack)
            .onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { Test::execute(channel, message, deliveryTag, redelivered); });

        handler.loop();
        isRunning_ = true;
      }
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
