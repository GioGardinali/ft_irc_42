
#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

namespace Replies {
    inline std::string rpl(const std::string& code, const std::string& nick, const std::string& text) {
        std::string n = nick.empty() ? "*" : nick;
        return ":" + std::string("ft_irc") + " " + code + " " + n + " " + text + "\r\n";
    }
    inline std::string RPL_WELCOME(const std::string& nick, const std::string& user, const std::string& host) {
        return ":" + std::string("ft_irc") + " 001 " + nick + " :Welcome to the ft_irc network " + nick + "!" + user + "@" + host + "\r\n";
    }
    inline std::string RPL_YOURHOST(const std::string& nick) { return rpl("002", nick, ":Your host is ft_irc, running v0.1"); }
    inline std::string RPL_CREATED(const std::string& nick) { return rpl("003", nick, ":This server was created just now"); }
    inline std::string RPL_MYINFO(const std::string& nick) { return rpl("004", nick, "ft_irc v0.1 oi ntkl"); }
    inline std::string RPL_MOTDSTART(const std::string& nick) { return ":" + std::string("ft_irc") + " 375 " + nick + " :- ft_irc Message of the day - \r\n"; }
    inline std::string RPL_MOTD(const std::string& nick, const std::string& line) { return ":" + std::string("ft_irc") + " 372 " + nick + " :- " + line + "\r\n"; }
    inline std::string RPL_ENDOFMOTD(const std::string& nick) { return ":" + std::string("ft_irc") + " 376 " + nick + " :End of /MOTD command\r\n"; }

    inline std::string ERR_UNKNOWNCOMMAND(const std::string& nick, const std::string& cmd) { return rpl("421", nick, cmd + " :Unknown command"); }
    inline std::string ERR_NEEDMOREPARAMS(const std::string& nick, const std::string& cmd) { return rpl("461", nick, cmd + " :Not enough parameters"); }
    inline std::string ERR_ALREADYREGISTRED(const std::string& nick) { return rpl("462", nick, ":You may not reregister"); }
    inline std::string ERR_PASSWDMISMATCH(const std::string& nick) { return rpl("464", nick, ":Password incorrect"); }
    inline std::string ERR_NONICKNAMEGIVEN(const std::string& nick) { return rpl("431", nick, ":No nickname given"); }
    inline std::string ERR_ERRONEUSNICKNAME(const std::string& nick, const std::string& bad) { return rpl("432", nick, bad + " :Erroneous nickname"); }
    inline std::string ERR_NICKNAMEINUSE(const std::string& nick, const std::string& taken) { return rpl("433", nick, taken + " :Nickname is already in use"); }
    inline std::string ERR_NOTREGISTERED(const std::string& nick) { return rpl("451", nick, ":You have not registered"); }
    inline std::string ERR_NOORIGIN(const std::string& nick) { return rpl("409", nick, ":No origin specified"); }

    inline std::string ERR_NOSUCHNICK(const std::string& nick, const std::string& target) { return rpl("401", nick, target + " :No such nick/channel"); }
    inline std::string ERR_NOSUCHCHANNEL(const std::string& nick, const std::string& chan) { return rpl("403", nick, chan + " :No such channel"); }
    inline std::string ERR_CANNOTSENDTOCHAN(const std::string& nick, const std::string& chan) { return rpl("404", nick, chan + " :Cannot send to channel"); }
    inline std::string ERR_NORECIPIENT(const std::string& nick, const std::string& cmd) { return rpl("411", nick, ":No recipient given (" + cmd + ")"); }
    inline std::string ERR_NOTEXTTOSEND(const std::string& nick) { return rpl("412", nick, ":No text to send"); }
    inline std::string ERR_NOTONCHANNEL(const std::string& nick, const std::string& chan) { return rpl("442", nick, chan + " :You're not on that channel"); }
    inline std::string ERR_USERONCHANNEL(const std::string& nick, const std::string& user, const std::string& chan) { return rpl("443", nick, user + " " + chan + " :is already on channel"); }
    inline std::string RPL_NAMREPLY(const std::string& nick, const std::string& chan, const std::string& names) { return ":" + std::string("ft_irc") + " 353 " + nick + " = " + chan + " :" + names + "\r\n"; }
    inline std::string RPL_ENDOFNAMES(const std::string& nick, const std::string& chan) { return ":" + std::string("ft_irc") + " 366 " + nick + " " + chan + " :End of /NAMES list.\r\n"; }
}

#endif
