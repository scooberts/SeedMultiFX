#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace daisy::seed;

DaisySeed hw;
Chorus chorus;
Tremolo trem;
Oscillator osc;
Encoder encoder;
DelayLine<float, 48000> delay;
float delay_out, feedback, signal_out;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		// Delay Configuration
		delay_out = delay.Read();
		signal_out = in[0][i] + delay_out;
		feedback = (delay_out * 0.5) + (in[0][i]);
		delay.Write(feedback);

		out[0][i] = signal_out;

		// Chorus
		out[0][i] = chorus.Process(out[0][i]);

	}
}

int main(void)
{
	hw.Init();
	hw.StartLog();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	float sample_rate = hw.AudioSampleRate();
	float delay_multiplier = 0.375;

	chorus.Init(sample_rate);

	trem.Init(sample_rate);

	osc.Init(sample_rate);
	osc.SetFreq(1000); 

	delay.Init();
	delay.SetDelay(sample_rate * delay_multiplier);

	GPIO pin1;
	GPIO pin2;
	GPIO pin3;
	pin1.Init(D0, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
	pin2.Init(D1, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
	pin3.Init(D2, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);

	hw.StartAudio(AudioCallback);

	encoder.Init(D0, D2, D1);
	int32_t value = 0;
	while(1) {
		value = value + encoder.Increment();
		hw.PrintLine("%d", value);
	}
}
