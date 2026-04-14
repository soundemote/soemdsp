string name="Phase Noise";
string description=name;

int getTailSize() { return -1; }
void processBlock(BlockData& data)
{
    for(uint i=0;i<data.samplesToProcess;i++)
    {
        timeA = timeA + 5;
        data.samples[0][i]=processSample();
        data.samples[1][i]=processSample();
    }
}

int timeA = 100000;
const double amt = 10000;
double processSample()
{

    // the idea that noise can be in phase if you always stick to the ratio from the starting point.. then when it happends to be in place it should have proper phases for that
    // another possibility is frequency sweeps, but always in phase..


    double dbl = amt * rand() + 1.0;
    return sin((timeA+0.0) * dbl) / dbl;

    //return cos((timeA+0.0) * amt * rand());
    //return (cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()))*0.25;
    // return (
    //       cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) 
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand()) + cos((timeA+0.0) * amt * rand())
    //     ) / 32.0 ;
        // return (
        //   cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) 
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand()) + cos((timeA+0.0) + amt * rand())
        // ) / 32.0 ;

 // return (
 //          cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        + cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))+cos((timeA+0.0) * (1.0 + amt * rand()))
 //        ) / 32.0 ;

}


