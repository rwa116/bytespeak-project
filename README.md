Dependencies:

host:

    sudo apt install g++
    sudo apt install g++-arm-linux-gnueabihf
    sudo chmod 777 bytespeak-wav-files // necessary for sudo to write files when running piper

target:

    sudo apt install g++
    sudo apt install espeak
    sudo apt-get install gawk
    run application as su (sudo ./byte_speaker)