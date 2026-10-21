#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::String parameterName (int index)
    {
        using P = Ambix_encoderAudioProcessor;

        if (index >= P::SourceParamsBase && index < P::kTotalParams)
        {
            const int srcIdx = (index - P::SourceParamsBase) / P::kPerSourceParams + 1;
            const int sub    = (index - P::SourceParamsBase) % P::kPerSourceParams;
            const char* subNames[] = { "Az", "El", "Size", "Mute", "Solo" };
            return String("Src") + String(srcIdx) + subNames[sub];
        }

        switch (index) {
            case P::AzimuthParam:           return "Azimuth";
            case P::ElevationParam:         return "Elevation";
            case P::SizeParam:              return "Size";
            case P::WidthParam:             return "Source width";
            case P::AzimuthSetParam:        return "SetAzimuth";
            case P::AzimuthSetRelParam:     return "SetRelAzimuth";
            case P::AzimuthMvParam:         return "MoveAzimuth";
            case P::ElevationSetParam:      return "SetElevation";
            case P::ElevationSetRelParam:   return "SetRelElevation";
            case P::ElevationMvParam:       return "MoveElevation";
            case P::SpeedParam:             return "MoveSpeed";
            case P::LinkedParam:            return "Linked";
            case P::NumActiveSourcesParam:  return "ActiveSources";
            default:                        return "";
        }
    }
}

void Ambix_encoderAudioProcessor::addParameters()
{
    // Index order, starting values as the constructor sets the shadow copies.
    // Units stay empty: the legacy getParameterLabel() never overrode the
    // base class, so no host ever saw them.
    for (int i = 0; i < kTotalParams; ++i)
    {
        float initial = 0.f;

        if (i >= SourceParamsBase)
        {
            const int srcIdx = (i - SourceParamsBase) / kPerSourceParams;
            switch ((i - SourceParamsBase) % kPerSourceParams) {
                case SrcAz:   initial = source_params[srcIdx].az;   break;
                case SrcEl:   initial = source_params[srcIdx].el;   break;
                case SrcSize: initial = source_params[srcIdx].size; break;
                case SrcMute: initial = source_params[srcIdx].mute; break;
                case SrcSolo: initial = source_params[srcIdx].solo; break;
            }
        }
        else
        {
            switch (i) {
                case AzimuthParam:           initial = azimuth_param;           break;
                case ElevationParam:         initial = elevation_param;         break;
                case SizeParam:              initial = size_param;              break;
                case WidthParam:             initial = width_param;             break;
                case AzimuthSetParam:        initial = azimuth_set_param;       break;
                case AzimuthSetRelParam:     initial = azimuth_set_rel_param;   break;
                case AzimuthMvParam:         initial = azimuth_mv_param;        break;
                case ElevationSetParam:      initial = elevation_set_param;     break;
                case ElevationSetRelParam:   initial = elevation_set_rel_param; break;
                case ElevationMvParam:       initial = elevation_mv_param;      break;
                case SpeedParam:             initial = speed_param;             break;
                case LinkedParam:            initial = linked_param;            break;
                case NumActiveSourcesParam:  initial = num_active_sources_param; break;
                default: break;
            }
        }

        auto* p = new ambix::Parameter (*this, i, parameterName (i), initial,
                                        [this, i] (float v, int) { return parameterText (i, v); });
        paramObjs[(size_t) i] = p;
        addParameter (p);
    }
}

void Ambix_encoderAudioProcessor::beginGesture (int index)
{
    if (auto* p = getParam (index))
        if (gestureDepth[(size_t) index]++ == 0)
            p->beginChangeGesture();
}

void Ambix_encoderAudioProcessor::endGesture (int index)
{
    if (auto* p = getParam (index))
        if (gestureDepth[(size_t) index] > 0 && --gestureDepth[(size_t) index] == 0)
            p->endChangeGesture();
}

void Ambix_encoderAudioProcessor::setParameterFromUi (int index, float newValue)
{
    if (auto* p = getParam (index))
    {
        beginGesture (index);
        p->setValueNotifyingHost (newValue);
        endGesture (index);
    }
}

