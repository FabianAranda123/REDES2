gnome-terminal -e 'sh -c "gcc Master.c -o master; ./master ;sleep 10000"'
sleep 1
gnome-terminal -e 'sh -c "./Master/s.sh ;sleep 10000"'
sleep 1
gnome-terminal -e 'sh -c "./Mirrors/mirrors.sh ;sleep 10000"'
sleep 1
gnome-terminal -e 'sh -c "./Workers/workers.sh ;sleep 10000"'
