#termEmulator=konsole
#termFlags="--noclose --nofork -e"

termEmulator=urxvt
termFlags="-e"

<<terminal_selector
terms=(konsole gnome-terminal urxvt rxvt x-terminal-emulator termit terminator)
# termFlags = (...)
for t in ${terms[*]}
do
    if [ $(command -v $t) ]
    then
        detected_term=$t
        break
    fi
done 
terminal_selector

$termEmulator $termFlags ./virtSerial.sh &
sleep 10
$termEmulator $termFlags ./testApp /dev/ttyS11 rx &
$termEmulator $termFlags ./testApp /dev/ttyS10 tx &