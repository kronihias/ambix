#include "PluginProcessor.h"
#include "../../common/JuceCompat.h"
#include "PluginEditor.h"

int Ambix_encoderAudioProcessor::getNumParameters()
{
    return kTotalParams;
}

float Ambix_encoderAudioProcessor::getParameter (int index)
{
    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int srcIdx = (index - SourceParamsBase) / kPerSourceParams;
        const int sub    = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz:   return source_params[srcIdx].az;
            case SrcEl:   return source_params[srcIdx].el;
            case SrcSize: return source_params[srcIdx].size;
        }
        return 0.f;
    }

    switch (index) {
        case AzimuthParam:           return azimuth_param;
        case ElevationParam:         return elevation_param;
        case SizeParam:              return size_param;
        case WidthParam:             return width_param;
        case AzimuthSetParam:        return azimuth_set_param;
        case AzimuthSetRelParam:     return azimuth_set_rel_param;
        case AzimuthMvParam:         return azimuth_mv_param;
        case ElevationSetParam:      return elevation_set_param;
        case ElevationSetRelParam:   return elevation_set_rel_param;
        case ElevationMvParam:       return elevation_mv_param;
        case SpeedParam:             return speed_param;
        case LinkedParam:            return linked_param;
        case NumActiveSourcesParam:  return num_active_sources_param;
        default:                     return 0.f;
    }
}

