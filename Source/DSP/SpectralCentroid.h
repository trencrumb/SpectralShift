//
// Created by Kyle Ramsey on 12/16/2025.
// Spectral Centroid Analyzer using FFT
//

#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

/**
 * Real-time spectral centroid analyzer using overlapping FFT windows.
 *
 * Calculates the frequency-domain "center of mass" of an audio signal,
 * providing a measure of spectral brightness. Higher values indicate
 * brighter/more high-frequency content.
 *
 * Implementation details:
 * - FFT size: 2048 samples (2^11)
 * - Window: Hann window
 * - Overlap: 75% (hop size = 512 samples)
 * - Temporal smoothing: ~250ms time constant
 * - Frequency range: 20 Hz to 20 kHz (clamped)
 *
 * The centroid is only updated when sufficient energy is present in the
 * signal to avoid noise artifacts during silence.
 */
class SpectralCentroid
{
public:
    SpectralCentroid() = default;

    void prepare(double sampleRate, int maxBlockSize)
    {
        this->sampleRate = sampleRate;

        // Initialize FFT buffers
        fftBuffer.resize(fftSize * 2, 0.0f);  // Real + imaginary
        inputBuffer.resize(fftSize, 0.0f);
        magnitudes.resize(fftSize / 2 + 1, 0.0f);  // Only need positive frequencies

        // Pre-calculate bin frequencies (SIMD optimization)
        const int numBins = fftSize / 2 + 1;
        binFrequencies.resize(numBins - 1);  // Skip DC bin
        const float binWidthHz = static_cast<float>(sampleRate / fftSize);
        for (int bin = 1; bin < numBins; ++bin)
        {
            binFrequencies[bin - 1] = bin * binWidthHz;
        }

        // Reset counters
        writePosition = 0;
        samplesUntilNextFFT = hopSize;

        // Update happens every hopSize samples
        const float timeConstantMs = 800.0f;
        const float timeConstantSeconds = timeConstantMs / 1000.0f;
        const float updateRateHz = static_cast<float>(sampleRate) / hopSize;
        smoothingCoeff = std::exp(-1.0f / (timeConstantSeconds * updateRateHz));

        // Initialize centroid values
        rawCentroidHz = 1000.0f;
        smoothedCentroidHz = 1000.0f;
    }

    void reset()
    {
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(inputBuffer.begin(), inputBuffer.end(), 0.0f);
        std::fill(magnitudes.begin(), magnitudes.end(), 0.0f);
        writePosition = 0;
        samplesUntilNextFFT = hopSize;
        rawCentroidHz = 1000.0f;
        smoothedCentroidHz = 1000.0f;
    }

    void processBlock(const float* monoBuffer, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            // Write sample into circular buffer
            inputBuffer[writePosition] = monoBuffer[i];
            writePosition = (writePosition + 1) % fftSize;

            // Decrement counter
            samplesUntilNextFFT--;

            // Perform FFT when we've accumulated enough samples
            if (samplesUntilNextFFT <= 0)
            {
                performFFTAndCalculate();
                samplesUntilNextFFT = hopSize;  // Reset for next hop
            }
        }
    }

    float getCentroidHz() const
    {
        return smoothedCentroidHz;
    }

    float getRawCentroidHz() const
    {
        return rawCentroidHz;
    }

