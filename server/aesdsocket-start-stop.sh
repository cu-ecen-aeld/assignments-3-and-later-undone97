#! /bin/sh

case "$1" in
    start)
            echo "starting socket"
            start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
            ;;
    stop)
            echo "stopping socket"
            start-stop-daemon -K -n aesdsocket
            ;;
    *)
            echo "wrong usage. Select start or stop"
    esac
exit 0