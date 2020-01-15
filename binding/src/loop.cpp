#include "loop.h"

boost::asio::io_service* gio = NULL;

boost::asio::io_service& loopGet() {
	return *gio;
}
void loopSet(boost::asio::io_service* io) {
	gio = io;
}