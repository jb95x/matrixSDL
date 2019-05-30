#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>

typedef struct
{
    //Specify how long is the rain trail
    int trailSize;
    //Specify the min number of drops per collumn
    int minDrops;
    //Specify the max number of drops per collumn
    int maxDrops;
    //Collumn minimum speed
    int minSpeed;
    //Column maximum speed
    int maxSpeed;
    //Drop Color
    int dR;
    int dG;
    int dB;
    //Trail Color
    int tR;
    int tG;
    int tB;
    //Font Size
    int fontSize;
    //Frame Time
    int frameTime;
} Options;

Options InitOptions(int trailSize, int minDrops, int maxDrops, int minSpeed, int maxSpeed, int dR, int dG, int dB, int tR, int tG, int tB, int fontSize, int frameTime)
{
    Options options;
    options.trailSize = trailSize;
    options.minDrops = minDrops;
    options.maxDrops = maxDrops;
    options.minSpeed = minSpeed;
    options.maxSpeed = maxSpeed;
    options.dR = dR;
    options.dG = dG;
    options.dB = dB;
    options.tR = tR;
    options.tG = tG;
    options.tB = tB;
    options.fontSize = fontSize;
    options.frameTime = frameTime;
    return options;
}

typedef struct
{
    //Enviroment width
    int width;
    //Enviroment height
    int height;
    //Letter Columns Size
    int cols;
    //Letter Rows Size
    int rows;
    //Total area
    int area;
    //Letter spacing width
    int spacingWidth;
    //Letter spacing height
    int spacingHeight;
    //Alpha step for trail
    int alphaUnit;
} EVar;

EVar InitEvar(int width, int height, int cols, int rows, int area, int spacingWidth, int spacingHeight, int alphaUnit)
{
    EVar evar;
    evar.width = width;
    evar.height = height;
    evar.cols = cols;
    evar.rows = rows;
    evar.area = area;
    evar.spacingWidth = spacingWidth;
    evar.spacingHeight = spacingHeight;
    evar.alphaUnit = alphaUnit;
    return evar;
}

int getCoord(int x, int y, int width)
{
    return y * width + x;
}

int mod(int x, int n)
{
    return (x % n + n) % n;
}

int millisTime(clock_t time)
{
    return double(time) / CLOCKS_PER_SEC * 1000;
}

void initializeMaskMatrix(int *matrix, int mSize)
{
    for (int i = 0; i < mSize; i++)
    {
        matrix[i] = 0;
    }
}

int initializeSpeedVector(int *matrix, int width, int min, int max)
{
    int maxValue = 0;
    for (int i = 0; i < width; i++)
    {
        matrix[i] = min + (rand() % (max - min));
        if (maxValue < matrix[i])
        {
            maxValue = matrix[i];
        }
    }
    return maxValue;
}

void initializeCharMatrix(int *matrix, int mSize)
{
    for (int i = 0; i < mSize; i++)
    {
        matrix[i] = rand() % 26;
    }
}

void markHeads(int *matrix, int width, int height, int min, int max)
{
    for (int i = 0; i < width; i++)
    {
        int n = min + (rand() % (max - min));
        int k = height / n;
        for (int j = 0; j < n; j++)
        {
            int coordY = (n + (j * k)) % height;
            matrix[getCoord(i, coordY, width)] = -1;
        }
    }
}

