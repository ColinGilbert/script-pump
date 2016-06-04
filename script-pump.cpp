#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

#include <boost/filesystem.hpp>
#include <NetworkHost.hpp>


static std::vector<uint32_t> hosts_to_init;


struct connection_handler_functor
{
	void operator()(uint32_t i)
	{
		hosts_to_init.push_back(i);
	}
};


struct message_handler_functor
{
	void operator()(const std::string& s)
	{
		// No-op :)
	}
};

typedef noob::network_host<message_handler_functor, connection_handler_functor> nethost;

std::string load_file(const std::string& path)
{

	fmt::MemoryWriter ww;
	std::ifstream f(path);
	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}


// The tag argument refers to the short word prepended to the file. This is so that the other host knows what to do with it.a
void send_file(const boost::filesystem::path& p, const std::string& tag, nethost& net)
{

	fmt::MemoryWriter ww;
	std::string file_contents = load_file(p.generic_string());
	ww << tag << ": " << file_contents;
	std::vector<uint32_t> h;
	net.send(h, ww.str(), 2, true);

	fmt::MemoryWriter log;
	log << "[ScriptPump] Broadcasting " << p.generic_string() << ", tag = " << tag;
	std::cout << log.str() << std::endl;
}


int main(int argc, char* argv[])
{
	boost::filesystem::path init("init");
	boost::filesystem::path update("update");
	boost::filesystem::path current_cmd("current_cmd");

	nethost network;

	std::string host_addr = "localhost";
	uint16_t port_num = 4242;
	size_t max_hosts = 12;
	size_t max_channels = 3;
	if (!network.init(host_addr.c_str(), port_num, max_hosts, max_channels))
	{
		fmt::MemoryWriter ww;
		ww << "[ScriptPump] Could not init network server at address " << host_addr << ":" << port_num << ", max hosts = " << max_hosts << ", max channels = " << max_channels << ".";
		std::cout << ww.str() << std::endl;
		return 1;
	}


	boost::system::error_code ec;
	std::time_t last_write_init, last_write_update, last_write_current_cmd;
	last_write_init = last_write_update = last_write_current_cmd = 0;

	while (true)
	{
		network.tick();
		
		std::time_t temp_last_write = boost::filesystem::last_write_time(init, ec);
		if (ec == 0)
		{
			if (temp_last_write > last_write_init)
			{
				send_file(init, "INIT", network);
				last_write_init = temp_last_write;
			}
		}

		temp_last_write = boost::filesystem::last_write_time(update, ec);
		if (ec == 0)
		{
			if (temp_last_write > last_write_update)
			{
				send_file(update, "UPDATE", network);
				last_write_update = temp_last_write;
			}
		}
		
		temp_last_write = boost::filesystem::last_write_time(current_cmd, ec);
		if (ec == 0)
		{
			if (temp_last_write > last_write_current_cmd)
			{
				send_file(current_cmd, "CMD", network);
				last_write_current_cmd = temp_last_write;
			}
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}
