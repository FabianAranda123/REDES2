
gnome-terminal -e 'sh -c "gcc mirror.c -o ./M1/M1; cd ./M1; ./M1 192.168.1.192 6667;sleep 10000"'

gnome-terminal -e 'sh -c "gcc mirror.c -o ./M2/M2; cd ./M2; ./M2 192.168.1.192 7778;sleep 10000"'

gnome-terminal -e 'sh -c "gcc mirror.c -o ./M3/M3; cd ./M3; ./M3 192.168.1.192 8889;sleep 10000"'


