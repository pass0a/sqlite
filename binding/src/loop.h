#include <boost/asio.hpp>

boost::asio::io_service& loopGet();
void loopSet(boost::asio::io_service* io);