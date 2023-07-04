#include <gtest/gtest.h>
#include <string>

#include "misc/uri.hh"

namespace http
{
    using string_couple = std::pair<std::string, std::string>;

    struct TargetResourceTest : testing::Test
    {
        std::string target;
        std::string expected;
    };

    struct TargetResourceProcessingTest
        : TargetResourceTest
        , testing::WithParamInterface<string_couple>
    {
        TargetResourceProcessingTest()
        {
            this->target = GetParam().first;
            this->expected = GetParam().second;
        }
    };

    TEST_P(TargetResourceProcessingTest, remove_relative_path)
    {
        process_target_resource(target);
        EXPECT_EQ(target, expected);
    }

    INSTANTIATE_TEST_SUITE_P(
        Default, TargetResourceProcessingTest,
        testing::Values(std::make_pair("/index.html", "/index.html"),
                        std::make_pair("/../index.html", "/index.html"),
                        std::make_pair("/../../index.html", "/index.html"),
                        std::make_pair("/.././../../../index.html",
                                       "/index.html"),
                        std::make_pair("/www/../srv/./../index.html",
                                       "/www/srv/index.html"),
                        std::make_pair("/.././www/../srv/./../index.html",
                                       "/www/srv/index.html"),
                        std::make_pair("/././www/./srv/././index.html",
                                       "/www/srv/index.html"),
                        std::make_pair("/www/srv/hey/index.html",
                                       "/www/srv/hey/index.html")));

} // namespace http