void markTails(int *matrix, int width, int height, int trailSize)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            if (matrix[getCoord(i, j, width)] == -1)
            {
                for (int w = 0; w < trailSize; w++)
                {
                    int temp = j - w - 1;
                    int coord = getCoord(i, mod(temp, height), width);

                    if (matrix[coord] != -1)
                    {
                        matrix[coord] = trailSize - w - 1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}

void moveRain(int *maskMatrix, int *charMatrix, int *speedVector, int width, int height, int frame)
{
    for (int i = 0; i < width; i++)
    {
        if (frame % speedVector[i] == 0)
        {
            int current = maskMatrix[getCoord(i, 0, width)];
            for (int j = 0; j < height; j++)
            {
                if (maskMatrix[getCoord(i, j, width)] == -1)
                {
                    charMatrix[getCoord(i, j, width)] = rand() % 26;
                }
                int tempCoord = getCoord(i, mod(j + 1, height), width);
                int temp = maskMatrix[tempCoord];
                maskMatrix[tempCoord] = current;
                current = temp;
            }
        }
    }
}

int poolEvents(SDL_Event ev)
{
    bool isRunning = true;
    while (SDL_PollEvent(&ev) != 0)
    {
        if (ev.type == SDL_QUIT)
        {
            isRunning = false;
        }
        else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            isRunning = false;
        }
    }
    return isRunning;
}

void liveRendering(Options settings, EVar eVar, SDL_Renderer *renderer, TTF_Font *font, int *maskMatrix, int *charMatrix, int *speedVector, std::vector<SDL_Texture *> &characterCache, int frame)
{
    SDL_Texture *currentTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, eVar.width, eVar.height);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, currentTexture);
    SDL_RenderFillRect(renderer, NULL);
    for (int i = 0; i < eVar.cols; i++)
    {
        for (int j = 0; j < eVar.rows; j++)
        {
            int x = charMatrix[getCoord(i, j, eVar.cols)];
            int y = maskMatrix[getCoord(i, j, eVar.cols)];
            if (y != 0)
            {
                if (y == -1)
                {
                    y = 0;
                }
                else
                {
                    y = settings.trailSize - y + 1;
                }
                SDL_Rect destiny;
                destiny.h = eVar.spacingHeight;
                destiny.w = eVar.spacingWidth;
                destiny.x = i * destiny.w;
                destiny.y = j * destiny.h;
                int coord = getCoord(x, y, 26);
                SDL_RenderCopy(renderer, characterCache.at(coord), NULL, &destiny);
            }
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, currentTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(currentTexture);
    currentTexture = nullptr,
    moveRain(maskMatrix, charMatrix, speedVector, eVar.cols, eVar.rows, frame);
}

void appLoop(Options settings, EVar eVar, SDL_Renderer *renderer, TTF_Font *font, int *maskMatrix, int *charMatrix, int *speedVector, std::vector<SDL_Texture *> &characterCache)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Event ev;
    int frame = 0;
    int delay = 0;
    clock_t renderTime = clock();
    while (poolEvents(ev))
    {
        renderTime = clock();
        liveRendering(settings, eVar, renderer, font, maskMatrix, charMatrix, speedVector, characterCache, frame);
        frame++;
        renderTime = clock() - renderTime;
        delay = (settings.frameTime - millisTime(renderTime)) > 0 ? (settings.frameTime - millisTime(renderTime)) : 0;
        SDL_Delay(delay);
    }
}

void buildCharacterCache(Options settings, EVar eVar, SDL_Renderer *renderer, TTF_Font *font, std::vector<SDL_Texture *> &characterCache)
{
    SDL_Surface *textSurface = nullptr;
    SDL_Color color;
    for (int j = 0; j < settings.trailSize + 1; j++)
    {
        for (int i = 0; i < 26; i++)
        {
            if (j == 0)
            {
                color = {(uint8_t)settings.dR, (uint8_t)settings.dG, (uint8_t)settings.dB, 255};
            }
            else
            {
                uint8_t hardMath = (uint8_t)(eVar.alphaUnit * (j - 1));
                color = {(uint8_t)(settings.tR * hardMath), (uint8_t)(settings.tG * hardMath), (uint8_t)(settings.tB * hardMath), 255};
            }
            textSurface = TTF_RenderGlyph_Solid(font, static_cast<char>(97 + i), color);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            characterCache.push_back(textTexture);
            SDL_FillRect(textSurface, NULL, 0x000000);
        }
    }
    SDL_FreeSurface(textSurface);
    textSurface = nullptr;
}

