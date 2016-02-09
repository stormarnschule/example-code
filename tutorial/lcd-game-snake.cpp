// hardware
#include <gpio.h>    // gpio pin controlling
#include <pcd8544.h> // controlling the lcd (phillips 8544)
// operating system
#include <signal.h> // ctrl-c handler
#include <unistd.h> // general os structs
// output
#include <iostream> // std::cout
// game logic
#include <chrono> // timing the game
#include <random> // used for spawning food
#include <vector> // snake is a vector of points

const auto Scale = 2, SnakeLength = 5, FoodSpawnInterval = 30;

const auto TopMargin = 8, Width = (pcd8544::width - 2) / Scale,
           Height = (pcd8544::height - 2 - TopMargin) / Scale;

const auto UpdateInterval = std::chrono::milliseconds(200);

uintmax_t Tick = 0, Score = 0;

volatile bool running = true;

// buttons
auto button_left = gpio::button_pin(24), button_right = gpio::button_pin(25);

bool is_button_pressed = false;

// types
enum class direction { up, right, down, left };

std::string to_string(const direction& d)
{
    switch (d) {
    case direction::up:
        return "up";
    case direction::right:
        return "right";
    case direction::down:
        return "down";
    case direction::left:
        return "left";
    default:
        return "";
    }
}

struct point {
    int x, y;
};

// ctr-c handler
void my_handler(int s)
{
    std::cout << "Caught signal " << s << "\n";
    running = false;
}

