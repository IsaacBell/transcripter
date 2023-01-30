namespace Workers
{
  template <typename WorkPolicy>
  class Base
  {
  public:
    void e() {
      SimplePocoHandler handler("127.0.0.1", 5672);
  AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
  AMQP::Channel channel(&connection);

  channel.onReady([&]()
                  { std::cout << "Worker is ready!" << std::endl; });

  channel.declareExchange("ts-exchange", AMQP::direct);
  channel.declareQueue("ts-generic-response");
  channel.bindQueue("ts-exchange", "ts-generic-response", "generic-response");
  channel
      .consume("ts-generic-request", AMQP::noack)
      .onReceived(
          [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
          {
        std::cout << "[x] Received " << message.body() << std::endl << std::endl;

        std::string msg = message.body();
        std::vector<std::string> tokens;
        std::stringstream ss(msg);
        std::string token;
        while (std::getline(ss, token, ' '))
        {
          tokens.push_back(token);
        }

        int id = std::stoi(tokens[0]);
        int n = std::stoi(tokens[1]);

        std::cout << "Request id is " << id << " and n is " << n << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        n = n * 2;

        std::string response = std::to_string(id) + " " + std::to_string(n) + " | ";
        std::cout << "Response is " << response << std::endl;

        if (channel.ready())
        {
            channel.publish("ts-exchange", "generic-response", response);
        } else {
            std::cout << "Can't publish, channel unavailable" << std::endl;
        } });

  handler.loop();
    }

  private:
    std::string baseUrl_;
    Poco::Net::HTTPCredentials auth_;
  };
}