private:
    static constexpr int fftOrder = 11;        // 2^11 = 2048
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 4; // 512 samples (75% overlap)
    static constexpr float energyThreshold = 1e-6f; // Minimum energy to update centroid

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window {
        fftSize,
        juce::dsp::WindowingFunction<float>::hann,
        false  // Don't normalize (we'll handle magnitude scaling)
    };

    std::vector<float> fftBuffer;
    std::vector<float> inputBuffer;
    std::vector<float> magnitudes;
    std::vector<float> binFrequencies;  // Pre-calculated frequency for each bin

    int writePosition = 0;
    int samplesUntilNextFFT = hopSize;

    double sampleRate = 44100.0;
    float rawCentroidHz = 1000.0f;
    float smoothedCentroidHz = 1000.0f;
    float smoothingCoeff = 0.0f;

    void performFFTAndCalculate()
    {
        // Copy from circular buffer to FFT buffer in correct order
        // Start from writePosition (oldest sample) and wrap around
        for (int i = 0; i < fftSize; ++i)
        {
            int readPos = (writePosition + i) % fftSize;
            fftBuffer[i] = inputBuffer[readPos];
        }

        // Apply window function
        window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);

        // Perform FFT (real-to-complex)
        fft.performRealOnlyForwardTransform(fftBuffer.data(), true);

        // Calculate magnitudes from complex FFT output (SIMD optimized)
        calculateMagnitudesSIMD();

        // Calculate centroid from magnitudes
        rawCentroidHz = calculateCentroidFromMagnitudes();

        // Apply temporal smoothing
        smoothedCentroidHz = smoothingCoeff * smoothedCentroidHz +
                             (1.0f - smoothingCoeff) * rawCentroidHz;
    }

    void calculateMagnitudes()
    {
        const int numBins = fftSize / 2 + 1;

        for (int bin = 1; bin < numBins - 1; ++bin)
        {
            float real = fftBuffer[bin];
            float imag = fftBuffer[fftSize - bin];
            magnitudes[bin] = std::sqrt(real * real + imag * imag);
        }
    }


    void calculateMagnitudesSIMD()
    {
        const int numBins = fftSize / 2 + 1;

        // DC bin (bin 0) - always scalar
        magnitudes[0] = std::abs(fftBuffer[0]);

        // SIMD-optimized magnitude calculation for bins 1 to N-1
        // Process 4 bins at a time using JUCE's FloatVectorOperations
        const int simdBins = numBins - 1;  // Exclude DC and Nyquist

        // Prepare temporary buffers for SIMD processing
        std::vector<float> realParts(simdBins);
        std::vector<float> imagParts(simdBins);

        // Extract real and imaginary parts
        for (int bin = 1; bin < numBins - 1; ++bin)
        {
            realParts[bin - 1] = fftBuffer[bin];
            imagParts[bin - 1] = fftBuffer[fftSize - bin];
        }

        // Square the real parts (in-place)
        juce::FloatVectorOperations::multiply(realParts.data(), realParts.data(), simdBins);

        // Square the imaginary parts (in-place)
        juce::FloatVectorOperations::multiply(imagParts.data(), imagParts.data(), simdBins);

        // Add squared components: real^2 + imag^2
        juce::FloatVectorOperations::add(&magnitudes[1], realParts.data(), imagParts.data(), simdBins);

        // Take square root to get magnitudes
        for (int bin = 1; bin < numBins - 1; ++bin)
        {
            magnitudes[bin] = std::sqrt(magnitudes[bin]);
        }

        // Nyquist bin - always scalar
        if constexpr (numBins > 1)
        {
            magnitudes[numBins - 1] = std::abs(fftBuffer[fftSize / 2]);
        }
    }

    float calculateCentroidFromMagnitudes()
    {
        const int numBins = fftSize / 2 + 1;

        // SIMD-optimized weighted sum calculation
        // Use pre-calculated frequency array (set in prepare())
        std::vector<float> weightedMagnitudes(numBins - 1);

        // Multiply pre-calculated frequencies by magnitudes (SIMD)
        juce::FloatVectorOperations::multiply(weightedMagnitudes.data(),
                                               binFrequencies.data(),
                                               &magnitudes[1],
                                               numBins - 1);

        // Sum the weighted magnitudes (compiler will auto-vectorize this loop)
        float weightedSum = 0.0f;
        for (int i = 0; i < numBins - 1; ++i)
            weightedSum += weightedMagnitudes[static_cast<size_t>(i)];

        // Sum the magnitudes (compiler will auto-vectorize this loop)
        float magnitudeSum = 0.0f;
        for (int i = 1; i < numBins; ++i)
            magnitudeSum += magnitudes[static_cast<size_t>(i)];

        // Check if we have enough energy to calculate centroid
        if (magnitudeSum < energyThreshold)
        {
            // Not enough energy, maintain last valid centroid
            return smoothedCentroidHz;
        }

        // Calculate centroid
        float centroid = weightedSum / (magnitudeSum + 1e-12f);  // Add epsilon for safety

        // Clamp to valid range
        centroid = juce::jlimit(20.0f, 20000.0f, centroid);

        return centroid;
    }
};