void buildAssets(Options settings, EVar eVar, SDL_Renderer *renderer, TTF_Font *font)
{
    int maskMatrix[eVar.area];
    int charMatrix[eVar.area];
    int speedVector[eVar.cols];
    initializeMaskMatrix(maskMatrix, eVar.area);
    initializeCharMatrix(charMatrix, eVar.area);
    int maxSpeed = initializeSpeedVector(speedVector, eVar.cols, settings.minSpeed, settings.maxSpeed);
    markHeads(maskMatrix, eVar.cols, eVar.rows, settings.minDrops, settings.maxDrops);
    markTails(maskMatrix, eVar.cols, eVar.rows, settings.trailSize);
    std::vector<SDL_Texture *> characterCache;
    buildCharacterCache(settings, eVar, renderer, font, characterCache);
    //std::cout << "Character cache size: " << characterCache.size() << std::endl;
    appLoop(settings, eVar, renderer, font, maskMatrix, charMatrix, speedVector, characterCache);
    //std::cout << "Freeing " << characterCache.size() << " entries from character cache" << std::endl;
    for (SDL_Texture *texture : characterCache)
    {
        SDL_DestroyTexture(texture);
    }
}

EVar setupEnviromentVariables(Options settings, int windowWidth, int windowHeight)
{
    int unit = settings.fontSize;
    int cols = ceil((float)windowWidth / (float)unit);
    int rows = ceil((float)windowHeight / (float)unit);
    int area = cols * rows;
    int spacingWidth = unit;
    int spacingHeight = unit;
    int alphaUnit = 255 / settings.trailSize;
    return InitEvar(windowWidth, windowHeight, cols, rows, area, spacingWidth, spacingHeight, alphaUnit);
}

void gatherEnviromentInfo(Options settings, SDL_Renderer *renderer, SDL_Window *window, TTF_Font *font)
{
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    EVar eVar = setupEnviromentVariables(settings, windowWidth, windowHeight);
    buildAssets(settings, eVar, renderer, font);
}

void setupFont(Options settings, SDL_Renderer *renderer, SDL_Window *window)
{
    TTF_Font *font = TTF_OpenFont("matrix-norfok.ttf", settings.fontSize);
    if (font == NULL)
    {
        std::cout << "SDL_ttf OpenFont Error: " << TTF_GetError() << std::endl;
    }
    else
    {
        //TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        gatherEnviromentInfo(settings, renderer, window, font);
    }
    TTF_CloseFont(font);
}

void setupRenderer(Options settings, SDL_Window *window)
{
    SDL_Renderer *renderer = nullptr;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        std::cout << "SDL CreateRenderer Error: " << SDL_GetError() << std::endl;
    }
    else
    {
        setupFont(settings, renderer, window);
    }
    SDL_DestroyRenderer(renderer);
}

void setupWindow(Options settings)
{
    SDL_Window *window = nullptr;
    SDL_DisplayMode dm;
    SDL_GetDesktopDisplayMode(0, &dm);

    window = SDL_CreateWindow(
        "Matrix Rain",                                                   // window title
        SDL_WINDOWPOS_CENTERED,                                          // initial x position
        SDL_WINDOWPOS_CENTERED,                                          // initial y position
        dm.w,                                                            // width, in pixels
        dm.h,                                                            // height, in pixels
        SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED //flags
    );

    if (window == NULL)
    {
        std::cout << "Window creation error: " << SDL_GetError() << std::endl;
    }
    else
    {
        setupRenderer(settings, window);
    }
    SDL_DestroyWindow(window);
    window = nullptr;
}

void initializeTTFEnvironment(Options settings)
{
    if (TTF_Init() < 0)
    {
        std::cout << "SDL_ttf Initialization Error: " << TTF_GetError() << std::endl;
    }
    else
    {
        setupWindow(settings);
    }
    TTF_Quit();
}

void initializeSDLEnvironment(Options settings)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Video Initialization Error: " << SDL_GetError() << std::endl;
    }
    else
    {
        initializeTTFEnvironment(settings);
    }
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int trailSize = 20;
    int minDrops = 3;
    int maxDrops = 5;
    int minSpeed = 1;
    int maxSpeed = 6;
    int dR = 255;
    int dG = 255;
    int dB = 255;
    int tR = 0;
    int tG = 255;
    int tB = 0;
    int fontSize = 12;
    int frameTime = 16;
    Options settings = InitOptions(trailSize, minDrops, maxDrops, minSpeed, maxSpeed, dR, dG, dB, tR, tG, tB, fontSize, frameTime);
    initializeSDLEnvironment(settings);
    //std::cout << "I GOT HERE, GOOD NEWS !" << std::endl;
    return 0;
}