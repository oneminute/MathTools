#ifndef COMMON_H
#define COMMON_H

enum ToolType
{
    TT_EigenMatrix = 0,
    TT_CovMatrix = 1,
    TT_PCA = 2,
    TT_Probability
};

enum DistributionType
{
    DT_BERNOULLI = 0,
    DT_MULTINOULLI,
    DT_NORMAL
};

#endif // COMMON_H