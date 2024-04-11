#ifndef BUZZER_H
#define BUZZER_H

class buzzer
{
public:
    buzzer(bool correct);
    ~buzzer();
    void play_sound();
    void stop_sound();
};

#endif 
// end code