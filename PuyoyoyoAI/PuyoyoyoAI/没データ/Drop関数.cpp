#include "deque"
std::deque<int> queue;

static void drop(int place)
{
    queue.clear();
    if (place < 12)
    {
        int num = place;
        if (num >= 6) num -= 6;
        switch (num)
        {
        case 0:
        {
            queue.push_back(0);
            if (place == num + 6)
            {
                queue.push_back(3);
                queue.push_back(3);
            }
            break;
        }
        case 1:
        {
            queue.push_back(4);
            if (place == num + 6)
            {
                queue.push_back(3);
                queue.push_back(3);
            }
            break;
        }
        case 2:
        {
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        case 3:
        {
            queue.push_back(5);
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        case 4:
        {
            queue.push_back(1);
            queue.push_back(3);
            if (place == num + 6)
            {
                queue.push_back(3);
            }
            else
            {
                queue.push_back(2);
            }
            break;
        }
        case 5:
        {
            queue.push_back(1);
            if (place == num + 6)
            {
                queue.push_back(2);
                queue.push_back(2);
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        int num = place - 12;
        switch (num)
        {
        case 0:
        {
            queue.push_back(0);
            queue.push_back(2);
            break;
        }
        case 1:
        {
            queue.push_back(2);
            break;
        }
        case 2:
        {
            queue.push_back(5);
            queue.push_back(2);
            break;
        }
        case 3:
        {
            queue.push_back(1);
            queue.push_back(3);
            queue.push_back(2);
            queue.push_back(2);
            break;
        }
        case 4:
        {
            queue.push_back(1);
            queue.push_back(2);
            break;
        }
        case 5:
        {
            queue.push_back(0);
            queue.push_back(3);
            break;
        }
        case 6:
        {
            queue.push_back(4);
            queue.push_back(3);
            break;
        }
        case 7:
        {
            queue.push_back(3);
            break;
        }
        case 8:
        {
            queue.push_back(5);
            queue.push_back(3);
            break;
        }
        case 9:
        {
            queue.push_back(1);
            queue.push_back(3);
            break;
        }
        default:
            break;
        }
    }
}

static void Operations()
{
    static int puyoMode = 0;
    static int frame = 0;
    if (puyoMode)
    {
        int action = queue.front(); // 前方要素を取得
        queue.erase(queue.begin());

        switch (action)
        {
        case 0:
            PPT.InputKey(VK_LEFT, true, true);
            frame = 1;
            break;
        case 1:
            PPT.InputKey(VK_RIGHT, true, true);
            frame = 1;
            break;
        case 2:
            PPT.InputKey('Z', false, true);
            break;
        case 3:
            PPT.InputKey('X', false, true);
            break;
        case 4:
            PPT.InputKey(VK_LEFT, false, true);
            break;
        case 5:
            PPT.InputKey(VK_RIGHT, false, true);
            break;
        default:
            break;
        }
        puyoMode = 0;
    }
    else
    {
        if (frame)
        {
            PPT.InputKey(VK_LEFT, true, false);
            PPT.InputKey(VK_RIGHT, true, false);
            frame--;
        }
        if (!queue.empty()) puyoMode = 1;
        PPT.InputKey('Z', false, false);
        PPT.InputKey('X', false, false);
        PPT.InputKey(VK_LEFT, false, false);
        PPT.InputKey(VK_RIGHT, false, false);
        /*if (frame > 0)
        {
            /*if ((frame == 4 || frame == 2) && !queue.empty() && queue.front() != 6) PPT.InputKey(queue.front() * -2 + 94, false, true);
            if ((frame == 3 || frame == 1) && !queue.empty() && queue.front() != 6) PPT.InputKey(queue.front() * -2 + 94, false, false);
            if ((frame == 3 || frame == 1) && !queue.empty()) queue.erase(queue.begin());
            if (frame == 1)
            {
                PPT.InputKey(VK_LEFT, true, false);
                PPT.InputKey(VK_RIGHT, true, false);
            }
            frame--;
        }
        else
        {
            if (!queue.empty()) puyoMode = 1;
            PPT.InputKey('Z', false, false);
            PPT.InputKey('X', false, false);
            PPT.InputKey(VK_LEFT, false, false);
            PPT.InputKey(VK_RIGHT, false, false);
        }*/
    }
}