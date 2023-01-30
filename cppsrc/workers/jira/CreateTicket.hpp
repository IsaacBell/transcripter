#include <amqpcpp.h>

#include "../base.hpp"

namespace Workers
{
  class CreateJiraTicket
  {
    void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      //
    }
  }
} // namespace Workers
