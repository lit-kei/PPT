
static int JudgementColor(int x, int y) {
    //       |    RGB      | color
    //empty  |             |   0
    //red    | (c2,7f,7d)  |   1
    //yerrow | (dc,77,1d)  |   2
    //green  | (34,bd,2e)  |   3
    //blue   | (48,68,d1)  |   4
    //purple | (b0,7c,d3)  |   5
    //ojama  | (7f,67,6b)  |   6

    BYTE r, g, b;
    PPT.GetPixelColor(x, y, r, g, b);

    auto [h, s, v] = rgbToHsv(r, g, b);

    static int color = 0;
    static int difference = 0;

    if (r >= g)
    {
        if (b >= r)
        {
            difference = b - g;
        }
        else if (g >= b)
        {
            difference = r - b;
        }
        else
        {
            difference = r - g;
        }
    }
    else
    {
        if (b >= g)
        {
            difference = b - r;
        }
        else if (r >= b)
        {
            difference = g - b;
        }
        else
        {
            difference = g - r;
        }
    }

    if (difference <= 0x19 && r >= 0x70)
    {
        color = 6;
        return color;
    }

    if (r >= 0x90)
    {
        if (b >= 0x90)
        {
            color = 5;
        }
        else if (b <= 0x3f)
        {
            color = 2;
        }
        else
        {
            color = 1;
        }
    }
    else if (b >= 0x90)
    {
        color = 4;
    }
    else if (g >= 0x90)
    {
        color = 3;
    }
    else
    {
        color = 0;
    }
    return color;
}