void setup_handler()
{
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

// main game functions
void init(), tick(pcd8544 &lcd), render(pcd8544 &lcd); // main game loop
void input(), move();                                  // game update logic

int main()
{
    setup_handler();

    std::cout << "setting up lcd\n";

    // dc = 22, sce = 17, rst = 27
    auto lcd = pcd8544(22, 17, 27);
    lcd.set_contrast(60);

    std::cout << "setting up game\n";

    // init game
    init();
    render(lcd);

    std::cout << "running game\n";

    while (running) {
        tick(lcd);
    }

    return EXIT_SUCCESS;
}

void tick(pcd8544& lcd)
{
    static std::chrono::time_point<std::chrono::high_resolution_clock> s_last_tick;
    auto now = std::chrono::high_resolution_clock::now();

    input(); // update input every time possible

    // move snake and render only at certain times
    // this controls the snake's speed and frame rate
    if (now - s_last_tick >= UpdateInterval) {
        move();
        render(lcd);

        s_last_tick = now;
        Tick++;
    }
}

// game logic variables
std::vector<point> snake = {};
point food;
direction dir;

void init()
{
    dir = direction::left;

    for (auto x = Width / 2 - SnakeLength / 2, i = 0; i < SnakeLength; i++, x++) {
        snake.push_back({ x, Height / 2 });
    }
}

std::default_random_engine::result_type rnd()
{
    static auto s_rnd
        = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    return s_rnd();
}

point rnd_point()
{
    int x = rnd() % Width, y = rnd() % Height;
    return { x, y };
}

void spawn_food()
{
    auto p = rnd_point();
    food = p;
}

void move_segment(point& p, bool back = false)
{
    switch (dir) {
    case direction::up:
        if (!back)
            p.y--;
        else
            p.y++;
        break;
    case direction::down:
        if (!back)
            p.y++;
        else
            p.y--;
        break;
    case direction::left:
        if (!back)
            p.x--;
        else
            p.x++;
        break;
    case direction::right:
        if (!back)
            p.x++;
        else
            p.x--;
        break;
    }

    if (p.x > Width - 1)
        p.x = 0;
    else if (p.x < 0)
        p.x = Width - 1;

    if (p.y > Height - 1)
        p.y = 0;
    else if (p.y < 0)
        p.y = Height - 1;
}

void input()
{
    auto oldDirection = dir;

    if (is_button_pressed == false) {
        // left button
        if (button_left.state()) {
            switch (dir) {
            case direction::left:
                dir = direction::down;
                break;
            case direction::up:
                dir = direction::left;
                break;
            case direction::down:
                dir = direction::right;
                break;
            case direction::right:
                dir = direction::up;
                break;
            }
        }

        // right button
        else if (button_right.state()) {
            switch (dir) {
            case direction::left:
                dir = direction::up;
                break;
            case direction::up:
                dir = direction::right;
                break;
            case direction::down:
                dir = direction::left;
                break;
            case direction::right:
                dir = direction::down;
                break;
            }
        }

        // no button
        else {
            return;
        }

        std::cout << "changed dir from " << to_string(oldDirection) << " to " << to_string(dir)
                  << "\n";
        is_button_pressed = true;
        std::cout << "is_button_pressed = " << is_button_pressed << "\n";
    }

    // reset is_button_pressed
    if (!button_left.state() && !button_right.state()) {
        is_button_pressed = false;
        std::cout << "is_button_pressed = " << is_button_pressed << "\n";
    }
}

void move()
{
    // move snake
    bool ateFood = false;
    auto i = 0;
    point oldP{};
    for (auto& p : snake) {
        // set new direction

        if (i == 0) {
            oldP = p;
            move_segment(p);
        }
        else {
            auto tempP = p;
            p = oldP;
            oldP = tempP;
        }
        // move segment

        // check food collision on head (i = 0)
        if (i == 0 && p.x == food.x && p.y == food.y) {
            Score++;
            std::cout << "Om nom nom nom, score: " << Score << "\n";
            ateFood = true;
        }

        i++;
    }

    // add new segment if food was eaten
    if (ateFood) {
        auto last = snake.at(snake.size() - 1);
        move_segment(last, true);
        snake.push_back(last);
    }

    // spawn food randomly, or if snake ate food
    if (ateFood || Tick % FoodSpawnInterval == 0) {
        spawn_food();
    }
}

inline void draw_px(pcd8544& lcd, int x, int y) { lcd.draw_pixel(x + 1, y + 1 + TopMargin, true); }

void draw_header(pcd8544& lcd)
{
    static const std::vector<std::vector<uint8_t>> text_bitmap = {
        { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
        { 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 },
        { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1 }
    };

    static const std::vector<std::vector<uint8_t>> digits = {
        { 1, 1, 1, 1 }, { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 },

        { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },

        { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 1, 1, 1, 1 }, { 1, 0, 0, 0 }, { 1, 1, 1, 1 },

        { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 1, 1, 1, 1 },

        { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },

        { 1, 1, 1, 1 }, { 1, 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 1, 1, 1, 1 },

        { 1, 1, 1, 1 }, { 1, 0, 0, 0 }, { 1, 1, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 },

        { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 },

        { 1, 1, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 },

        { 1, 1, 1, 1 }, { 1, 0, 0, 1 }, { 1, 1, 1, 1 }, { 0, 0, 0, 1 }, { 1, 1, 1, 1 },
    };

    for (size_t y = 0; y < text_bitmap.size(); y++) {
        auto& row = text_bitmap.at(y);
        for (size_t x = 0; x < row.size(); x++) {
            lcd.draw_pixel(x + 2, y + 2, row.at(x));
        }
    }

    auto score = Score % 100;
    auto firstDigit = score / 10, secondDigit = score - firstDigit * 10;

    // first score digit
    if (firstDigit != 0) {
        for (size_t y = 0; y < 5; y++) {
            auto& row = digits.at(5 * (firstDigit) + y);
            for (size_t x = 0; x < row.size(); x++) {
                lcd.draw_pixel(pcd8544::width - 1 - (row.size() + 1) * 2 + x, y + 2, row.at(x));
            }
        }
    }

    // second score digit
    for (size_t y = 0; y < 5; y++) {
        auto& row = digits.at(5 * (secondDigit) + y);
        for (size_t x = 0; x < row.size(); x++) {
            lcd.draw_pixel(pcd8544::width - 1 - (row.size() + 1) * 1 + x, y + 2, row.at(x));
        }
    }
}

void render(pcd8544& lcd)
{
    lcd.clear_display();

    // border
    for (size_t x = 0; x < pcd8544::width; x++) {
        lcd.draw_pixel(x, 0, true);
        lcd.draw_pixel(x, TopMargin, true);
        lcd.draw_pixel(x, pcd8544::height - 1, true);
    }
    for (size_t y = 0; y < pcd8544::height; y++) {
        lcd.draw_pixel(0, y, true);
        lcd.draw_pixel(pcd8544::width - 1, y, true);
    }

    // header
    draw_header(lcd);

    // snake
    for (auto& p : snake) {
        auto x = p.x * Scale, y = p.y * Scale;

        for (auto dx = 0; dx < Scale; dx++)
            for (auto dy = 0; dy < Scale; dy++)
                draw_px(lcd, x + dx, y + dy);
    }

    // food
    draw_px(lcd, food.x * Scale - 1, food.y * Scale);
    draw_px(lcd, food.x * Scale, food.y * Scale);
    draw_px(lcd, food.x * Scale, food.y * Scale + 1);

    lcd.display();
}
