#include "misc/exceptions.hh"

std::ostream &operator<<(std::ostream &o, http::RequestException &e)
{
    o << e.what() << '\n';
    return o;
}