// The legacy setParameter() body, minus sendChangeMessage(): the parameter
// already holds newValue, the shadow copies still hold the previous values.
void Ambix_encoderAudioProcessor::parameterValueSet (int index, float newValue)
{
    // Shadows take what the parameter holds rather than newValue: if two
    // threads set the same parameter at once, the shadow then still ends up
    // where the parameter (and so the saved state) did.
    const float stored = paramObjs[index]->get();

    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int srcIdx = (index - SourceParamsBase) / kPerSourceParams;
        const int sub    = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz:   source_params[srcIdx].az   = stored; break;
            case SrcEl:   source_params[srcIdx].el   = stored; break;
            case SrcSize: source_params[srcIdx].size = stored; break;
            case SrcMute: source_params[srcIdx].mute = stored; break;
            case SrcSolo: source_params[srcIdx].solo = stored; break;
        }
        return;
    }

    float tempvalue = 0.0f;

    switch (index) {
        case AzimuthParam:
            azimuth_param = stored;
            break;

        case ElevationParam:
            elevation_param = stored;
            break;

        case SizeParam:
            size_param = stored;
            break;

        case WidthParam:
            width_param = stored;
            break;

        case LinkedParam: {
            const bool wasLinked = linked_param >= 0.5f;
            const bool nowLinked = newValue     >= 0.5f;
            linked_param = stored;
            if (wasLinked && ! nowLinked)
                linkedToUnlinkedSnapshot();
            else if (! wasLinked && nowLinked)
                unlinkedToLinkedSnapshot();
            break;
        }

        case NumActiveSourcesParam:
            num_active_sources_param = stored;
            break;

        case AzimuthSetParam:
            if ((newValue != azimuth_set_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
                paramObjs[AzimuthParam]->setValueNotifyingHost (newValue);
            azimuth_set_param = stored;
            break;

        case AzimuthSetRelParam:
            if ((newValue != azimuth_set_rel_param) && ((azimuth_mv_param > 0.48f) && (azimuth_mv_param < 0.52f)))
            {
                tempvalue = azimuth_param + (newValue - azimuth_set_rel_param);
                if (tempvalue < 0.0f)      tempvalue = 1.0f + tempvalue;
                else if (tempvalue > 1.0f) tempvalue = tempvalue - 1.0f;
                paramObjs[AzimuthParam]->setValueNotifyingHost (tempvalue);
            }
            azimuth_set_rel_param = stored;
            break;

        case AzimuthMvParam:
            azimuth_mv_param = stored;
            break;

        case ElevationSetParam:
            if ((newValue != elevation_set_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
                paramObjs[ElevationParam]->setValueNotifyingHost (newValue);
            elevation_set_param = stored;
            break;

        case ElevationSetRelParam:
            if ((newValue != elevation_set_rel_param) && ((elevation_mv_param > 0.48f) && (elevation_mv_param < 0.52f)))
            {
                tempvalue = elevation_param + (newValue - elevation_set_rel_param);
                if (tempvalue < 0.0f)      tempvalue = 1.0f + tempvalue;
                else if (tempvalue > 1.0f) tempvalue = tempvalue - 1.0f;
                paramObjs[ElevationParam]->setValueNotifyingHost (tempvalue);
            }
            elevation_set_rel_param = stored;
            break;

        case ElevationMvParam:
            elevation_mv_param = stored;
            break;

        case SpeedParam:
            speed_param = stored;
            break;

        default:
            break;
    }
}

// The legacy getParameterText(), for any value. MoveAzimuth / MoveElevation
// read the current MoveSpeed, as they always did.
juce::String Ambix_encoderAudioProcessor::parameterText (int index, float value) const
{
    String text;

    if (index >= SourceParamsBase && index < kTotalParams)
    {
        const int sub = (index - SourceParamsBase) % kPerSourceParams;
        switch (sub) {
            case SrcAz:   text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
            case SrcEl:   text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
            case SrcSize: text << String(value).substring(0, 5);                         break;
            case SrcMute: text << (value >= 0.5f ? "on" : "off"); break;
            case SrcSolo: text << (value >= 0.5f ? "on" : "off"); break;
        }
        return text;
    }

    const float speed = speed_param;

    switch (index) {
        case AzimuthParam:           text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case ElevationParam:         text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case SizeParam:              text << String(value).substring(0, 5); break;
        case WidthParam:             text << String(value * 360).substring(0, 5) << " deg"; break;
        case AzimuthSetParam:        text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case AzimuthSetRelParam:     text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;

        case AzimuthMvParam:
            if (value <= 0.48f)
                text << String(powf(speed*360.f, (0.45f - value)*2.22222f)).substring(0, 5) << " deg/sec";
            else if (value >= 0.52f)
                text << String(powf(speed*360.f, (value - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
            else
                text << "do not rotate";
            break;

        case ElevationSetParam:      text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;
        case ElevationSetRelParam:   text << String((value - 0.5f) * 360).substring(0, 5) << " deg"; break;

        case ElevationMvParam:
            if (value <= 0.48f)
                text << String(powf(speed*360.f, (0.45f - value)*2.22222f)).substring(0, 5) << " deg/sec";
            else if (value >= 0.52f)
                text << String(powf(speed*360.f, (value - 0.55f)*2.22222f)).substring(0, 5) << " deg/sec";
            else
                text << "do not rotate";
            break;

        case SpeedParam:             text << String((value) * 360).substring(0, 5) << " deg"; break;
        case LinkedParam:            text << (value >= 0.5f ? "linked" : "unlinked"); break;
        case NumActiveSourcesParam:  text << String (activeSourcesFromNorm (value)); break;

        default: break;
    }
    return text;
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
            paramObjs[AzimuthParam]->setValueNotifyingHost (wrap01 (azimuth_param   + dAz));
        if (moveEl)
            paramObjs[ElevationParam]->setValueNotifyingHost (wrap01 (elevation_param + dEl));
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
                paramObjs[(size_t) sourceParamIndex (i, SrcAz)]->setValueNotifyingHost (
                                           wrap01 (source_params[i].az + dAz));
            if (moveEl)
                paramObjs[(size_t) sourceParamIndex (i, SrcEl)]->setValueNotifyingHost (
                                           wrap01 (source_params[i].el + dEl));
        }
    }
}
#endif
