array<double> inputParameters(4);
array<string> inputParametersNames   = {"Frequency", "Discrete Gain", "Smoothed Gain", "Noise Gain"};
array<double> inputParametersMin     = {  0, 0, 0, 0};
array<double> inputParametersMax     = { 10, 1,  1,  1};
array<double> inputParametersDefault = {  1, 0,  0,  0};
array<string> inputParametersUnits   = {"Hz"};
array<string> inputParametersFormats = {".3", ".2", ".2", ".2"};

string name = "Smoothed Sample & Hold";
string description = "Genreates smoothed and/or discrete values using a precise timer.";

SmoothedSampleAndHold SnH;

void initialize()
{
	SnH.setSampleRate(sampleRate);
}

int getTailSize()
{
	return -1;
}

void updateInputParameters()
{
	SnH.setFrequency(inputParameters[0]);
}

void processBlock(BlockData& data)
{

}

void processSample(array<double>& ioSample)
{
	SnH.increment();

    double discrete = SnH.getSampledValue() * inputParameters[1];
    double smoothed = SnH.getSmoothedValue() * inputParameters[2];
    double noise    = SnH.getNoiseValue() * inputParameters[3];

	for (uint ch = 0; ch < audioInputsCount; ++ch)
		ioSample[ch] = discrete + smoothed + noise;
}
