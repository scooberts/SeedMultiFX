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
CpuLoadMeter loadMeter;
float delayRead, feedback, delayOutput, interSignal;
int fxToggle;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	loadMeter.OnBlockStart();
	for (size_t i = 0; i < size; i++)
	{
		interSignal = in[0][i];
		
		//
		// Effects
		//

		// Delay
		if((fxToggle & 0b1) == 0b1) {
			delayRead = delay.Read();

			// Calculate and write feedback
			feedback = (delayRead * 0.5) + (in[0][i]);
			delay.Write(feedback);

			// Output
			interSignal = interSignal + delayRead;
		}

		// Chorus
		if(((fxToggle >> 1) & 0b1) == 0b1) {
			interSignal = 3 * chorus.Process(interSignal);
		}

		// Output
		out[0][i] = interSignal;

	}
	loadMeter.OnBlockEnd();
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

	loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());

	/////////////////
	//Encoder Stuff//
	encoder.Init(D0, D2, D1);
	int32_t encoderVal = 0;
	/////////////////

	hw.StartAudio(AudioCallback);

	while(1) {
		
			switch (encoderVal) {
		case 1:
			fxToggle = 0b0001; break;
		case 2:
			fxToggle = 0b0010; break;
		case 3:
			fxToggle = 0b0011; break;
		default:
			fxToggle = 0b0000; break;
		}

		const float avgLoad = loadMeter.GetAvgCpuLoad();
		const float maxLoad = loadMeter.GetMaxCpuLoad();
		const float minLoad = loadMeter.GetMinCpuLoad();

		encoder.Debounce();
		if(encoder.Increment() == 1 && encoderVal < 3) {
			encoderVal = encoderVal + 1;
		} else if(encoder.Increment() == -1 && encoderVal > 0){
			encoderVal = encoderVal - 1;
		}

		hw.PrintLine("%u", fxToggle);

		/* hw.PrintLine("Processing Load %:");
        hw.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
        hw.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
        hw.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f)); */
	}
}
