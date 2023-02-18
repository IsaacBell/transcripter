#include <iostream>
#include <span>
#include <string>
#include <regex>
#include <format>

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/MailRecipient.h>

struct EmailAttrs
{
public:
  std::string_view body;
  std::string_view sender;
  std::string_view subject;
  std::span<std::string_view> recipients;

  bool hasValue() const
  {
    return !body.empty() && !sender.empty() && !subject.empty() && !recipients.empty();
  }
  ,
};

/*
  std::string_view input = "email from:sender@example.com subject:Test email This is a test email to:recipient1@example.com, recipient2@example.com";

  Mail::send(input);
*/

class Mail
{
public:
  static bool send(const std::string_view input)
  {
    auto attrs = compose(input);
    if (!attrs.hasValue())
      return false;
    return sendEmail(*attrs);
  }

private:
  static bool sendEmail(const EmailAttrs &attrs)
  {
    try
    {
      Poco::Net::MailMessage message;
      message.setSender(attrs.sender.data());
      message.setSubject(attrs.subject.data());
      message.setContent(attrs.body.data());

      message.clearRecipients();
      message.addRecipients(attrs.recipients.data(), attrs.recipients.size());

      Poco::Net::SMTPClientSession session(std::getenv("SMTP_SERVER_HOST") ?: "smtp.gmail.com");

      session.login();
      session.sendMessage(message);
      session.close();
      return true;
    }
    catch (const Poco::Exception &ex)
    {
      std::cerr << std::format("MailSendError: {}", ex.displayText()) << '\n';
      return false;
    }
  }

  static std::optional<EmailAttrs> compose(const std::string_view &input)
  {
    std::string_view command;
    EmailAttrs attrs;

    // extract the first word of the string
    const std::regex wordRegex("[^\\s]+");
    std::smatch wordMatch;
    if (std::regex_search(input, wordMatch, wordRegex))
    {
      command = wordMatch.str();
    }

    if (command != "email")
    {
      std::cerr << std::format("Invalid command: {}", command) << '\n';
      return std::nullopt;
    }

    // extract sender's email
    const std::regex emailRegex("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}");
    std::smatch emailMatch;
    if (std::regex_search(input, emailMatch, emailRegex))
    {
      attrs.sender = emailMatch.str();
    }

    // extract email body
    const auto tickPos = input.find('`');
    attrs.body = input.substr(tickPos + 1).remove_prefix(1);
    attrs.body = attrs.body.substr(0, attrs.body.find('`'));

    // recipients
    const std::regex recipientsRegex("(?:[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(?:,\\s*)?)+");
    std::sregex_iterator recipientsIter(input.begin(), input.end(), recipientsRegex);
    std::sregex_iterator recipientsEnd;
    for (; recipientsIter != recipientsEnd; ++recipientsIter)
    {
      const auto &recipientMatch = *recipientsIter;
      attrs.recipients.emplace_back(recipientMatch.str());
    }

    // extract subject
    const auto startPos = input.find("subject:") + 8;
    const auto endPos = input.find('`', startPos);
    attrs.subject = input.substr(startPos, endPos - startPos);

    return attrs;
  }
};
