#include <iostream>
#include <openssl/ssl.h>
#include <signal.h>

#include "config/config.hh"
#include "run/run.hh"

static bool check_args(int argc, char **argv)
{
    if (argc == 3)
        return std::string(argv[1]) == "-t";
    return argc == 2 && std::string(argv[1]) != "-t";
}

static void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        EVP_cleanup();
        exit(0);
    }
}

int main(int argc, char **argv)
{
    signal(SIGINT, signal_handler);
    if (!check_args(argc, argv))
    {
        std::cerr << "Usage: ./spider [-t] config_file.json\n";
        return 1;
    }
    try
    {
        auto server_config = http::ServerConfig(argv[argc - 1]);
        if (argc == 3)
            return 0;
        http::run(server_config, argv);
    }
    catch (std::exception &e)
    {
        std::cerr << "Input file invalid.\n";
        return 1;
    }
    return 0;
}
