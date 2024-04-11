#include "hal/buzzer.h"

#include <iostream>
#include <unistd.h>

bool enabled = false;
buzzer::buzzer(bool correct)
{
    if (!enabled)
    {
        // config the pin
        system("config-pin p9_22 pwm");

        enabled = true;
    }

    if (correct)
    {
        // set the period
        system("echo 2000000 > /dev/bone/pwm/0/a/period");
        system("echo 1000000 > /dev/bone/pwm/0/a/duty_cycle");
    }
    else
    {
        // set the period
        system("echo 4000000 > /dev/bone/pwm/0/a/period");
        system("echo 100000 > /dev/bone/pwm/0/a/duty_cycle");
        system("echo 1 > /dev/bone/pwm/0/a/enable");
        // sleep for 500ms
        usleep(500000);
        system("echo 0 > /dev/bone/pwm/0/a/enable");
    }

    system("echo 1 > /dev/bone/pwm/0/a/enable");
    sleep(1);
    system("echo 0 > /dev/bone/pwm/0/a/enable");
}

buzzer::~buzzer()
{
    system("echo 0 > /dev/bone/pwm/0/a/enable");
}
