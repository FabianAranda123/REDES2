gnome-terminal -e 'sh -c "gcc worker.c -o ./W1/W1; cd ./W1; ./W1 192.168.1.192 6667 192.168.1.192 6666;sleep 10000"'

gnome-terminal -e 'sh -c "gcc worker.c -o ./W2/W2; cd ./W2; ./W2 192.168.1.192 7778 192.168.1.192 7777;sleep 10000"'

gnome-terminal -e 'sh -c "gcc worker.c -o ./W3/W3; cd ./W3; ./W3 192.168.1.192 8889 192.168.1.192 8888;sleep 10000"'

