#include "daisy_seed.h"
#include "daisysp.h"
#include "vox.h"
#include "flt.h"

using namespace daisy;
using namespace daisysp;

#define NUM_ADC_CHANNELS (3)
enum AdcChannel {
    PITCH = 0,
    FREQ,
    TIMBRE
};

DaisySeed hw;

const int kVoxCount = 15;
constexpr float kVoxVolumeKof = 3.0 / kVoxCount;
Vox voxs[kVoxCount];

Filter flt;

bool gate = true;

/******************
 * Audio Callback
 ******************/
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float output = 0;
        if(gate)
        {
            for(Vox &v : voxs)
            {
                output += v.Process();
            }
            output = flt.Process(output) * kVoxVolumeKof;
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
                 gate);
}


int main(void)
{
    /* Setup */
    hw.Init();
    hw.StartLog();

    /* Setup Pins */
    AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
    adc_config[PITCH].InitSingle(seed::A0);
    adc_config[FREQ].InitSingle(seed::A1);
    adc_config[TIMBRE].InitSingle(seed::A2);
    hw.adc.Init(&adc_config[0], NUM_ADC_CHANNELS);

    GPIO my_button;
    my_button.Init(seed::D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

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
        float pitch = hw.adc.GetFloat(PITCH);
        float freq  = hw.adc.GetFloat(FREQ);
        float timbre = hw.adc.GetFloat(TIMBRE);
        gate = my_button.Read();

        hw.SetLed(gate);

        for(Vox &vox : voxs)
        {
            vox.Read(pitch, freq);
        }
        flt.SetTimbre(timbre);

        daisy::System::Delay(4);
    }
}
