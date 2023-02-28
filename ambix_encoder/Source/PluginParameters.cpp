#include "PluginProcessor.h"
#include "PluginEditor.h"

int Ambix_encoderAudioProcessor::getNumParameters()
{

    return NumParameters;
}

float Ambix_encoderAudioProcessor::getParameter (int index)
{

    switch (index) {
        case AzimuthParam:
            return azimuth_param;
            break;

        case ElevationParam:
            return elevation_param;
            break;

        case SizeParam:
            return size_param;
            break;

#if INPUT_CHANNELS > 1
        case WidthParam:
            return width_param;
            break;
#endif
            // advanced ctl
        case AzimuthSetParam:
            return azimuth_set_param;
            break;

        case AzimuthSetRelParam:
            return azimuth_set_rel_param;
            break;

        case AzimuthMvParam:
            return azimuth_mv_param;
            break;


        case ElevationSetParam:
            return elevation_set_param;
            break;

        case ElevationSetRelParam:
            return elevation_set_rel_param;
            break;

        case ElevationMvParam:
            return elevation_mv_param;
            break;

        case SpeedParam:
            return speed_param;
            break;

        default:
            return 0.f;
    }

}

void Ambix_encoderAudioProcessor::setParameter (int index, float newValue)
{

    float tempvalue = 0.0f;

    switch (index) {
        case AzimuthParam:
            azimuth_param=newValue;
            calcAzimuth();

            break;

        case ElevationParam:
            elevation_param=newValue;
            for (int i=0; i < AmbiEnc.size(); i++) {
                AmbiEnc.getUnchecked(i)->elevation=elevation_param;
            }
            break;

        case SizeParam:
            size_param = newValue;
            for (int i=0; i < AmbiEnc.size(); i++) {
                AmbiEnc.getUnchecked(i)->size=size_param;
            }
            break;

#if INPUT_CHANNELS > 1
        case WidthParam:
            width_param=newValue;
            calcAzimuth();
            break;
#endif
            // advanced ctl

        case AzimuthSetParam: // set Azimuth if NOT moving!
            if ((newValue != azimuth_set_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
                setParameterNotifyingHost(AzimuthParam, newValue);
            azimuth_set_param=newValue;
            break;

        case AzimuthSetRelParam: // relative movement!
            if ((newValue != azimuth_set_rel_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
            {
                tempvalue = azimuth_param+(newValue-azimuth_set_rel_param);
                if (tempvalue < 0.0f) { // wrap it around!
                    tempvalue = 1.0f + tempvalue;
                } else if (tempvalue > 1.0f){
                    tempvalue = tempvalue - 1.0f;
                }
                setParameterNotifyingHost(AzimuthParam, tempvalue);
            }
            azimuth_set_rel_param=newValue;
            break;

        case AzimuthMvParam:
            azimuth_mv_param = newValue;
            break;


        case ElevationSetParam:
            if ((newValue != elevation_set_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
                setParameterNotifyingHost(ElevationParam, newValue);
            elevation_set_param=newValue;
            break;

        case ElevationSetRelParam: // relative movement!
            if ((newValue != elevation_set_rel_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
            {
                tempvalue = elevation_param + (newValue-elevation_set_rel_param);
                if (tempvalue < 0.0f) { // wrap it around!
                    tempvalue = 1.0f + tempvalue;
                } else if (tempvalue > 1.0f){
                    tempvalue = tempvalue - 1.0f;
                }
                setParameterNotifyingHost(ElevationParam, tempvalue);
            }
            elevation_set_rel_param=newValue;
            break;

        case ElevationMvParam:
            elevation_mv_param = newValue;
            break;

        case SpeedParam:
            speed_param = newValue;
            break;

        default:
            break;

    }

    sendChangeMessage(); // send message to gui!
}

// calculate the individual azimuth for all sources
void Ambix_encoderAudioProcessor::calcAzimuth()
{
    if (AmbiEnc.size() == 1) {
        AmbiEnc.getFirst()->azimuth = azimuth_param;
    }
    else if (AmbiEnc.size() > 1)
    {
        for (int i=0; i < AmbiEnc.size(); i++) {

            float angle = azimuth_param - width_param / 2.f + i * width_param / (AmbiEnc.size()-1);

            // angle = fmodf(angle, 1.f);
            if (angle < 0.f)
                angle = 1.f + angle;

            if (angle > 1.f) {
                angle -= 1.f;
            }

            //std::cout << "source # " << i << " angle: " << (angle-0.5f)*360 << std::endl;

            AmbiEnc.getUnchecked(i)->azimuth = angle;
        }

    }

}

const String Ambix_encoderAudioProcessor::getParameterName (int index)
{
    switch (index) {
        case AzimuthParam:
            return "Azimuth";
            break;

        case ElevationParam:
            return "Elevation";
            break;

        case SizeParam:
            return "Size";
            break;

#if INPUT_CHANNELS > 1
        case WidthParam:
            return "Source width";
            break;
#endif

            // advanced ctl
        case AzimuthSetParam:
            return "SetAzimuth";
            break;

        case AzimuthSetRelParam:
            return "SetRelAzimuth";
            break;

        case AzimuthMvParam:
            return "MoveAzimuth";
            break;


        case ElevationSetParam:
            return "SetElevation";
            break;

        case ElevationSetRelParam:
            return "SetRelElevation";
            break;

        case ElevationMvParam:
            return "MoveElevation";
            break;

        case SpeedParam:
            return "MoveSpeed";
            break;

        default:
            return "";
    }

}

const String Ambix_encoderAudioProcessor::getParameterText (int index)
{
    String text;

    switch (index) {
        case AzimuthParam:
            text << String((azimuth_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case ElevationParam:
            text << String((elevation_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case SizeParam:
            text << String(size_param).substring(0, 5);
            break;

#if INPUT_CHANNELS > 1
        case WidthParam:
            text << String(width_param * 360).substring(0, 5) << " deg";
            break;
#endif
            // advanced ctl
        case AzimuthSetParam:
            text << String((azimuth_set_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case AzimuthSetRelParam:
            text << String((azimuth_set_rel_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case AzimuthMvParam:
			if (azimuth_mv_param <= 0.48f)
			{
                text << String(powf(speed_param*360.f, (0.45f - azimuth_mv_param)*2.22222f)).substring(0, 5) << " deg/sec"; // from 0->90deg/sec
			} else if (azimuth_mv_param >= 0.52f) {
                text << String(powf(speed_param*360.f, (azimuth_mv_param - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
			} else {
				text << "do not rotate";
			}
			break;


        case ElevationSetParam:
            text << String((elevation_set_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case ElevationSetRelParam:
            text << String((elevation_set_rel_param - 0.5f) * 360).substring(0, 5) << " deg";
            break;

        case ElevationMvParam:
			if (elevation_mv_param <= 0.48f)
			{
                text << String(powf(speed_param*360.f, (0.45f - elevation_mv_param)*2.22222f)).substring(0, 5) << " deg/sec"; // from 0->90deg/sec
			} else if (elevation_mv_param >= 0.52f) {
                text << String(powf(speed_param*360.f, (elevation_mv_param - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
			} else {
				text << "do not rotate";
			}
			break;

        case SpeedParam:
            text << String((speed_param) * 360).substring(0, 5) << " deg";
            break;

        default:
            break;
    }

	return text;
}

const String Ambix_encoderAudioProcessor::getParameterLabel(int index)
{
    String text;

    switch (index) {
        case AzimuthParam:
            text << "degree";
            break;

        case ElevationParam:
            text << "degree";
            break;

        case SizeParam:
            text << "";
            break;

#if INPUT_CHANNELS > 1
        case WidthParam:
            text << "degree";
            break;
#endif

            // advanced ctl
        case AzimuthSetParam:
            text << "degree";
            break;

        case AzimuthSetRelParam:
            text << "degree";
            break;

        case AzimuthMvParam:
            text << "degree/sec";
            break;


        case ElevationSetParam:
            text << "degree";
            break;

        case ElevationSetRelParam:
            text << "degree";
            break;

        case ElevationMvParam:
            text << "degree/sec";
            break;

        case SpeedParam:
            text << "degree/sec";
            break;

        default:
            break;
    }

	return text;
}

#if WITH_ADVANCED_CONTROL
void Ambix_encoderAudioProcessor::calcNewParameters(double SampleRate, int BufferLength)
{
    // calculate moving parameters

    double factor = (double)BufferLength/SampleRate;
    float speed_fact2 = (float)factor * 0.002777777f; // (1/360)

    float deg_sec = speed_param*360.f;

    float newval = 0.f;

    if ((azimuth_mv_param < 0.48f) || (azimuth_mv_param > 0.52f))
	{
        if (azimuth_mv_param < 0.48f)
            newval = azimuth_param - powf(deg_sec, (0.48f - azimuth_mv_param)*2.0833333f) * speed_fact2;

        if (azimuth_mv_param > 0.52f)
            newval = azimuth_param + powf(deg_sec, (azimuth_mv_param - 0.52f)*2.0833333f) * speed_fact2;

		if (newval < 0.f)
			newval = 1.f;
		if (newval > 1.f)
			newval = 0.f;
		setParameterNotifyingHost(AzimuthParam, newval);
	}

    if ((elevation_mv_param <= 0.45f) || (elevation_mv_param >= 0.55f))
	{
        if (elevation_mv_param <= 0.45f)
            newval = elevation_param - powf(deg_sec, (0.45f - elevation_mv_param)*2.22222f) * speed_fact2;

        if (elevation_mv_param >= 0.55f)
            newval = elevation_param + powf(deg_sec, (elevation_mv_param - 0.55f)*2.22222f) * speed_fact2;

		if (newval < 0.f)
			newval = 1.f;
		if (newval > 1.f)
			newval = 0.f;
		setParameterNotifyingHost(ElevationParam, newval);
	}

}
#endif