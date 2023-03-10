# Worker System

A Message Bus built on [RabbitMQ](https://www.rabbitmq.com/).

Depends on [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP).

To start a Client:

```cpp
Workers::Client.run();
```

## Creating Work Policies

All work policies should have a routing key and an execution strategy:

```cpp
namespace Workers
{
  class Test : public WorkPolicy<Test>
  {
    static const std::string RoutingKey = "ts-test*";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body,
        Test &worker) {}
  }
}
```

Note that all routing keys begin with "ts-".

## Exchanges

- `"ts-email-exchange"` for sending emails
- `"ts-api-exchange"` for posting to an API
- `"ts-jira-exchange"` for creating Jira tickets
- `"ts-google-exchange"` for creating Jira tickets

## Queues

- "ts-email-queue"
- "ts-jira-queue"
- "ts-test-queue"
- "ts-google-queue"

## Routing Keys

- "ts-send-email"
- "ts-create-jira-ticket"
- "ts-google-schedule-meeting"
- "ts-test"

## Example worker inputs

```text
email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`

jira X-PROJECT QNET-7 X-SUMMARY Do some dry-wall work. X-DESCRIPTION There are BATS in the walls!

schedule `Q2 Planning Meeting`
```
