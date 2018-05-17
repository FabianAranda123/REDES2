gnome-terminal -e 'sh -c "gcc worker.c -o ./W1/W1; cd ./W1; ./W1 127.0.0.1 6667 127.0.0.1 6666;sleep 10000"'

gnome-terminal -e 'sh -c "gcc worker.c -o ./W2/W2; cd ./W2; ./W2 127.0.0.1 7778 127.0.0.1 7777;sleep 10000"'

gnome-terminal -e 'sh -c "gcc worker.c -o ./W3/W3; cd ./W3; ./W3 127.0.0.1 8889 127.0.0.1 8888;sleep 10000"'

