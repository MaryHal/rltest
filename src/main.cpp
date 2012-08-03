#include <cstdio>
#include <libtcod/libtcod.hpp>

#include <vector>
#include <map>
#include <cstring>

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

    void normalize()
    {
        if (x > x + w)
        {
            x = x + w;
            w = -w;
        }

        if (y > y + h)
        {
            y = y + h;
            h = -h;
        }
    }

    bool collide(const Point& point)
    {
        if (point.x > x && point.x < x + w)
            return true;
        if (point.y > y && point.y < y + h)
            return true;
        return false;
    }

    bool collide(const Rect<T>& rect)
    {
        if (y > rect.y + rect.h)
            return false;
        if (y + h < rect.y)
            return false;
        if (x > rect.x + rect.w)
            return false;
        if (x + w < rect.x)
            return false;

        return true;
    }
};

typedef Rect<int> IntRect;

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
    static const int MapHeight = 30;
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
        clear();
    }

    virtual ~Map()
    {
        if (mapConsole)
            delete mapConsole;
    }

    virtual void clear()
    {
        tcodMap.clear(false, true);
        std::memset(map, 0, sizeof(map));
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
    static const int MinRoomSize = 4;
    static const int MaxRoomSize = 12;

    TCODPath* pathfinder;
    TCODRandom* random;

    public:
    DungeonMap()
        : Map(),
          pathfinder(NULL)
    {
        // No diagonal movements
        pathfinder = new TCODPath(&tcodMap, 0.0f);
        random = TCODRandom::getInstance();
    }

    ~DungeonMap()
    {
        if (pathfinder)
            delete pathfinder;
    }

    bool roomCollision(std::vector<IntRect>& roomList, IntRect& currentRoom)
    {
        // Check for collisions with all previous rooms
        for (std::vector<IntRect>::iterator iter = roomList.begin();
             iter != roomList.end();
             ++iter)
        {
            if (currentRoom.collide(*iter))
            {
                return true;
            }
        }

        return false;
    }

    void snapToEdge(const IntRect& room, Point& p)
    {
        switch (random->get(0, 3))
        {
        case 0: // Left
            p.x = room.x;
            break;
        case 1: // Right
            p.x = room.x + room.w;
            break;
        case 2: // Up
            p.y = room.y;
            break;
        case 3:
            p.y = room.y + room.h;
            break;
        default:
            break;
        }
    };

    void copyRoom(const IntRect& r)
    {
        for (int y = r.y; y <= r.y + r.h; ++y)
        {
            for (int x = r.x; x <= r.x + r.w; ++x)
            {
                tcodMap.setProperties(x, y, true, false);
                map[y][x] = ' ';
            }
        }

        map[r.y][r.x]             = '+';
        map[r.y][r.x + r.w]       = '+';
        map[r.y + r.h][r.x]       = '+';
        map[r.y + r.h][r.x + r.w] = '+';

        for (int i = r.x + 1; i < r.x + r.w; ++i)
        {
            map[r.y][i]       = '-';
            map[r.y + r.h][i] = '-';
        }

        for (int i = r.y + 1; i < r.y + r.h; ++i)
        {
            map[i][r.x]       = '|';
            map[i][r.x + r.w] = '|';
        }
    }

    virtual void generate()
    {
        printf("Generatin\'\n");
        const int numRooms = 4;
        std::vector<IntRect> roomList;

        for (int i = 0; i < numRooms; ++i)
        {
            IntRect r(random->get(1, MapWidth  - MaxRoomSize - 2),
                      random->get(1, MapHeight - MaxRoomSize - 2),
                      random->get(MinRoomSize - 1, MaxRoomSize - 1),
                      random->get(MinRoomSize - 1, MaxRoomSize - 1));

            // Check for collisions with all previous rooms
            if (roomCollision(roomList, r))
            {
                --i;
                continue;
            }

            copyRoom(r);

            // Everything checks out.
            roomList.push_back(r);

            if (roomList.size() > 1)
            {
                // We want the second-to-last item. I feel like this is ugly.
                IntRect prevRoom = roomList.at(roomList.size() - 2);

                Point p1(random->get(r.x + 1, r.x + r.w - 1),
                         random->get(r.y + 1, r.y + r.h - 1));

                Point p2(random->get(prevRoom.x + 1, prevRoom.x + prevRoom.w - 1),
                         random->get(prevRoom.y + 1, prevRoom.y + prevRoom.h - 1));

                snapToEdge(r, p1);
                snapToEdge(prevRoom, p2);

                tcodMap.setProperties(p1.x, p1.y, true, true);
                tcodMap.setProperties(p2.x, p2.y, true, true);

                pathfinder->compute(p1.x, p1.y,
                                    p2.x, p2.y);

                map[p1.y][p1.x] = 'X';
                map[p2.y][p2.x] = 'X';

                // Walk the path in reverse connecting the rooms
                while (!pathfinder->isEmpty())
                {
                    int x;
                    int y;
                    if (pathfinder->walk(&x, &y, false))
                    {
                        printf("Walkin\' %d %d\n", x, y);
                        tcodMap.setProperties(x, y, true, true);
                        map[y][x] = '#';
                    }
                    else
                        printf("Whoops.");
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
        Map::draw(console);
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
        if (key.pressed && key.c == 'q')
        {
            map->clear();
            map->generate();
        }
    }

    void logic()
    {
    }

    void draw()
    {
        map->draw(TCODConsole::root);
        me.draw(TCODConsole::root);
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

