#include <iostream>
#include <vector>
#include <string>
#include <regex>

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/SMTPClientSession.h>
#include <Poco/Net/MailRecipient.h>

struct EmailAttrs
{
public:
  std::string body;
  std::string sender;
  std::string subject;
  std::vector<std::string> recipients;
};

class Mail
{
public:
  void send(std::string &input)
  {
    sendEmail(extractEmailDetails(input));
  }

private:
  void sendEmail(const EmailAttrs &attrs)
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
  }

  EmailAttrs extractEmailDetails(std::string &input)
  {
    std::string command;
    EmailAttrs attrs;

    // extract the first word of the string
    std::regex wordRegex("[^\\s]+");
    std::smatch wordMatch;
    if (std::regex_search(input, wordMatch, wordRegex))
      command = wordMatch[0];
    if (command != "email")
      return;

    // extract sender's email
    const std::regex emailRegex("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}");
    std::smatch emailMatch;
    if (std::regex_search(input, emailMatch, emailRegex))
      attrs.sender = emailMatch[0];

    // extract email body
    std::size_t tickPos = input.find("`");
    attrs.body = input.substr(tickPos + 1, input.length() - tickPos - 2);

    // recipients
    const std::regex recipientsRegex("(?:[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(?:,\s*)?)+");
    std::sregex_iterator iter(input.begin(), input.end(), recipientsRegex);
    std::sregex_iterator end;
    for (; iter != end; ++iter)
      attrs.recipients.push_back(iter->str());

    return attrs;
  }
};
