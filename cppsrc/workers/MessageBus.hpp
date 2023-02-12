#include <iostream>
#include <thread>
#include <chrono>
#include <memory>

#include "PocoHandler.hpp"
#include "policies/SendEmail.hpp"
#include "policies/Test.hpp"
#include "policies/TranscribeVideo.hpp"
#include "policies/jira/CreateTicket.hpp"
#include "policies/google/ScheduleGoogleMeeting.hpp"

namespace Workers
{
  class MessageBus
  {
  public:
    std::unique_ptr<AMQP::Channel> channel_; // store the channel in a unique pointer

    static MessageBus &getInstance()
    {
      static MessageBus instance;
      return instance;
    }

    bool isRunning() { return isRunning_; }

    void publish(const std::string &exchange, const std::string &routingKey, const std::string &message)
    {
      AMQP::Envelope env{message};
      env.setExchange(exchange);
      env.setRoutingKey(routingKey);
      channel_->publish(env);
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
        channel_ = std::make_unique<AMQP::Channel>(connection);

        channel->onReady([&]()
                         { spdlog::info("Client is connected to the bus!"); });

        // Declare Client connections
        channel->declareExchange("Clients", AMQP::fanout);
        channel->declareQueue("Client1");
        channel->declareQueue("Client2");
        channel->bindQueue("Clients", "Client1", "");
        channel->bindQueue("Clients", "Client2", "");
        channel->consume("Client1", AMQP::noack)->onReceived([&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                                                             { processMessage(channel, message, deliveryTag, redelivered); });
        channel->consume("Client2", AMQP::noack)->onReceived([&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                                                             { processMessage(channel, message, deliveryTag, redelivered); });

        // Exchanges
        channel->declareExchange(SendEmail::Exchange, AMQP::direct);
        channel->declareExchange(CreateJiraTicket::Exchange, AMQP::direct);
        channel->declareExchange(TranscribeVideo::Exchange, AMQP::direct);
        channel->declareExchange(ScheduleGoogleMeeting::Exchange, AMQP::direct);
        channel->declareExchange(Test::Exchange, AMQP::direct);

        // Queues
        channel->declareQueue(TranscribeVideo::Queue);
        channel->declareQueue(SendEmail::Queue);
        channel->declareQueue(CreateJiraTicket::Queue);
        channel->declareQueue(ScheduleGoogleMeeting::Queue);
        channel->declareQueue(Test::Queue);

        channel
            ->bindQueue(ScheduleGoogleMeeting::Exchange, ScheduleGoogleMeeting::Queue, ScheduleGoogleMeeting::RoutingKey)
            ->consume(ScheduleGoogleMeeting::RoutingKey, AMQP::noack)
            ->onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { ScheduleGoogleMeeting::execute(channel, message, deliveryTag, redelivered); });

        channel
            ->bindQueue(SendEmail::Exchange, SendEmail::Queue, SendEmail::RoutingKey)
            ->consume(SendEmail::RoutingKey, AMQP::noack)
            ->onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { SendEmail::execute(channel, message, deliveryTag, redelivered); });

        channel
            ->bindQueue(CreateJiraTicket::Exchange, CreateJiraTicket::Queue, CreateJiraTicket::RoutingKey)
            ->consume(CreateJiraTicket::RoutingKey, AMQP::noack)
            ->onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { CreateJiraTicket::execute(channel, message, deliveryTag, redelivered); });

        channel
            ->bindQueue(TranscribeVideo::Exchange, TranscribeVideo::Queue, TranscribeVideo::RoutingKey)
            ->consume(TranscribeVideo::RoutingKey, AMQP::noack)
            ->onReceived(
                [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
                { TranscribeVideo::execute(channel, message, deliveryTag, redelivered); });

        channel
            ->bindQueue(Test::Exchange, Test::Queue, Test::RoutingKey)
            ->consume(Test::RoutingKey, AMQP::noack)
            ->onReceived(
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
  };
}
