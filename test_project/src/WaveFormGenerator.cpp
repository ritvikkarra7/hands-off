#include <math.h>
#include "WaveFormGenerator.h"


const float chromatic[] = {261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88};
const float major[]     = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88};
const float minor[]     = {261.63, 293.66, 311.13, 349.23, 392.00, 415.30, 466.16};
const float pentatonic[]= {261.63, 293.66, 329.63, 392.00, 440.00};

WaveFormGenerator::WaveFormGenerator(int sample_rate, int frequency, float magnitude)
{
    setScale(SCALE_DEFAULT);  // Ensures safe initialization
    m_sample_rate = sample_rate;
    m_frequency = frequency;
    m_magnitude = magnitude;
    m_current_position = 0;
}

void WaveFormGenerator::getFrames(Frame_t *frames, int number_frames)
{   
    if (m_frequency == 0)
        // If the frequency is 0, we should not generate any samples.
        // Set the frames to zero and return false.
        // This will stop the I2S output from sending any data.
        // This is important to avoid sending garbage data to the DAC.
    {
        for (int i = 0; i < number_frames; i++)
        {
            frames[i].left = frames[i].right = 0;
        }
    }

    else {

        float full_wave_samples = m_sample_rate / m_frequency;
        float step_per_sample = M_TWOPI / full_wave_samples;
    
        for (int i = 0; i < number_frames; i++)
        {
            float phase = fmod(m_current_position, M_TWOPI);
            float value = 0.0f;
    
            switch (m_wave_type)
            {
                case SINE:
                    value = sin(phase);
                    break;
    
                case SQUARE:
                    value = (phase < M_PI) ? 1.0f : -1.0f;
                    break;
    
                case TRIANGLE:
                    {
                        float normalized_phase = phase / M_TWOPI;
                        value = 4.0f * fabs(normalized_phase - 0.5f) - 1.0f;
                    }
                    break;
                case SAWTOOTH:

                    {
                        float normalized_phase = phase / M_TWOPI;
                        value = 2.0f * normalized_phase - 1.0f;
                    }
                    break;
                    
                default:
                    value = 0.0f; // Default case, should not happen
                    break;
            }
    
            frames[i].left = frames[i].right = 16384 * m_magnitude * value;
    
            m_current_position += step_per_sample;
            if (m_current_position > M_TWOPI)
            {
                m_current_position -= M_TWOPI;
            }
        }

    }

}

void WaveFormGenerator::setWaveType(WaveType type)
{
    m_wave_type = type; 
}

void WaveFormGenerator::setFrequency(int frequency)
{
    m_frequency = frequency;
}

void WaveFormGenerator::setMagnitude(float magnitude)
{
    m_magnitude = magnitude;
}

void WaveFormGenerator::setScale(Scale scale) {
    Serial.printf("[WaveFormGenerator] Setting scale to %d\n", scale);

    currentScale = scale;
    switch (scale) {
        case SCALE_CHROMATIC:
            scaleFrequencies = chromatic;
            scaleLength = sizeof(chromatic) / sizeof(float);
            break;
        case SCALE_MAJOR:
            scaleFrequencies = major;
            scaleLength = sizeof(major) / sizeof(float);
            break;
        case SCALE_MINOR:
            scaleFrequencies = minor;
            scaleLength = sizeof(minor) / sizeof(float);
            break;
        case SCALE_PENTATONIC:
            scaleFrequencies = pentatonic;
            scaleLength = sizeof(pentatonic) / sizeof(float);
            break;
        case SCALE_DEFAULT:
        default:
            scaleFrequencies = nullptr;
            scaleLength = 0;
            break;
    }

    currentNoteIndex = -1;
    Serial.printf("[WaveFormGenerator] scaleLength = %d\n", scaleLength);
}

float WaveFormGenerator::getNoteFromDuration(float duration) {
    if (!scaleFrequencies || scaleLength <= 0) {
        // Continuous (non-quantized) mapping
        Serial.printf("%.2f ms duration, returning continuous frequency\n", duration);
        return 4509.7 * exp(-0.001075 * duration);
    }

    // Exponential mapping: map log(duration) to note index
    const float minDuration = 100.0f;
    const float maxDuration = 20000.0f;
    float norm = (log(duration) - log(minDuration)) / (log(maxDuration) - log(minDuration));
    norm = constrain(norm, 0.0f, 1.0f);

    int targetIndex = round((1.0f - norm) * (scaleLength - 1)); // closer = higher freq

    // Hysteresis
    const int hysteresis = 1; // index difference allowed before change
    if (currentNoteIndex == -1 || abs(targetIndex - currentNoteIndex) > hysteresis) {
        currentNoteIndex = targetIndex;
    }

    return scaleFrequencies[currentNoteIndex];
}

