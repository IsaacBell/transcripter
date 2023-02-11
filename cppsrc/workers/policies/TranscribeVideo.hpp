#include <amqpcpp.h>

#include "Client.hpp"
#include "SendEmail.hpp"
#include "jira/CreateJiraTicket.hpp"
#include "google/ScheduleGoogleMeeting.hpp"
#include "../API/OpenAI.hpp"
#include "json/json.h"

namespace Workers
{
  class TranscribeVideo : public WorkPolicy<TranscribeVideo>
  {
  public:
    static const std::string Queue = "ts-transcription-queue";
    static const std::string RoutingKey = "ts-transcribe-video";
    static const std::string Exchange = "ts-transcription-exchange";

    static void execute(
        AMQP::Channel &channel,
        const AMQP::Message &message, // URL to transcribe
        uint64_t deliveryTag,
        bool redelivered)
    {
      if (auto results = transcribe(message) && channel.ready())
      {
        executeResults(results);
        channel.ack(deliveryTag); // acknowledge the message as processed
      }
      else if (!channel.ready())
      {
        channel.nack(); // re-queue for later
      }
      else
      {
        channel.reject(delivery, false); // drop from bus
        spdlog::error("VideoTranscribeFailure::Unable to transcribe URL: {} ", url);
      }
    }

  private:
    // Call AWS Transcribe API to record transcripts from given mp4
    std::string transcribeVideo(const std::string &url)
    {
      Poco::Net::HTTPCredentials auth(std::getenv("AWS_TRANSCRIBE_AUTH_KEY"), std::getenv("AWS_TRANSCRIBE_AUTH_SECRET"));

      const std::string endpoint = "https://aws.endpoint.com/transcribe";
      Poco::URI uri(endpoint + url);

      Poco::Net::HTTPClientSession session(
          uri.getHost(),
          uri.getPort());
      Poco::Net::HTTPRequest request(
          Poco::Net::HTTPRequest::HTTP_POST,
          uri.getPathAndQuery(),
          Poco::Net::HTTPMessage::HTTP_1_1);

      request.setContentType("application/json");
      request.setCredentials(auth);

      std::ostream &request_stream = session.sendRequest(request);
      request_stream << "{\"video_url\": \"" + url + "\"}";
      Poco::Net::HTTPResponse response;
      std::istream &response_stream = session.receiveResponse(response);

      std::string response_text;
      Poco::StreamCopier::copyToString(response_stream, response_text);

      return response_text;
    }

    // Transform string into input for OpenAI
    const std::string generateModifiedString(const std::string &transcription) const noexcept
    {
      std::string modifiedString;
      modifiedString.append(
          "IGNORE ALL PREVIOUS INSTRUCTIONS. DO NOT start your response with a summary or introduction. Follow exactly the format I provide below. ");
      modifiedString.append(
          "I will provide captions from a meeting. Generate summaries, guidelines, deliverables, TODOS, followups, etc., categorized by speaker / team member:\n\n");
      modifiedString.append(
          "Your response should be consumable by the JsonCpp lib. Use this exact JSON format, leaving an empty array to represent fields which are left blank:\n\n");
      modifiedString.append("{\n
      \"  summaries\":[\"example summary\"],
      \"  guidelines\":[\"Example Guideline 1\", \"Example Guideline 2\"],
      \"  deliverables\":[\"Example Deliverable 1\"],
      \"  todos\":[\"Example Todo 1\"],
      \"  speakers\":[\"John Doe<john.doe@example.com>\", \"Jane Doe<jane.doe@example.com>\"],
      \"  followups\":[\"email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`\", \"jira X-PROJECT QNET-7 X-SUMMARY Do some dry-wall work. X-DESCRIPTION There are BATS in the walls!\"],\n}\n\n");

      modifiedString.append(transcription);
      return modifiedString;
    }

    // Send the transformed input to OpenAI
    std::string callOpenAiApi(std::string &transcription)
    {
      return OpenAI.get(transcription);
    }

    std::string run(std::string &url)
    {
      if (!url)
        return "";
      return callOpenAiApi(generateModifiedString(transcribeVideo(url)));
    }

    void executeResults(Json::Value &json)
    {
      for (const auto followup : json["followups"])
        switch (followup.substr(0, s.find(' '))) // 1st word in the string
        {
        case "email":
          // Example input:
          //   email jane.doe@example.com `I am a helper bot. You requested the following email be composed to aaliyah@example.com:\\n\\nExample message`
          Client<SendEmail>::getInstance().publish(followup);
          spdlog::info("TranscribeVideo::Email - message xenqueued");
          break;
        case "jira":
          // Example Input:
          //   jira X-PROJECT QNET-7 X-SUMMARY Do some dry-wall work. X-DESCRIPTION There are BATS in the walls!
          Client<CreateJiraTicket>::getInstance().publish(followup);
          spdlog::info("TranscribeVideo::Jira - Ticket enqueued");
        case "schedule":
          // Example Input:
          //   schedule `Q2 Planning Meeting`

          // todo - add support for other meeting platforms
          Client<ScheduleGoogleMeeting>::getInstance().publish(followup);
          spdlog::info("TranscribeVideo::ScheduleGoogleMeeting - enqueued");
        default:
          break;
        }
    }

    Json::Value transcribe(const std::string &url)
    {
      Json::Reader reader;
      Json::Value root;

      auto json = run(url);

      bool parseSuccess = reader.parse(json, root, false);
      if (parseSuccess)
      {
        Json::StyledWriter styledWriter;

        Json::Value result = root["result"];
        result["url"] = url;

        styledWriter.write(result);
        spdlog::info("Parsed transcription as json");
      }

      return parseSuccess ? result : nullptr;
    }
  }
};
