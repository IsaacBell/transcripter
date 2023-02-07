#include <amqpcpp.h>

#include "Client.hpp"

namespace Workers
{
  class CreateJiraTicket : WorkPolicy<CreateJiraTicket>
  {
  public:
    static const std::string routingKey = "ts-create-jira-ticket";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redelivered)
    {
      std::string msg = message.body();

      // parse JSON input
      Json::Value root;
      Json::Reader reader;
      bool parsingSuccessful = reader.parse(msg, root);
      if (!parsingSuccessful)
      {
        // Handle error
        return;
      }

      Json::Value payload;
      payload["fields"]["project"]["key"] = root["projectKey"].asString();
      payload["fields"]["summary"] = root["summary"].asString();
      payload["fields"]["description"] = root["description"].asString();
      payload["fields"]["issuetype"]["name"] = "Task";

      Json::FastWriter writer;
      std::string jsonPayload = writer.write(payload);

      Poco::Net::HTTPRequest request(
          Poco::Net::HTTPRequest::HTTP_POST,
          "https://jira.example.com/rest/api/2/issue",
          Poco::Net::HTTPMessage::HTTP_1_1);

      request.setContentType("application/json");

      request.setContentLength(jsonPayload.length());

      Poco::Net::HTTPClientSession session("jira.example.com");

      std::ostream &request_stream = session.sendRequest(request);
      request_stream << jsonPayload;

      Poco::Net::HTTPResponse response;
      std::istream &response_stream = session.receiveResponse(response);

      Poco::JSON::Parser parser;
      Poco::Dynamic::Var result = parser.parse(response_stream);

      Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
      std::string ticket_key = object->get("key").convert<std::string>();

      channel.ack(deliveryTag); // acknowledge the message as processed
    }
  };
} // namespace Workers
