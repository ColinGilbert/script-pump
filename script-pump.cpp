#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

#include <boost/filesystem.hpp>
#include <NetworkHost.hpp>

struct handler_functor
{
	void operator()(const std::string& s)
	{
		// No-op :)
	}
};


int main(int argc, char* argv[])
{
	boost::filesystem::path init("init.as");
	boost::filesystem::path update("update.as");
	boost::filesystem::path current_cmd("current_cmd.as");


	noob::network_host<handler_functor> network;

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

	while (true)
	{
		network.tick();
		//{
		//	fmt::MemoryWriter ww;
		//	ww << "[ScriptPump] Attempting to load " << init.generic_string();
		//	std::cout << ww.str() << std::endl;
		//}

		if (!boost::filesystem::exists(init))
		{
		//	fmt::MemoryWriter ww;
		//	ww << "[ScriptPump] File " << init.generic_string() << " does not exist!";
		//	std::cout << ww.str() << std::endl;
		}
		else
		{
			if (!boost::filesystem::is_regular_file(init))
			{
		//		fmt::MemoryWriter ww;
		//		ww << "[ScriptPump] Could not use " << init.generic_string() << " as it is not a regular file.";
		//		std::cout << ww.str() << std::endl;
			}
			else
			{
				boost::system::error_code ec;
				std::time_t last_write = boost::filesystem::last_write_time(init, ec);
				auto current_time = std::chrono::system_clock::now();
				if (last_write > std::chrono::system_clock::to_time_t(current_time))
				{
					if (ec != 0)
					{
		//				fmt::MemoryWriter ww;
		//				ww << "[ScriptPump] Error getting to file at " << init.generic_string() << ", " <<  ec.message();
		//				std::cout << ww.str() << std::endl;
					}
					else
					{	
						fmt::MemoryWriter ww;
						std::ifstream f(init.generic_string());
						std::stringstream buffer;
						buffer << f.rdbuf();
						ww << "INIT: " << buffer.str();
						std::vector<uint32_t> h;
						network.send(h, ww.str(), 2, true);

						fmt::MemoryWriter log;
						log << "[ScriptPump] Sending file " << init.generic_string();
						std::cout << log.str() << std::endl;
					}

					// current_time isn't so current anymore but that's okay.
					last_write = std::chrono::system_clock::to_time_t(current_time);

					std::this_thread::sleep_for(std::chrono::milliseconds(25));
				}
			}
		}
	}
}
