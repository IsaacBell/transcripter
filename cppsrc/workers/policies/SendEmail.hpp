#include <amqpcpp.h>

#include "Client.hpp"
#include "../Service/Mail.hpp"

namespace Workers
{
  class SendEmail : public WorkPolicy<SendEmail>
  {
  public:
    static const std::string routingKey = "ts-send-email";

    static void execute(
        AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redelivered)
    {
      bool sent = Mail::send(message.body());

      if (sent && channel.ready())
      {
        channel.ack(deliveryTag); // acknowledge the message as processed
      }
      else
      {
        channel.nack(); // re-queue for later
        spdlog::error("AMQP::Can't publish, channel unavailable:{}:{}",
                      channel.getPeerAddress(), channel.getPeerPort());
      }
    };
  };

}; // namespace Workers
