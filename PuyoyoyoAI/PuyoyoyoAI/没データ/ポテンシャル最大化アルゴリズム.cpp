
static void Potential()
{
    int best[4] = { -100000, -100000, -100000, -100000 };
    int nowBest[3] = { 0 };
    int score = 0;
    int count = 0;
    for (int i = 0; i < 22; i++)
    {
        score = 0;
        count = 0;
        SetTmp2();
        GetStarted(copyField, copyFloor, nowPuyo[0], nowPuyo[1], i);
        if (!detectChain())
        {
            for (int j = 0; j < 22; j++)
            {
                score = 0;
                count = 0;
                SetTmp1();
                GetStarted(copyField, copyFloor, nextPuyo[0], nextPuyo[1], j);
                if (!ConnectCheck(copyField))
                {
                    for (int k = 0; k < 22; k++)
                    {
                        score = 0;
                        count = 0;
                        SetTmp();
                        GetStarted(copyField, copyFloor, nextNextPuyo[0], nextNextPuyo[1], k);
                        score += FieldEvaluation();
                        if (ConnectCheck(copyField))
                        {
                            while (true)
                            {
                                int save = ChainStart(copyField, count);
                                if (save == 0) break;
                                else score += save;
                                count++;
                                fall(copyField);
                            }
                        }

                        for (int l = 0; l < 6; l++)
                        {
                            copyField[l] = tmpField[l];
                            copyFloor[l] = tmpFloor[l];
                        }
                        if (score > best[0])
                        {
                            best[0] = score;
                            best[1] = i;
                            best[2] = j;
                            best[3] = k;
                        }
                        //puyoNum = tmp[2];
                    }
                }
                else
                {
                    score += FieldEvaluation();
                    while (true)
                    {
                        int save = ChainStart(copyField, count);
                        if (save == 0) break;
                        else score += save;
                        count++;
                        fall(copyField);
                    }
                    if (score > best[0])
                    {
                        best[0] = score;
                        best[1] = i;
                        best[2] = j;
                        best[3] = -1;
                    }
                }
                for (int l = 0; l < 6; l++)
                {
                    copyField[l] = tmpField1[l];
                    copyFloor[l] = tmpFloor1[l];
                }
                //puyoNum = tmp[1];
            }
        }
        else
        {
            while (true)
            {
                int save = ChainStart(copyField, count);
                if (save == 0) break;
                else score += save;
                count++;
                fall(copyField);
            }
            for (int l = 0; l < 6; l++)
            {
                copyField[l] = tmpField[l];
                copyFloor[l] = tmpFloor[l];
            }
            if (score >= nowBest[0])
            {
                nowBest[0] = score;
                nowBest[1] = i;
                nowBest[2] = count;
            }
            if (score > best[0])
            {
                best[0] = score;
                best[1] = i;
                best[2] = -1;
                best[3] = -1;
            }
        }
        for (int l = 0; l < 6; l++)
        {
            copyField[l] = tmpField2[l];
            copyFloor[l] = tmpFloor2[l];
        }
        //puyoNum = tmp[0];
    }
    //if (nowBest[0] >= 3000 && puyoNum >= 36)
    if (nowBest[2] >= 10)
    {
        drop(nowBest[1]);
        dropQueue.clear();
        dropQueue.push_back(nowBest[0]);
        dropQueue.push_back(nowBest[2]);
        return;
    }
    drop(best[1]);
    dropQueue.clear();
    dropQueue.push_back(best[2]);
    dropQueue.push_back(best[3]);

    WCHAR text[] = L"%d";
    DrawString(hMemDC, 0, 340, text, best[0]);
}