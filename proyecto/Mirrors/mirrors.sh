
gnome-terminal -e 'sh -c "gcc mirror.c -o ./M1/M1; cd ./M1; ./M1 127.0.0.1 6667;sleep 10000"'

gnome-terminal -e 'sh -c "gcc mirror.c -o ./M2/M2; cd ./M2; ./M2 127.0.0.1 7778;sleep 10000"'

gnome-terminal -e 'sh -c "gcc mirror.c -o ./M3/M3; cd ./M3; ./M3 127.0.0.1 8889;sleep 10000"'


