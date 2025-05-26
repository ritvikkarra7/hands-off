#ifndef __wave_form_generator_h__
#define __wave_form_generator_h__

#include "SampleSource.h"

enum WaveType {
    SINE, 
    SQUARE, 
    TRIANGLE,
    SAWTOOTH
}; 

enum Scale {
    SCALE_DEFAULT, 
    SCALE_CHROMATIC, 
    SCALE_MAJOR, 
    SCALE_MINOR,  
    SCALE_PENTATONIC,
};


class WaveFormGenerator : public SampleSource
{
private:
    int m_sample_rate;
    int m_frequency;
    float m_magnitude;
    float m_current_position;
    WaveType m_wave_type = SINE; 
    Scale currentScale;
    const float* scaleFrequencies;
    int scaleLength;
    int currentNoteIndex = -1;

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
    void setScale(Scale scale);
    float getNoteFromDuration(float duration);

};



#endif