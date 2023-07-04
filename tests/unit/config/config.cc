#include "config/config.hh"

#include <gtest/gtest.h>
#include <string>

#include "events/register.hh"

namespace http
{
    struct ServerConfigTest : testing::Test
    {};

    TEST_F(ServerConfigTest, one_vhost)
    {
        auto path = "tests/json_config/one_vhost.json";
        auto server_config = http::ServerConfig(path);
        ASSERT_EQ(server_config.vhosts.size(), 1);
        auto vhost = *server_config.vhosts.begin();
        EXPECT_EQ(vhost.ip, "127.0.0.1");
        EXPECT_EQ(vhost.port, 8000);
        EXPECT_EQ(vhost.server_name, "localhost");
        EXPECT_EQ(vhost.root, ".");
        EXPECT_EQ(vhost.default_file, "index.html");
    }

    TEST_F(ServerConfigTest, one_vhost_ipv6)
    {
        auto path = "tests/json_config/one_vhost_ipv6.json";
        auto server_config = http::ServerConfig(path);
        ASSERT_EQ(server_config.vhosts.size(), 1);
        auto vhost = *server_config.vhosts.begin();
        EXPECT_EQ(vhost.ip, "::1");
        EXPECT_EQ(vhost.port, 8000);
        EXPECT_EQ(vhost.server_name, "localhost");
        EXPECT_EQ(vhost.root, ".");
        EXPECT_EQ(vhost.default_file, "index.html");
    }

    TEST_F(ServerConfigTest, one_vhost_custom_file)
    {
        auto path = "tests/json_config/one_vhost_custom_file.json";
        auto server_config = http::ServerConfig(path);
        ASSERT_EQ(server_config.vhosts.size(), 1);
        auto vhost = *server_config.vhosts.begin();
        EXPECT_EQ(vhost.ip, "127.0.0.1");
        EXPECT_EQ(vhost.port, 8000);
        EXPECT_EQ(vhost.server_name, "localhost");
        EXPECT_EQ(vhost.root, ".");
        EXPECT_EQ(vhost.default_file, "index.php");
    }

    TEST_F(ServerConfigTest, bad_config)
    {
        auto path = "tests/json_config/bad_config.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_config2)
    {
        auto path = "tests/json_config/bad_config2.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_ip)
    {
        auto path = "tests/json_config/impossible_ip.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_ip2)
    {
        auto path = "tests/json_config/impossible_ip2.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_ip3)
    {
        auto path = "tests/json_config/impossible_ip3.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_ip4)
    {
        auto path = "tests/json_config/impossible_ip4.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_ip5)
    {
        auto path = "tests/json_config/impossible_ip5.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_port)
    {
        auto path = "tests/json_config/impossible_port.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, impossible_port2)
    {
        auto path = "tests/json_config/impossible_port2.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_transaction_timeout)
    {
        auto path = "tests/json_config/bad_transaction_timeout.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_proxy_timeout)
    {
        auto path = "tests/json_config/bad_proxy_timeout.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_throughput_val)
    {
        auto path = "tests/json_config/bad_throughput_val.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_throughput_time)
    {
        auto path = "tests/json_config/bad_throughput_time.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, bad_keep_alive_timeout)
    {
        auto path = "tests/json_config/bad_keep_alive_timeout.json";
        try
        {
            auto server_config = http::ServerConfig(path);
            FAIL();
        }
        catch (...)
        {}
    }

    TEST_F(ServerConfigTest, timeout_transaction)
    {
        event_register.throughput_val = std::nullopt;
        event_register.throughput_time = std::nullopt;
        event_register.transaction = std::nullopt;
        event_register.keep_alive = std::nullopt;
        auto path = "tests/json_config/timeout_transaction.json";
        auto server_config = http::ServerConfig(path);
        ASSERT_EQ(event_register.transaction.has_value(), true);
        EXPECT_EQ(event_register.transaction.value(), 5);
        EXPECT_EQ(event_register.keep_alive.has_value(), false);
        EXPECT_EQ(event_register.throughput_val.has_value(), false);
        EXPECT_EQ(event_register.throughput_time.has_value(), false);
    }

    TEST_F(ServerConfigTest, timeout_keep_alive)
    {
        event_register.throughput_val = std::nullopt;
        event_register.throughput_time = std::nullopt;
        event_register.transaction = std::nullopt;
        event_register.keep_alive = std::nullopt;
        auto path = "tests/json_config/timeout_keep_alive.json";
        auto server_config = http::ServerConfig(path);
        ASSERT_EQ(event_register.keep_alive.has_value(), true);
        EXPECT_EQ(event_register.keep_alive.value(), 12);
        EXPECT_EQ(event_register.transaction.has_value(), false);
        EXPECT_EQ(event_register.throughput_val.has_value(), false);
        EXPECT_EQ(event_register.throughput_time.has_value(), false);
    }

} // namespace http
