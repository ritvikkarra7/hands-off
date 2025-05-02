#ifndef __wave_form_generator_h__
#define __wave_form_generator_h__

#include "SampleSource.h"

enum WaveType {
    SINE, 
    SQUARE, 
    TRIANGLE
}; 

class WaveFormGenerator : public SampleSource
{
private:
    int m_sample_rate;
    int m_frequency;
    float m_magnitude;
    float m_current_position;
    WaveType m_wave_type = SINE; 

public:
    WaveFormGenerator(int sample_rate, int frequency, float magnitude);
    virtual int sampleRate() { return m_sample_rate; }
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample. Each sample should be signed 16 bits
    virtual void getFrames(Frame_t *frames, int number_frames);
    // Setter
    void setFrequency(int frequency);
    void setMagnitude(float magnitude);

    void setWaveType(WaveType type);

};



#endif