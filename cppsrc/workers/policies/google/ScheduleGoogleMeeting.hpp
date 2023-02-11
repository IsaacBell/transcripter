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
    static const std::string RoutingKey = "ts-google-schedule-meeting";
    static const std::string Queue = "ts-google-queue";
    static const std::string Exchange = "ts-google-exchange";

    static void execute(
        const AMQP::Channel &channel,
        const AMQP::Message &message,
        uint64_t deliveryTag,
        bool redeliveredtask_body)
    {
      if (message.body().size() < 11)
      {
        channel->reject(deliveryTag, false);
        spdlog::info("GoogleSchedulingError::Message size too small, rejecting: {}", message.body());
        return;
      }

      Poco::URI uri("https://www.googleapis.com/calendar/v3/calendars/primary/events/quickAdd");

      uri.addQueryParameter("text", message.body().substr(10, message.body().size() - 1));
      uri.addQueryParameter("sendUpdates", "all");

      Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery(), Poco::Net::HTTPMessage::HTTP_1_1);

      request.setContentType("application/x-www-form-urlencoded");

      Poco::Net::HTTPClientSession session;
      session.sendRequest(request);

      Poco::Net::HTTPResponse response;
      std::istream &rs = session.receiveResponse(response);

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
