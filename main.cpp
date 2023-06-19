#include "daisy_seed.h"
#include "daisysp.h"
#include "vox.h"
#include "flt.h"

using namespace daisy;
using namespace daisysp;

#define NUM_ADC_CHANNELS (3)
enum AdcChannel {
    P_RIGHT = 0,
    P_CENTRE,
    P_LEFT
};

typedef enum _SwitchState {
    S_RIGHT,
    S_CENTRE,
    S_LEFT,
} SwitchState_t;

DaisySeed hw;

const int kVoxCount = 15;
constexpr float kVoxVolumeKof = 3.0 / kVoxCount;
Vox voxs[kVoxCount];

Filter flt;

SwitchState_t tri_switch = S_LEFT;

float hardClip(float in)
{
    in = in > 1.f ? 1.f : in;
    in = in < -1.f ? -1.f : in;
    return in;
}

float softClip(float in)
{
    if(in > 0)
        return 1 - expf(-in);
    return -1 + expf(in);
}

/******************
 * Audio Callback
 ******************/
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float output = 0;
        if(S_LEFT != tri_switch)
        {
            for(Vox &v : voxs)
            {
                output += v.Process();
            }
            output = flt.Process(output) * kVoxVolumeKof;

            // DISTORTION
            if (S_RIGHT == tri_switch) {
                const float pregain = 0.1 * 10 + 1.2;
                const float gain = 0.1 * 100 + 1.2;
                const float drywet = 0.25;
                float input = output * pregain;

                float wet = softClip(input * gain);
                output = wet * drywet + input * (1 - drywet);
            }
        }
        out[0][i] = output;
        out[1][i] = output;
    }
}


/* Debug Print Statement */
inline void debug(float pitch, float freq, float timbre) {
    hw.PrintLine("pitch [" FLT_FMT3 "], freq [" FLT_FMT3
                 "], timbre [" FLT_FMT3 "], switch [%d]",
                 FLT_VAR3(pitch),
                 FLT_VAR3(freq),
                 FLT_VAR3(timbre),
                 tri_switch);
}

inline SwitchState_t read_tri_switch(bool left, bool right) {
    if (left && !right) {
        return S_LEFT;
    }
    if (right && !left) {
        return S_RIGHT;
    }
    return S_CENTRE;
}

int main(void)
{
    /* Setup */
    hw.Init();
    hw.StartLog();

    /* Setup Pins */
    AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
    adc_config[P_RIGHT].InitSingle(seed::A0);
    adc_config[P_CENTRE].InitSingle(seed::A1);
    adc_config[P_LEFT].InitSingle(seed::A2);
    hw.adc.Init(&adc_config[0], NUM_ADC_CHANNELS);

    GPIO switch_left;
    GPIO switch_right;
    switch_left.Init(seed::D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
    switch_right.Init(seed::D28, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

    /* Setup Audio */
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    float sampleRate = hw.AudioSampleRate();

    // Vox setup
    float voxFreq = Vox::kOscLowestFreq;
    for (Vox &v: voxs) {
        v.Init(sampleRate, voxFreq);
        // Pythagorean tuning 2:1 (octave), 3:2 (perfect fifth)
        voxFreq *= 1.5;
        // if the note is too high, divide the frequency by two
        // (to stop it getting too high)
        if(voxFreq > 1500)
        {
            voxFreq *= 0.5;
        }
    }

    flt.Init(sampleRate);

    hw.adc.Start();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        tri_switch = read_tri_switch(switch_left.Read(), switch_right.Read());
        float left = hw.adc.GetFloat(P_LEFT);
        float centre  = hw.adc.GetFloat(P_CENTRE);
        float right = hw.adc.GetFloat(P_RIGHT);

        hw.SetLed(S_RIGHT == tri_switch);

        for(Vox &vox : voxs)
        {
            vox.Read(right, centre);
        }
        flt.SetTimbre(left);

        daisy::System::Delay(4);
    }
}
