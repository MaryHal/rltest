#include <cstdio>
#include <libtcod/libtcod.hpp>

#include <map>
#include <cstring>

template <typename T> class Rect
{
    public:
    T x;
    T y;
    T w;
    T h;

    Rect() {}
    Rect(T xcoord, T ycoord, T width, T height)
        : x(xcoord), y(ycoord), w(width), h(height)
    {
    }

    bool collide(const Rect<T>& rect)
    {
        if (y > rect.y + rect.h)
            return true;
        if (y + h < rect.y)
            return true;
        if (x > rect.x + rect.w)
            return true;
        if (x + w < rect.x)
            return true;

        return false;
    }
};

typedef Rect<int> IntRect;

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
        // Vi-keys
        movement['h'] = Point(-1,  0);
        movement['j'] = Point( 0,  1);
        movement['k'] = Point( 0, -1);
        movement['l'] = Point( 1,  0);
        movement['y'] = Point(-1, -1);
        movement['u'] = Point( 1, -1);
        movement['b'] = Point(-1,  1);
        movement['n'] = Point( 1,  1);

        // Numpad
        movement[TCODK_4] = Point(-1,  0);
        movement[TCODK_2] = Point( 0,  1);
        movement[TCODK_8] = Point( 0, -1);
        movement[TCODK_6] = Point( 1,  0);
        movement[TCODK_7] = Point(-1, -1);
        movement[TCODK_9] = Point( 1, -1);
        movement[TCODK_1] = Point(-1,  1);
        movement[TCODK_3] = Point( 1,  1);

        // Arrows
        movement[TCODK_UP]    = Point( 0, -1);
        movement[TCODK_DOWN]  = Point( 0,  1);
        movement[TCODK_LEFT]  = Point(-1,  0);
        movement[TCODK_RIGHT] = Point( 1,  0);
    }

    const Point getMove(const TCOD_key_t& key)
    {
        if (key.vk == TCODK_CHAR)
            return movement[key.c];
        return movement[key.vk];
    }

    const TCOD_key_t getKey() const
    {
        //return TCODConsole::checkForKeypress();
        return TCODConsole::waitForKeypress(false);  //turn-based
    }
};

class Drawable
{
    public:
    virtual void draw(TCODConsole* console) const = 0;
};

class Player
{
    private:
    int x;
    int y;
    char glyph;

    public:
    Player()
        : x(0), y(0), glyph('@')
    {
    }

    void handleInput(Input& input, const TCOD_key_t& key)
    {
        if (key.pressed)
        {
            Point p = input.getMove(key);

            x += p.x;
            y += p.y;
        }
    }

    virtual void draw(TCODConsole* console) const
    {
        console->setCharForeground(x, y, TCODColor::white);
        console->setChar(x, y, glyph);
    }
};

class Map : public Drawable
{
    protected:
    static const int MapHeight = 20;
    static const int MapWidth  = 46;

    TCODMap tcodMap;
    TCODConsole* mapConsole;
    char map[MapHeight][MapWidth];

    public:
    Map()
        : tcodMap(MapWidth, MapHeight),
          mapConsole(NULL)
    {
        mapConsole = new TCODConsole(MapWidth, MapHeight);

        // Clear everything.
        tcodMap.clear(true, true);
        std::memset(map, 0, sizeof(map));
    }

    virtual ~Map()
    {
        if (mapConsole)
            delete mapConsole;
    }

    virtual void generate()
    {
    }

    virtual void draw(TCODConsole* console) const
    {
        TCODConsole::blit(mapConsole, 0, 0, MapWidth, MapHeight, console, 0, 0, 1.0f, 0.0f);
    }
};

class DungeonMap : public Map
{
    private:
    static const int MinRoomSize = 3;
    static const int MaxRoomSize = 6;

    TCODPath* pathfinder;
    TCODRandom* random;

    public:
    DungeonMap()
        : Map(),
          pathfinder(NULL)
    {
        // No diagonal movements
        //pathfinder = new TCODDijkstra(tcodMap, 0.0f);
        random = TCODRandom::getInstance();
    }

    ~DungeonMap()
    {
        if (pathfinder)
            delete pathfinder;
    }

    virtual void generate()
    {
        const int numRooms = 6;
        for (int i = 0; i < numRooms; ++i)
        {
            IntRect r(random->get(0, MapWidth - MaxRoomSize),
                      random->get(0, MapHeight - MaxRoomSize),
                      random->get(MinRoomSize, MaxRoomSize),
                      random->get(MinRoomSize, MaxRoomSize));

            for (int y = r.y; y < r.y + r.h; ++y)
            {
                for (int x = r.x; x < r.x + r.w; ++x)
                {
                    map[y][x] = '#';
                }
            }
        }
    }

    virtual void draw(TCODConsole* console) const
    {
        for (int y = 0; y < MapHeight; ++y)
        {
            for (int x = 0; x < MapWidth; ++x)
            {
                console->setCharForeground(x, y, TCODColor::white);
                console->setChar(x, y, map[y][x]);
            }
        }
    }
};

class State
{
    protected:

    public:
    State() {}
    virtual ~State() {}

    virtual void handleInput(Input& input, const TCOD_key_t& key) = 0;
    virtual void logic() = 0;
    virtual void draw() = 0;
};

class TestState : public State
{
    private:
    Map* map;
    Player me;

    public:
    TestState()
        : map(NULL)
    {
        map = new DungeonMap;
        map->generate();
    }

    ~TestState()
    {
        if (map)
            delete map;
    }

    void handleInput(Input& input, const TCOD_key_t& key)
    {
        me.handleInput(input, key);
        if (key.c == 'q')
            map->generate();
    }

    void logic()
    {
    }

    void draw()
    {
        me.draw(TCODConsole::root);
        map->draw(TCODConsole::root);
    }
};

class Application
{
    private:
    bool running;

    Input input;
    State* currentState;

    TCODRandom* random;

    public:
    Application()
        : running(false),
          currentState(NULL),
          random(NULL)
    {
        initialize();
        random = TCODRandom::getInstance();
    }

    ~Application()
    {
        if (currentState)
            delete currentState;
    }

    void initialize()
    {
        TCODConsole::setCustomFont("terminal8x8_aa_tc.png", TCOD_FONT_LAYOUT_TCOD);

        TCODConsole::initRoot(46, 40, "Test Game", false, TCOD_RENDERER_GLSL);
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
            TCODConsole::root->clear();
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

