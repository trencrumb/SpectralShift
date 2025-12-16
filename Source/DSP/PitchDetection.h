// Source/DSP/PitchDetection.h
#pragma once
#include <cmath>

inline float estimatePitchHzAutocorr(const float* x, int n, float sampleRate,
float minHz = 50.0f, float maxHz = 500.0f)
{
    const int minLag = (int) (sampleRate / maxHz);
    const int maxLag = (int) (sampleRate / minHz);

    if (n <= maxLag + 1) return 0.0f;

    float bestCorr = 0.0f;
    int bestLag = 0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float corr = 0.0f;
        float energy0 = 0.0f;
        float energy1 = 0.0f;

        for (int i = 0; i < n - lag; ++i)
        {
            const float a = x[i];
            const float b = x[i + lag];
            corr += a * b;
            energy0 += a * a;
            energy1 += b * b;
        }

        const float denom = std::sqrt(energy0 * energy1) + 1.0e-12f;
        const float normCorr = corr / denom;

        if (normCorr > bestCorr)
        {
            bestCorr = normCorr;
            bestLag = lag;
        }
    }

    // Simple confidence gate
    if (bestCorr < 0.6f || bestLag == 0) return 0.0f;

    return sampleRate / (float) bestLag;
}

