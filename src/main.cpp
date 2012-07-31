#include <cstdio>
#include <libtcod/libtcod.hpp>

#include <map>

struct Point
{
    int x;
    int y;

    Point()
        : x(0),
          y(0)
    {
    }
    
    Point(int x, int y)
        : x(x),
          y(y)
    {
    }
};

class Input
{
    private:
        std::map<int, Point> movement;

    public:
        Input()
        {
            movement['h'] = Point(-1,  0);
            movement['j'] = Point( 0,  1);
            movement['k'] = Point( 0, -1);
            movement['l'] = Point( 1,  0);
            movement['y'] = Point(-1, -1);
            movement['u'] = Point( 1, -1);
            movement['b'] = Point(-1,  1);
            movement['n'] = Point( 1,  1);
        }

        Point getMove(int ch)
        {
            return movement[ch];
        }

        TCOD_key_t getKey()
        {
            //return TCODConsole::checkForKeypress();
            return TCODConsole::waitForKeypress(false);  //turn-based
        }
};

class Player
{
    private:
        int x;
        int y;

    public:
        Player()
            : x(0), y(0)
        {
        }

        void handleInput(const Input& input, TCOD_key_t key)
        {
            if (key.pressed)
            {
                Input i;
                Point p = i.getMove(key.c);

                x += p.x;
                y += p.y;
            }
        }

        void draw()
        {
            TCODConsole::root->setCharForeground(x, y, TCODColor::white);
            TCODConsole::root->setChar(x, y, '@');
        }
};

class State
{
    protected:
    public:
        State() {}
        virtual ~State() {}

        virtual void handleInput(const Input& input, TCOD_key_t key) = 0;
        virtual void logic() = 0;
        virtual void draw() = 0;
};

class TestState : public State
{
    private:
        Player me;

    public:
        TestState()
        {
        }

        ~TestState()
        {
        }

        void handleInput(const Input& input, TCOD_key_t key)
        {
            me.handleInput(input, key);
        }

        void logic()
        {
        }

        void draw()
        {
            me.draw();
        }
};

class Application
{
    private:
        bool running;

        Input input;
        State* currentState;

    public:
        Application()
            : running(false),
            currentState(NULL)
        {
            initialize();
        }

        ~Application()
        {
            if (currentState)
                delete currentState;
        }

        void initialize()
        {
            TCODConsole::setCustomFont("terminal8x8_aa_tc.png", TCOD_FONT_LAYOUT_TCOD);

            TCODConsole::initRoot(80, 40, "Test Game", false, TCOD_RENDERER_GLSL);
            TCODConsole::root->setDefaultBackground(TCODColor::black);

            // This is a Turn-based game. So this should be off.
            // TCODConsole::sys_set_fps(60) 
        }

        void setState(State* state)
        {
            if (currentState)
                delete currentState;

            currentState = state;
        }

        void run()
        {
            if (!currentState)
                return;

            running = true;
            while (running && !TCODConsole::isWindowClosed())
            {
                currentState->draw();
                TCODConsole::flush();

                currentState->logic();

                TCOD_key_t key = input.getKey();
                if (key.vk == TCODK_ESCAPE)
                {
                    running = false;
                    continue;
                }

                currentState->handleInput(input, key);
            }
        }
};

int main(int argc, char** argv)
{
    Application application;
    application.setState(new TestState);

    application.run();
    return 0;
};
