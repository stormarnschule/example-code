#include <gpio.h> // gpio pin controllling
#include <iostream> // std::cout for the usage of outputs
#include <vector> // creating a list of buttons

int main()
{
    // creates a list of all used pins and defines them as buttons
    std::vector<gpio::button_pin> buttons = {
        gpio::button_pin(23),
        gpio::button_pin(24),
        gpio::button_pin(25)
    };

    // permanent loop in which the buttons ar checked
    while (true) {
        // deletes the previous output
        std::cout << "\r";

        // goes through the list of buttons
        for (auto& pin : buttons) {

            // reads the status of a button
            bool knopfAn = pin.state();

            // status output
            std::cout << "Knopf No " << pin.number() << " ist " << (knopfAn ? "an" : "aus") << ". ";
        }
    }

    return EXIT_SUCCESS;
}
