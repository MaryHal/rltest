#include <cstdio>
#include <libtcod/libtcod.hpp>

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

        void handleInput(TCOD_key_t key)
        {
            putchar(key.c);
            fflush(stdout);
            if (key.pressed)
            {
                if (key.c == 'k')
                    --y;
                else if (key.c == 'j')
                    ++y;
                else if (key.c == 'h')
                    --x;
                else if (key.c == 'l')
                    ++x;
                else if (key.c == 'y')
                {
                    --y; --x;
                }
                else if (key.c == 'u')
                {
                    --y; ++x;
                }
                else if (key.c == 'b')
                {
                    ++y; --x;
                }
                else if (key.c == 'n')
                {
                    ++y; ++x;
                }
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

        virtual void handleInput(TCOD_key_t key) = 0;
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

        void handleInput(TCOD_key_t key)
        {
            me.handleInput(key);
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

            TCODConsole::initRoot(80, 40, "Test Game", false, TCOD_RENDERER_SDL);
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

                //TCOD_key_t key = TCODConsole::checkForKeypress();  //real-time
                TCOD_key_t key = TCODConsole::waitForKeypress(false);  //turn-based
                if (key.vk == TCODK_ESCAPE)
                {
                    running = false;
                    continue;
                }

                currentState->handleInput(key);
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
