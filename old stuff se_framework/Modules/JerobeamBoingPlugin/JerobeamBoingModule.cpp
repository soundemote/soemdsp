
	parDensity.init(i++, SLIDER, "Density", 0, 100, 0),
	parSharpness.init(i++, SLIDER, "Sharpness", -1, 1, 0, jura::Parameter::LINEAR_BIPOLAR),
	parRotX.init(i++, SLIDER, "RotX", -180, 180, 0, jura::Parameter::LINEAR_BIPOLAR),
	parRotY.init(i++, SLIDER, "RotY", -180, 180, 0, jura::Parameter::LINEAR_BIPOLAR),
	parZDepth.init(i++, SLIDER, "ZDepth", 0, 1, 0),
	parZAmount.init(i++, SLIDER, "ZAmount", 0, 1, 0),
	parEnds.init(i++, SLIDER, "Ends", -10, 10, 0, jura::Parameter::LINEAR_BIPOLAR),
	parJump.init(i++, SLIDER, "Jump", 0, 1, 0),
	parDirection.init(i++, SLIDER, "Direction", 0, 1, 0),
	parBoing.init(i++, SLIDER, "Boing", 0, 1, 0),
	parShape.init(i++, SLIDER, "Shape", 0, 1, 0),
	parVolume.init(i++, SLIDER, "Volume", 0, 2, 1),
	parVolumePreJump.init(i++, BUTTON, "VolumePreJump", 0, 1, 0),

	parDensity.setCallback([this](double v) { jbBoingCores.jbBoing.setDensity(v); });
	parSharpness.setCallback([this](double v) { jbBoingCores.jbBoing.setSharpness(v); });
	parRotX.setCallback([this](double v) { jbBoingCores.jbBoing.setRotX(v); });
	parRotY.setCallback([this](double v) { jbBoingCores.jbBoing.setRotY(v); });
	parZDepth.setCallback([this](double v) { jbBoingCores.jbBoing.setZDepth(v); });
	parZAmount.setCallback([this](double v) { jbBoingCores.jbBoing.setZAmount(v); });
	parEnds.setCallback([this](double v) { jbBoingCores.jbBoing.setEnds(v); });
	parBoing.setCallback([this](double v) { jbBoingCores.jbBoing.setBoing(v); });
	parJump.setCallback([this](double v) { jbBoingCores.jbBoing.setBoingStrength(v); });
	parDirection.setCallback([this](double v) { jbBoingCores.jbBoing.setDir(v); });
	parShape.setCallback([this](double v) { jbBoingCores.jbBoing.setShape(v); });
	parVolume.setCallback([this](double v) { jbBoingCores.jbBoing.setVolume(v); });
	parVolumePreJump.setCallback([this](double v) { jbBoingCores.jbBoing.setVolumePreJump(v); });