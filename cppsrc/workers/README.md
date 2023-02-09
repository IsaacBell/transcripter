# Worker System

This is a task queue system, built on [RabbitMQ](https://www.rabbitmq.com/).

The [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP) does the heavy lifting for us here.

To start a Client:

```
Workers::Client.run();
```

# Creating Work Policies

All work policies should have a routing key and an execution strategy:

```
namespace Workers
{
  class Test : public WorkPolicy<Test>
  {
    static const std::string routingKey = "ts-test*";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body,
        Test &worker) {}
  }
}

Note that all routing keys begin with "ts-".

## Exchanges

- `"ts-email-exchange"` for sending emails
- `"ts-api-exchange"` for posting to an API
- `"ts-jira-exchange"` for creating Jira tickets

## Queues

- "ts-email-queue"
- "ts-jira-queue"
- "ts-test-queue"


## Routing Keys

- "ts-send-email"
- "ts-create-jira-ticket"
- "ts-test"
