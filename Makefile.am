NULL =

EXTRA_DIST = autogen.sh
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
              -include $(top_builddir)/config.h \
              -I${top_srcdir}/src \
              -I${top_srcdir}/lib

AM_CXXFLAGS = \
              $(PICKY_CXXFLAGS) \
              $(WARN_CXXFLAGS) \
              $(BOOST_CPPFLAGS) \
              -Werror=unused-parameter \
              -Werror=missing-field-initializers \
              -Werror=overloaded-virtual \
              -Wno-inline \
	      -fsanitize=address \
	      -g \
	      -Og \
              $(NULL)

AM_LDFLAGS = \
             $(WARN_LDFLAGS) \
             $(BOOST_LDFLAGS) \
             -lev \
             -lssl \
             -lcrypto \
			 			-lstdc++fs \
             $(NULL)

AM_LDFLAGS += \
              -Wno-unused-command-line-argument \
	      -fsanitize=address \
              $(NULL)

bin_PROGRAMS = spider
lib_LTLIBRARIES = libspider.la

SOURCES = \
          src/config/config.cc \
          src/config/config.hh \
          src/error/init-error.hh \
          src/error/not-implemented.hh \
          src/error/parsing-error.hh \
          src/events/event-loop.cc \
          src/events/event-loop.hh \
          src/events/events.cc \
          src/events/events.hh \
          src/events/listener.cc \
          src/events/listener.hh \
          src/events/recv-response-body.cc \
          src/events/recv-response-body.hh \
          src/events/recv-response-header.cc \
          src/events/recv-response-header.hh \
          src/events/recvbody.cc \
          src/events/recvbody.hh \
          src/events/recvheader.cc \
          src/events/recvheader.hh \
          src/events/register.cc \
          src/events/register.hh \
          src/events/register.hxx \
          src/events/send-request.cc \
          src/events/send-request.hh \
          src/events/send-response.cc \
          src/events/send-response.hh \
          src/misc/addrinfo/addrinfo-error.cc \
          src/misc/addrinfo/addrinfo-error.hh \
          src/misc/addrinfo/addrinfo-iterator.hh \
          src/misc/addrinfo/addrinfo.cc \
          src/misc/addrinfo/addrinfo.hh \
          src/misc/exceptions.cc \
          src/misc/exceptions.hh \
          src/misc/fd.cc \
          src/misc/fd.hh \
	  src/misc/file_to_string.cc \
	  src/misc/file_to_string.hh \
	  src/misc/inet.cc \
	  src/misc/inet.hh \
          src/misc/json.hh \
          src/misc/openssl/base64.cc \
          src/misc/openssl/base64.hh \
          src/misc/openssl/ssl-error.cc \
          src/misc/openssl/ssl-error.hh \
          src/misc/openssl/ssl-wrapper.hh \
          src/misc/openssl/ssl.hh \
          src/misc/readiness/readiness.cc \
          src/misc/readiness/readiness.hh \
          src/misc/socket.hh \
          src/misc/sys-wrapper.hh \
          src/misc/unistd.hh \
          src/misc/uri.cc \
          src/misc/uri.hh \
          src/request/error.cc \
          src/request/error.hh \
          src/request/request.cc \
          src/request/request.hh \
          src/request/response.cc \
          src/request/response.hh \
          src/request/types.hh \
          src/run/run.cc \
          src/run/run.hh \
          src/socket/default-socket.cc \
          src/socket/default-socket.hh \
          src/socket/socket.hh \
          src/socket/ssl-socket.cc \
          src/socket/ssl-socket.hh \
          src/vhost/connection.hh \
          src/vhost/dispatcher.cc \
          src/vhost/dispatcher.hh \
          src/vhost/load-balancer-factory.cc \
          src/vhost/load-balancer-factory.hh \
          src/vhost/load-balancer.cc \
          src/vhost/load-balancer.hh \
          src/vhost/vhost-factory.cc \
          src/vhost/vhost-factory.hh \
          src/vhost/vhost-reverse-proxy.cc \
          src/vhost/vhost-reverse-proxy.hh \
	  src/vhost/vhost-static-file.cc \
          src/vhost/vhost-static-file.hh \
          src/vhost/vhost.cc \
          src/vhost/vhost.hh \
          $(NULL)


# Compile source files again to avoid silenting undefined references.
spider_SOURCES = \
                 $(SOURCES) \
                 src/main.cc \
                 $(NULL)

# https://www.gnu.org/software/automake/manual/html_node/Objects-created-both-with-libtool-and-without.html
spider_CXXFLAGS = $(AM_CXXFLAGS)

libspider_la_SOURCES = \
                       $(SOURCES) \
                       $(NULL)

# Configure TAP driver
TEST_LOG_DRIVER = \
    env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh
TESTS = tests/integration/launch.sh

if USING_GTEST
check_PROGRAMS = unit_test
TESTS += $(check_PROGRAMS)

unit_test_SOURCES = \
                    $(SOURCES) \
                    tests/unit/main.cc \
                    tests/unit/config/config.cc \
                    tests/unit/vhost/process-target-resource.cc
unit_test_CXXFLAGS = $(AM_CXXFLAGS) $(GTEST_CFLAGS)
unit_test_CPPFLAGS = $(AM_CPPFLAGS)
unit_test_LDFLAGS = $(AM_LDFLAGS) $(GTEST_LIBS)
endif
