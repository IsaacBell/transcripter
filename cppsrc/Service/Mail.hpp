#include <iostream>
#include <vector>
#include <string>
#include <regex>

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/MailRecipient.h>

struct EmailAttrs
{
  std::string body;
  std::string sender;
  std::string subject;
  std::vector<std::string> recipients;
};

class Mail
{
public:
  static bool send(const std::string &input)
  {
    auto attrs = extractEmailDetails(input);
    if (!attrs)
      return;
    return sendEmail(*attrs);
  }

private:
  static bool sendEmail(const EmailAttrs &attrs)
  {
    try
    {
      Poco::Net::MailMessage message;
      message.setSender(attrs.sender);
      message.setSubject(attrs.subject);
      message.setContent(attrs.body);

      for (const auto &recipient : attrs.recipients)
        message.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipient));

      Poco::Net::SMTPClientSession session(std::getenv("SMTP_SERVER_HOST") || "smtp.gmail.com ");

      session.login();
      session.sendMessage(message);
      session.close();
      return true;
    }
    catch (const Poco::Exception &ex)
    {
      std::cerr << "Error sending email: " << ex.what() << '\n';
      return false;
    }
  }

  static std::optional<EmailAttrs> extractEmailDetails(const std::string &input)
  {
    std::string command;
    EmailAttrs attrs;

    // extract the first word of the string
    std::regex wordRegex("[^\s]+");
    if (const auto &[wordIter, wordEnd] = std::regex_search(input, wordRegex); wordIter != wordEnd)
    {
      const auto &[wordMatch, wordMatchEnd] = *wordIter;
      command = std::string(wordMatch, wordMatchEnd);
    }

    if (command != "email")
      return {};

    // extract sender's email
    const std::regex emailRegex("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}");
    if (const auto &[emailIter, emailEnd] = std::regex_search(input, emailRegex); emailIter != emailEnd)
    {
      const auto &[emailMatch, emailMatchEnd] = *emailIter;
      attrs.sender = std::string(emailMatch, emailMatchEnd);
    }

    // extract email body
    std::size_t tickPos = input.find("`");
    attrs.body = input.substr(tickPos + 1, input.length() - tickPos - 2);

    // recipients
    const std::regex recipientsRegex("(?:[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(?:,\s*)?)+");
    std::sregex_iterator recipientsIter(input.begin(), input.end(), recipientsRegex);
    std::sregex_iterator recipientsEnd;
    for (; recipientsIter != recipientsEnd; ++recipientsIter)
    {
      const auto &[recipientMatch, recipientMatchEnd] = *recipientsIter;
      attrs.recipients.emplace_back(recipientMatch, recipientMatchEnd);
    }

    // extract subject
    std::size_t startPos = input.find("subject:") + 8;
    std::size_t endPos = input.find("`", startPos);
    attrs.subject = input.substr(startPos, endPos - startPos);

    return attrs;
  }
};
