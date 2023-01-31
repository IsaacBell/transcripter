#include <amqpcpp.h>

#include "../../Mail.hpp"

namespace Workers
{
  class SendEmail
  {
    void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      bool sent = Mail.send(message.body());

      if (sent && channel.ready())
      {
        channel.publish("ts-exchange", "generic-response", response);
      }
      else
      {
        // TODO: re-queue the worker
        std::cout << "Can't publish, channel unavailable"
                  << "\n";
      }
    };
  }
}; // namespace Workers
