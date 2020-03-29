#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sys/ioctl.h> // for sound
#include <vector>
#include <thread>
#include <atomic>
#include <string>

std::atomic<int> swaps = 0, comparisons = 0, currentPos;
std::string currentSort;

struct WindowInfo
{
    SDL_Window* win;
    SDL_Renderer* ren;
    int h, w;
};

template<typename T> void swap(T& a, T& b) { T c = a; a = b; b = c; }
void draw(WindowInfo& winInfo, const std::vector<int> nums);
void drawText(WindowInfo& winInfo, std::string text, int x, int y, int size = 12);

enum class Method { RANDOM, EQUAL };
void sizeVector(std::vector<int>& nums, int size, int maxVal, Method method = Method::EQUAL);

// non-recursive
void shuffle(std::vector<int>& nums);
void bubbleSort(std::vector<int>& nums);
void swapSort(std::vector<int>& nums);
void combSort(std::vector<int>& nums);
void selectionSort(std::vector<int>& nums);
void insertionSort(std::vector<int>& nums);
// recursive
void stoogeSort(std::vector<int>& nums, int start, int end);
void quickSort(std::vector<int>& nums, int start, int end);
void slowSort(std::vector<int>& nums, int start, int end);

// special xD
void flashSort(std::vector<int>& nums);

bool isSorted(std::vector<int> nums) { for (int n = 1, m = 0; n < nums.size(); n++, m++) { if (nums.at(n) < nums.at(m)) return false; } return true; }

// this is responsible for continous shuffling and sorting
void sortingPipeline(std::vector<int>& nums, int size, int maxVal, int msDelay);

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
    std::thread thr1(&sortingPipeline, std::ref(nums), winInfo.w / 2, winInfo.h, 5000);
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

        drawText(winInfo,
            currentSort +
            " - Swaps: " + std::to_string(swaps.load()) +
            " - Comparisons: " + std::to_string(comparisons.load()), 10, 5, 9);

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
    TTF_Quit();

    return 0;
}

void draw(WindowInfo& winInfo, std::vector<int> nums)
{
    SDL_Rect Bar;
    Bar.w = winInfo.w / nums.size();
    
    // draw the bar
    SDL_SetRenderDrawColor(winInfo.ren, 200, 100, 100, 255);
    Bar.x = currentPos * Bar.w;
    Bar.y = 0;
    Bar.h = winInfo.h;
    SDL_RenderFillRect(winInfo.ren, &Bar);

    for (int itr = 0; const auto& num : nums)
    {
        // draw the bar
        int color = itr % 2 == 0 ? 50 : 100;
        SDL_SetRenderDrawColor(winInfo.ren, color, color, color, color * 2);
        Bar.x = itr * Bar.w;
        Bar.y = winInfo.h - num;
        Bar.h = num;
        SDL_RenderFillRect(winInfo.ren, &Bar);

        // draw a quare at the top of the bar
        SDL_SetRenderDrawColor(winInfo.ren, 255, 255, 255, 255);
        Bar.y = winInfo.h - num;
        Bar.h = Bar.w;
        SDL_RenderFillRect(winInfo.ren, &Bar);

        itr++;
    }
}

void drawText(WindowInfo& winInfo, std::string text, int x, int y, int size)
{
    TTF_Font* font = TTF_OpenFont("OpenSans-Regular.ttf", 50);
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), SDL_Color({ 255, 255, 255 }));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(winInfo.ren, surf);
    SDL_Rect rect = { x, y, size * (int)text.size(), size * 3 };
    SDL_RenderCopy(winInfo.ren, tex, 0, &rect);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
    TTF_CloseFont(font);
}

void sizeVector(std::vector<int>& nums, int size, int maxVal, Method method)
{
    nums.clear();

    // float step = size / maxVal;

    switch(method)
    {
    case Method::RANDOM:
        for (int i = 0; i < size; i++)
        {
            nums.push_back(rand() % maxVal);
        }
        break;

    case Method::EQUAL:
        float step = maxVal / (float)size;
        for (int i = 0; i < size; i++)
        {
            nums.push_back(i * step);
        }
        break;
    }
}

