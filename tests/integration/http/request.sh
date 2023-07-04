#!/usr/bin/env sh

if [ -z "${SWS_VERSION}" ]; then
    export SWS_VERSION="1.1"
fi

if ! command -v nc > /dev/null; then
    exit 0
fi

killall -q spider

while pgrep -u $UID -x spider >/dev/null; do
    sleep 1;
done

./spider tests/json_config/"${SWS_CONF}".json > /dev/null &

sleep 2

if ! pgrep spider > /dev/null; then
    echo "spider not running" > /tmp/spiderlog
    exit 1
fi

echo -ne "${SWS_METHOD} ${SWS_TARGET} HTTP/${SWS_VERSION}\r\nHost: ${SWS_HOST}\r\nConnection: ${SWS_CON}\r\n\r\n" | nc localhost 8000 > /tmp/spider-nc-out

killall -q nc

while pgrep -u $UID -x nc >/dev/null; do
    sleep 1;
done

if [ ${SWS_FIELD} ]; then
    cat /tmp/spider-nc-out | grep "${SWS_FIELD}" | cut -d ':' -f 2 > /tmp/spiderlog
fi

cat /tmp/spider-nc-out | grep -E "^HTTP/" | cut -d ' ' -f 2 > /tmp/spidercode

killall -q spider

while pgrep -u $UID -x spider >/dev/null; do
    sleep 1;
done

if  [ "${SWS_EXPECTED}" ]; then
    if ! echo -ne " ${SWS_EXPECTED}\r\n" | diff /tmp/spiderlog - ; then
        exit 1
    fi
fi

if ! echo "${SWS_CODE}" | diff /tmp/spidercode - ; then
    exit 2
fi
