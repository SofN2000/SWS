#include "uri.hh"

namespace http
{
    static void replace_all_substring(std::string &str,
                                      const std::string &old_sub,
                                      const std::string &new_sub)
    {
        auto npos = std::string::npos;
        auto old_sub_size = old_sub.size();
        for (auto i = str.find(old_sub, 0); i != npos; i = str.find(old_sub, i))
        {
            str.replace(i, old_sub_size, new_sub);
        }
    }

    void process_target_resource(std::string &target)
    {
        replace_all_substring(target, "/../", "/");
        replace_all_substring(target, "/./", "/");
    }
} // namespace http
