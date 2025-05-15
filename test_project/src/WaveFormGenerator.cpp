#include <math.h>
#include "WaveFormGenerator.h"

WaveFormGenerator::WaveFormGenerator(int sample_rate, int frequency, float magnitude)
{
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

