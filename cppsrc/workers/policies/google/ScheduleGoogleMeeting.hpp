#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <amqpcpp.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>

namespace Workers
{
  class ScheduleGoogleMeeting : public WorkPolicy<ScheduleGoogleMeeting>
  {
  public:
    static const std::string routingKey = "ts-google-schedule-meeting";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      // Code goes here...
      Poco::URI uri("https://www.googleapis.com/calendar/v3/calendars/primary/events/quickAdd");

      // Set the query parameters
      uri.addQueryParameter("text", message.body());
      uri.addQueryParameter("sendUpdates", "all");

      // Create HTTP Request
      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);

      // Define the rest of the request
      request.setContentType("application/x-www-form-urlencoded");

      // Create a session and send the request
      Poco::Net::HTTPClientSession session;
      session.sendRequest(request);

      // Receive the response
      Poco::Net::HTTPResponse response;
      std::istream &rs = session.receiveResponse(response);

      // Handle response
      if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK && channel.ready())
      {
        channel.ack(deliveryTag); // acknowledge the message as processed
      }
      else if (!channel.ready())
        spdlog::error("AMQP::Can't publish, channel unavailable:{}:{}",
                      channel.getPeerAddress(), channel.getPeerPort());
      else
      {
        channel.nack(); // re-queue for later
        spdlog::error("GoogleAPIScheduleMeetingError::Status: {}, Body: {}", response.getStatus(), response.getBody());
      }
    };
  };
}; // namespace Workers
