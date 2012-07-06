#include <cstdio>
#include <libtcod/libtcod.hpp>

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

    public:
        TestState()
        {
        }

        ~TestState()
        {
        }

        void handleInput(TCOD_key_t key)
        {
        }

        void logic()
        {
        }

        void draw()
        {
            TCODConsole::root->setCharForeground(2, 5, TCODColor::white);
            TCODConsole::root->setChar(2, 5, '@');
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
                currentState->logic();
                currentState->draw();

                TCODConsole::flush();

                // Handle Input
                // key = TCODConsole::check_for_keypress()  //real-time
                TCOD_key_t key = TCODConsole::waitForKeypress(true);  //turn-based
                if (key.vk == TCODK_ESCAPE)
                {
                    running = false;
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
