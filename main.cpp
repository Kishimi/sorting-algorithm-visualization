#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sys/ioctl.h> // for sound
#include <vector>
#include <thread>
#include <atomic>

std::atomic<int> swaps = 0;

struct WindowInfo
{
    SDL_Window* win;
    SDL_Renderer* ren;
    int h, w;
};

template<typename T> void swap(T& a, T& b) { T c = a; a = b; b = c; }
void draw(WindowInfo& winInfo, const std::vector<int> nums);
void drawText(WindowInfo& winInfo, std::string text, int x, int y, int size = 12);

void shuffle(std::vector<int>& nums);
void bubbleSort(std::vector<int>& nums);
void selectionSort(std::vector<int>& nums);

bool isSorted(std::vector<int> nums) { for (int n = 1, m = 0; n < nums.size(); n++, m++) { if (nums.at(n) < nums.at(m)) return false; } return true; }

// this is responsible for continous shuffling and sorting
void sortingPipeline(std::vector<int>& nums);

int main()
{
    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    WindowInfo winInfo;
    winInfo.w = 1280;
    winInfo.h = 720;
    winInfo.win = SDL_CreateWindow("Sorting Algorithms Visualized", 100, 100, winInfo.w, winInfo.h, SDL_WINDOW_SHOWN);
    winInfo.ren = SDL_CreateRenderer(winInfo.win, -1, 0);

    std::vector<int> nums;
    for (int i = 0; i < winInfo.w; i++)
    {
        nums.push_back(rand() % winInfo.h);
    }

    std::thread thr1(&sortingPipeline, std::ref(nums));
    std::thread::native_handle_type thr1Handle = thr1.native_handle(); // needed to force quit the thread

    bool running = true;
    while (running)
    {
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT)
        {
            running = false;
        }

        // clear the screen
        SDL_SetRenderDrawColor(winInfo.ren, 0, 0, 0, 255);
        SDL_RenderClear(winInfo.ren);

        drawText(winInfo, "Swaps: " + std::to_string(swaps.load()), 10, 10);

        draw(winInfo, nums);

        SDL_RenderPresent(winInfo.ren);
        SDL_Delay(1);
    }

    // detach the thread
    thr1.detach();

    // close the thread
    pthread_cancel(thr1Handle);

    SDL_DestroyRenderer(winInfo.ren);
    SDL_DestroyWindow(winInfo.win);
    SDL_Quit();

    return 0;
}

void draw(WindowInfo& winInfo, std::vector<int> nums)
{
    SDL_Rect Bar;
    Bar.w = winInfo.w / nums.size();
    SDL_SetRenderDrawColor(winInfo.ren, 255, 255, 255, 255);

    for (int itr = 0; const auto& num : nums)
    {
        Bar.x = itr * Bar.w;
        Bar.y = winInfo.h - num;
        Bar.h = num;
        SDL_RenderFillRect(winInfo.ren, &Bar);
        itr++;
    }
}

void drawText(WindowInfo& winInfo, std::string text, int x, int y, int size)
{
    TTF_Font* font = TTF_OpenFont("OpenSans-Regular.ttf", 50);
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), SDL_Color({ 255, 255, 255 }));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(winInfo.ren, surf);
    SDL_Rect rect = { x, y, size * 12, size * 2 };
    SDL_RenderCopy(winInfo.ren, tex, 0, &rect);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    TTF_CloseFont(font);
}

void shuffle(std::vector<int>& nums)
{
    for (int y = 0; y < nums.size() / 10; y++)
    {
        for (int x = 0; x < nums.size() / 10; x++)
        {
            swap(nums[rand() % nums.size()], nums[rand() % nums.size()]);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void bubbleSort(std::vector<int>& nums)
{
    for (int y = 0; y < nums.size(); y++)
    {
        for (int x = 0; x < nums.size() - 1; x++)
        {
            if (nums.at(x) > nums.at(x + 1))
            {
                swap(nums[x], nums[x + 1]);
                swaps++;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (isSorted(nums))
        {
            return;
        }
    }
}

void selectionSort(std::vector<int>& nums)
{
    int nextElement = 0;
    for (int y = 0; y < nums.size(); y++)
    {
        int tiniest = INT8_MAX;
        int pos;
        for (int x = nextElement; x < nums.size(); x++)
        {
            if (nums.at(x) < tiniest)
            {
                tiniest = nums.at(x);
                pos = x;
            }
        }

        if (pos != nextElement)
        {
            swap(nums[nextElement], nums[pos]);
        }
        nextElement++;
        if (nextElement >= nums.size())
        {
            nextElement = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        if (isSorted(nums))
        {
            return;
        }
    }
}

void sortingPipeline(std::vector<int>& nums)
{
    while (true)
    {
        bubbleSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        swaps = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        selectionSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        swaps = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}