void Ambix_encoderAudioProcessor::setParameter (int index, float newValue)
{
    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int srcIdx = (index - SourceParamsBase) / kPerSourceParams;
        const int sub    = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz:   source_params[srcIdx].az   = newValue; break;
            case SrcEl:   source_params[srcIdx].el   = newValue; break;
            case SrcSize: source_params[srcIdx].size = newValue; break;
        }
        sendChangeMessage();
        return;
    }

    float tempvalue = 0.0f;

    switch (index) {
        case AzimuthParam:
            azimuth_param = newValue;
            break;

        case ElevationParam:
            elevation_param = newValue;
            break;

        case SizeParam:
            size_param = newValue;
            break;

        case WidthParam:
            width_param = newValue;
            break;

        case LinkedParam: {
            const bool wasLinked = linked_param >= 0.5f;
            const bool nowLinked = newValue     >= 0.5f;
            linked_param = newValue;
            if (wasLinked && ! nowLinked)
                linkedToUnlinkedSnapshot();
            else if (! wasLinked && nowLinked)
                unlinkedToLinkedSnapshot();
            break;
        }

        case NumActiveSourcesParam:
            num_active_sources_param = newValue;
            break;

        case AzimuthSetParam:
            if ((newValue != azimuth_set_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
                setParameterNotifyingHost(this, AzimuthParam, newValue);
            azimuth_set_param = newValue;
            break;

        case AzimuthSetRelParam:
            if ((newValue != azimuth_set_rel_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
            {
                tempvalue = azimuth_param + (newValue - azimuth_set_rel_param);
                if (tempvalue < 0.0f)      tempvalue = 1.0f + tempvalue;
                else if (tempvalue > 1.0f) tempvalue = tempvalue - 1.0f;
                setParameterNotifyingHost(this, AzimuthParam, tempvalue);
            }
            azimuth_set_rel_param = newValue;
            break;

        case AzimuthMvParam:
            azimuth_mv_param = newValue;
            break;

        case ElevationSetParam:
            if ((newValue != elevation_set_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
                setParameterNotifyingHost(this, ElevationParam, newValue);
            elevation_set_param = newValue;
            break;

        case ElevationSetRelParam:
            if ((newValue != elevation_set_rel_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
            {
                tempvalue = elevation_param + (newValue - elevation_set_rel_param);
                if (tempvalue < 0.0f)      tempvalue = 1.0f + tempvalue;
                else if (tempvalue > 1.0f) tempvalue = tempvalue - 1.0f;
                setParameterNotifyingHost(this, ElevationParam, tempvalue);
            }
            elevation_set_rel_param = newValue;
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

    sendChangeMessage();
}

const String Ambix_encoderAudioProcessor::getParameterName (int index)
{
    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int srcIdx = (index - SourceParamsBase) / kPerSourceParams + 1;
        const int sub    = (index - SourceParamsBase) % kPerSourceParams;
        const char* subNames[] = { "Az", "El", "Size" };
        return String("Src") + String(srcIdx) + subNames[sub];
    }

    switch (index) {
        case AzimuthParam:           return "Azimuth";
        case ElevationParam:         return "Elevation";
        case SizeParam:              return "Size";
        case WidthParam:             return "Source width";
        case AzimuthSetParam:        return "SetAzimuth";
        case AzimuthSetRelParam:     return "SetRelAzimuth";
        case AzimuthMvParam:         return "MoveAzimuth";
        case ElevationSetParam:      return "SetElevation";
        case ElevationSetRelParam:   return "SetRelElevation";
        case ElevationMvParam:       return "MoveElevation";
        case SpeedParam:             return "MoveSpeed";
        case LinkedParam:            return "Linked";
        case NumActiveSourcesParam:  return "ActiveSources";
        default:                     return "";
    }
}

const String Ambix_encoderAudioProcessor::getParameterText (int index)
{
    String text;

    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int srcIdx = (index - SourceParamsBase) / kPerSourceParams;
        const int sub    = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz:   text << String((source_params[srcIdx].az  - 0.5f) * 360).substring(0, 5) << " deg"; break;
            case SrcEl:   text << String((source_params[srcIdx].el  - 0.5f) * 360).substring(0, 5) << " deg"; break;
            case SrcSize: text << String(source_params[srcIdx].size).substring(0, 5);                         break;
        }
        return text;
    }

    switch (index) {
        case AzimuthParam:           text << String((azimuth_param - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case ElevationParam:         text << String((elevation_param - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case SizeParam:              text << String(size_param).substring(0, 5); break;
        case WidthParam:             text << String(width_param * 360).substring(0, 5) << " deg"; break;
        case AzimuthSetParam:        text << String((azimuth_set_param - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case AzimuthSetRelParam:     text << String((azimuth_set_rel_param - 0.5f) * 360).substring(0, 5) << " deg"; break;

        case AzimuthMvParam:
            if (azimuth_mv_param <= 0.48f)
                text << String(powf(speed_param*360.f, (0.45f - azimuth_mv_param)*2.22222f)).substring(0, 5) << " deg/sec";
            else if (azimuth_mv_param >= 0.52f)
                text << String(powf(speed_param*360.f, (azimuth_mv_param - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
            else
                text << "do not rotate";
            break;

        case ElevationSetParam:      text << String((elevation_set_param - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case ElevationSetRelParam:   text << String((elevation_set_rel_param - 0.5f) * 360).substring(0, 5) << " deg"; break;

        case ElevationMvParam:
            if (elevation_mv_param <= 0.48f)
                text << String(powf(speed_param*360.f, (0.45f - elevation_mv_param)*2.22222f)).substring(0, 5) << " deg/sec";
            else if (elevation_mv_param >= 0.52f)
                text << String(powf(speed_param*360.f, (elevation_mv_param - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
            else
                text << "do not rotate";
            break;

        case SpeedParam:             text << String((speed_param) * 360).substring(0, 5) << " deg"; break;
        case LinkedParam:            text << (linked_param >= 0.5f ? "linked" : "unlinked"); break;
        case NumActiveSourcesParam:  text << String (getActiveSources()); break;

        default: break;
    }
    return text;
}

const String Ambix_encoderAudioProcessor::getParameterLabel(int index)
{
    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int sub = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz: case SrcEl: return "degree";
            default:                return "";
        }
    }

    switch (index) {
        case AzimuthParam:
        case ElevationParam:
        case WidthParam:
        case AzimuthSetParam:
        case AzimuthSetRelParam:
        case ElevationSetParam:
        case ElevationSetRelParam:
            return "degree";
        case AzimuthMvParam:
        case ElevationMvParam:
        case SpeedParam:
            return "degree/sec";
        default:
            return "";
    }
}

#if WITH_ADVANCED_CONTROL
void Ambix_encoderAudioProcessor::calcNewParameters(double SampleRate, int BufferLength)
{
    const double factor = (double)BufferLength/SampleRate;
    const float speed_fact2 = (float)factor * 0.002777777f;
    const float deg_sec     = speed_param*360.f;

    // Compute az/el deltas in [0,1] param-space units. The dead-zone around
    // the centre detents (0.48..0.52 az / 0.45..0.55 el) is kept identical
    // to the original linked-mode behaviour.
    float dAz = 0.f;
    bool  moveAz = false;
    if (azimuth_mv_param < 0.48f)
    {
        dAz = -powf (deg_sec, (0.48f - azimuth_mv_param) * 2.0833333f) * speed_fact2;
        moveAz = true;
    }
    else if (azimuth_mv_param > 0.52f)
    {
        dAz =  powf (deg_sec, (azimuth_mv_param - 0.52f) * 2.0833333f) * speed_fact2;
        moveAz = true;
    }

    float dEl = 0.f;
    bool  moveEl = false;
    if (elevation_mv_param <= 0.45f)
    {
        dEl = -powf (deg_sec, (0.45f - elevation_mv_param) * 2.22222f) * speed_fact2;
        moveEl = true;
    }
    else if (elevation_mv_param >= 0.55f)
    {
        dEl =  powf (deg_sec, (elevation_mv_param - 0.55f) * 2.22222f) * speed_fact2;
        moveEl = true;
    }

    auto wrap01 = [] (float v)
    {
        if (v < 0.f) v += 1.f;
        if (v > 1.f) v -= 1.f;
        return v;
    };

    if (isLinked())
    {
        // Original behaviour: drive global az/el so the linked-mode auto-
        // spread rotates as a whole.
        if (moveAz)
            setParameterNotifyingHost (this, AzimuthParam,   wrap01 (azimuth_param   + dAz));
        if (moveEl)
            setParameterNotifyingHost (this, ElevationParam, wrap01 (elevation_param + dEl));
    }
    else
    {
        // Unlinked: apply the same delta to every active source so the
        // group translates as a constellation, preserving the relative
        // positions the user set up.
        const int active = getActiveSources();
        for (int i = 0; i < active; ++i)
        {
            if (moveAz)
                setParameterNotifyingHost (this, sourceParamIndex (i, SrcAz),
                                           wrap01 (source_params[i].az + dAz));
            if (moveEl)
                setParameterNotifyingHost (this, sourceParamIndex (i, SrcEl),
                                           wrap01 (source_params[i].el + dEl));
        }
    }
}
#endif
