
#include "Client.hpp"

Client::Client(int fd)
: _fd(fd), _in(), _out(), _passOK(false), _registered(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }
std::string& Client::inBuf() { return _in; }
std::string& Client::outBuf() { return _out; }

void Client::setPasswordOK(bool ok) { _passOK = ok; }
bool Client::passwordOK() const { return _passOK; }

void Client::setNick(const std::string& n) { _nick = n; }
void Client::setUser(const std::string& u) { _user = u; }
void Client::setRealName(const std::string& r) { _realname = r; }

const std::string& Client::nick() const { return _nick; }
const std::string& Client::user() const { return _user; }
const std::string& Client::realname() const { return _realname; }

bool Client::isRegistered() const { return _registered; }
void Client::setRegistered(bool v) { _registered = v; }

void Client::enqueue(const std::string& data) { _out += data; }

void Client::joinChannel(const std::string& ch) { _channels.insert(ch); }
void Client::leaveChannel(const std::string& ch) { _channels.erase(ch); }
const std::set<std::string>& Client::channelsJoined() const { return _channels; }