void shuffle(std::vector<int>& nums)
{
    currentSort = "Shuffle";
    for (int y = 0; y < nums.size(); y++)
    {
        for (int x = 0; x < nums.size(); x++)
        {
            swap(nums[rand() % nums.size()], nums[rand() % nums.size()]);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void bubbleSort(std::vector<int>& nums)
{
    for (int y = 0; y < nums.size(); y++)
    {
        for (int x = 0; x < nums.size() - y - 1; x++)
        {
            comparisons++;
            if (nums.at(x) > nums.at(x + 1))
            {
                swap(nums[x], nums[x + 1]);
                swaps++;
                currentPos = x;
            }

            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }

        if (isSorted(nums))
        {
            return;
        }
    }
}

void swapSort(std::vector<int>& nums)
{
    int tinierCount;

    while (!isSorted(nums))
    {
        for (int n = 0; n < nums.size(); n++)
        {
            tinierCount = 0;
            for (int m = 0; m < nums.size(); m++)
            {
                comparisons++;
                if (nums[m] < nums[n])
                {
                    tinierCount++;
                }
            }

            if (nums[n] != nums[tinierCount])
            {
                swap(nums[n], nums[tinierCount]);
            }
            
            currentPos = n;
            swaps++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void combSort(std::vector<int>& nums)
{
    int step =  nums.size();
    bool swapped;
    do
    {
        swapped = false;
        for (int i = 0; i < nums.size() - step; i++)
        {
            comparisons++;
            if (nums.at(i) > nums.at(i + step))
            {
                swap(nums[i], nums[i + step]);
                swapped = true;

                swaps++;
                currentPos = i;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        if (step > 1)
        {
            step = (int)(step / 1.3f);
        }
    } while (swapped == true || step > 1);
}

void selectionSort(std::vector<int>& nums)
{
    for (int y = 0; y < nums.size() - 1; y++)
    {
        int tiniest = INT16_MAX;
        int pos;
        for (int x = y; x < nums.size(); x++)
        {
            comparisons++;
            if (nums.at(x) < tiniest)
            {
                tiniest = nums.at(x);
                pos = x;
                currentPos = x;
            }

            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }

        swap(nums[y], nums[pos]);
        swaps++;
        currentPos = y;
        
        if (isSorted(nums))
        {
            return;
        }
    }
}

void insertionSort(std::vector<int>& nums)
{
    for (int y = 1; y < nums.size(); y++)
    {
        int x = y;
        while (nums[x - 1] > nums[x] && x > 0)
        {
            comparisons++;
            swap(nums[x - 1], nums[x]);
            swaps++;
            currentPos = x;
            x--;
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }
    }
}

void shakerSort(std::vector<int>& nums)
{
    int start = -1, end = nums.size();
    bool swapped = true;
    while (swapped)
    {
        swapped = false;
        start++;

        for (int n = start; n < end; n++)
        {
            comparisons++;
            if (nums[n] > nums[n + 1])
            {
                swap(nums[n], nums[n + 1]);
                swaps++;
                swapped = true;

                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                currentPos = n;
            }
        }

        if (!swapped)
        {
            break;
        }

        end--;

        for (int n = end; n >= start; n--)
        {
            comparisons++;
            if (nums[n] > nums[n + 1])
            {
                swap(nums[n], nums[n + 1]);
                swaps++;
                swapped = true;

                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }
    }
}

void stoogeSort(std::vector<int>& nums, int start, int end)
{
    comparisons++;
    if (nums[end - 1] < nums[start])
    {
        swap(nums[end - 1], nums[start]);
        swaps++;
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        currentPos = end - 1;
    }

    if ((end - start) > 2)
    {
        int third = (end - start) / 3;
        stoogeSort(nums, start, end - third);
        stoogeSort(nums, start + third, end);
        stoogeSort(nums, start, end - third);
    }
}

void quickSort(std::vector<int>& nums, int start, int end)
{
    int pivot = (start + end) / 2;
    if (end - start > 1)
    {
        for (int j = start; j < end; j++)
        {
            for (int n = start; n < pivot; n++)
            {
                comparisons++;
                if (nums[n] > nums[pivot])
                {
                    swap(nums[n], nums[pivot]);
                    swaps++;
                    pivot = n;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    currentPos = n;
                }
            }

            for (int n = end - 1; n > pivot; n--)
            {
                comparisons++;
                if (nums[n] < nums[pivot])
                {
                    swap(nums[n], nums[pivot]);
                    swaps++;
                    pivot = n;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    currentPos = n;
                }
            }
        }

        quickSort(nums, start, pivot);
        quickSort(nums, pivot, end);
    }
}

void slowSort(std::vector<int>& nums, int start, int end)
{
    if (start >= end) return;
    int mid = floor((start + end) / 2.0f);
    slowSort(nums, start, mid);
    slowSort(nums, mid + 1, end);

    comparisons++;
    if (nums.at(end) < nums.at(mid))
    {
        swap(nums[end], nums[mid]);
        swaps++;
        currentPos = mid;
    }

    slowSort(nums, start, end - 1);
}

void flashSort(std::vector<int>& nums)
{
    // find the min and max
    int minimun = INT16_MAX, maximum = 0;
    for (int n = 0; const auto& num : nums)
    {
        comparisons++;
        if (num < minimun)
        {
            minimun = num;
            currentPos = n;
        }
        comparisons++;
        if (num > maximum)
        {
            maximum = num;
            currentPos = n;
        }
        n++;
    }

    for (int n = 0; n < nums.size(); n++)
    {
        int pos = 1 + ((nums.size() - 2) * (float)((nums.at(n) - minimun) / (float)(maximum - minimun)));
        swaps++;
        swap(nums[n], nums[pos]);
        currentPos = n;
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    insertionSort(nums);
}

void sortingPipeline(std::vector<int>& nums, int size, int maxVal, int msDelay)
{
    while (true)
    {
        sizeVector(nums, size, maxVal, Method::RANDOM);
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "BubbleSort";
        bubbleSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));
        
        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "SwapSort";
        swapSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "CombSort";
        combSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "SelectionSort";
        selectionSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));
        
        currentSort = "InsertionSort";
        insertionSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "ShakerSort";
        shakerSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "StoogeSort";
        stoogeSort(nums, 0, nums.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "QuickSort";
        quickSort(nums, 0, nums.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "FlashSort";
        flashSort(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        sizeVector(nums, size / 5, maxVal);
        swaps = 0; comparisons = 0;
        shuffle(nums);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));

        currentSort = "SlooooooowSort";
        slowSort(nums, 0, nums.size() - 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

        swaps = 0; comparisons = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(msDelay / 2));
    }
}