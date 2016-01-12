#include <gpio.h> // gpio pin controllling

int main()
{
    // defines the variable ledPin and refers it to an GPIO-Pin
    auto ledPin = gpio::output_pin(15);

    // loop 10 times
    for (int i = 0; i < 10; i++) {
        // pin is set true, pin is HIGH, LED is on
        ledPin.set_state(true);
        // pause for 500ms, status is on hold and stays true
        delay(500);

        // pin set false again, pin is LOW, LED is off
        ledPin.set_state(false);
        // 500ms pause
        delay(500);
    }

    return EXIT_SUCCESS;
}
