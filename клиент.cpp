#include <boost/asio.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "12343"));

        std::cout << "Connected to timer server. Enter command (timer N): ";
        std::string msg;
        std::getline(std::cin, msg);
        msg += "\n";
        boost::asio::write(socket, boost::asio::buffer(msg));

        
        boost::asio::streambuf buf1;
        boost::asio::read_until(socket, buf1, '\n');
        std::istream is1(&buf1);
        std::string resp1;
        std::getline(is1, resp1);
        std::cout << resp1 << std::endl;

        
        boost::asio::streambuf buf2;
        boost::asio::read_until(socket, buf2, '\n');
        std::istream is2(&buf2);
        std::string resp2;
        std::getline(is2, resp2);
        std::cout << resp2 << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
    return 0;
}