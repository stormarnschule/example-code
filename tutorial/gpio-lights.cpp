#include <gpio.h> // gpio pin controllling
#include <iostream> // std::cout for the usage of outputs
#include <vector> // creating a list of buttons

int main()
{
    // creates a list of all used pins and defines them as inputs
    std::vector<gpio::output_pin> pins = {
        gpio::output_pin(25), gpio::output_pin(24), gpio::output_pin(23),
        gpio::output_pin(22), gpio::output_pin(27), gpio::output_pin(18),
        gpio::output_pin(17), gpio::output_pin(15), gpio::output_pin(14),
        gpio::output_pin(11), gpio::output_pin(10), gpio::output_pin(9)
    };

    // permanent loop in which the LEDS light up
    while (true) {

        // goes through the list of pins
        for (auto it = pins.begin(); it != pins.end(); it++) {
            // sets the LED on
            it->set_state(true);
            std::cout << "Lampe " << it->number() << ": " << it->state() << std::endl;

            // zeitliche Verzögerung um 300ms
            delay(300);

            // sets the LED off
            it->set_state(false);
            std::cout << "Lampe " << it->number() << ": " << it->state() << std::endl;
        }

        // goes reversed through the list of pins
        for (auto it = pins.rbegin(); it != pins.rend(); it++) {
            // sets the LED on
            it->set_state(true);
            std::cout << "Lampe " << it->number() << ": " << it->state() << std::endl;

            // zeitliche Verzögerung um 300ms
            delay(300);

            // sets the LED off
            it->set_state(false);
            std::cout << "Lampe " << it->number() << ": " << it->state() << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
