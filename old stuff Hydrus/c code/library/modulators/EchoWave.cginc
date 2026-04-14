#ifndef ECHOWAVE
#define ECHOWAVE

    #include "../../../AudioLink/Shaders/AudioLink.cginc"
    #include "../../CSK/CSK.cginc"
    #include "../../CSK/Patterns/Simplex.cginc"
    #include "../../CSK/Patterns/Voronoi.cginc"

    struct NLConfig {
         uint BandsEnabled;
         float4 noAudiolinkColor;
         uint BassPatternSelection;
         float BassPatternPassThrough;
         uint BassMixMode;
         float4 BassColor;
         float BassScale;
         float3 BassMoveSpeed;
         float BassBandCenter;
         float BassBandWidth;
         uint MidLowPatternSelection;
         uint MidLowMixMode;
         float MidLowPatternPassThrough;
         float4 MidLowColor;
         float MidLowScale;
         float3 MidLowMoveSpeed;
         float MidLowBandCenter;
         float MidLowBandWidth;
         uint MidHighPatternSelection;
         float MidHighPatternPassThrough;
         uint MidHighMixMode;
         float4 MidHighColor;
         float MidHighScale;
         float3 MidHighMoveSpeed;
         float MidHighBandCenter;
         float MidHighBandWidth;
         uint HighPatternSelection;
         float HighPatternPassThrough;
         uint HighMixMode;
         float4 HighColor;
         float HighScale;
         float3 HighMoveSpeed;
         float HighBandCenter;
         float HighBandWidth;
         bool demoEnabled;
         uint demoBand;
    };


    float squareNoise(float3 pos) {
        float ret = 0;
        //[unroll]
        for (float x = -1; x <= 1; x++) {
            //[unroll]
            for (float y = -1; y <= 1; y++) {
                //[unroll]
                for (float z = -1; z <= 1; z++) {
                    float3 cpos = pos + float3(x,y,z);
                    float ran = (rng(floor(cpos)));
                    ran = ran - (rng(floor(cpos * 2)) > 0.7 ? 1 : 0);
                    ran = ran - (rng(floor(cpos * 3)) > 0.9 ? 1 : 0);
                    ran = max(ran, 0);
                    ret += x == 0 && y == 0 && z == 0 ? ran / 9 : ran / 81;
                }
            }
        }
        return ret;
    }

    float fbm(float3 x) {
        float v = 0.0;
        float a = 0.5;
        float3 shift = float3(100, 100, 100);
        //[unroll]
        for (int i = 0; i < 5; ++i) {
            float n = abs(SimplexNoise3D(x));
            n = 1 - pow(1 - n, 3);
            v += a * n;
            x = x * 2.0 + shift;
            a *= 0.5;
        }
        return v;
    }

    /*
     * 0 = Simplex Noise
     * 1 = square Noise
     * 2 = Fractional Brownian Motion Ridge
     * 3 = Voronoi Distance
     * 4 = Voronoi Random
     * 5 = Voronoi Edge
     */
    float noiseSelect(float3 pos, uint selection) {
        [branch] switch(selection) {
            case 0:
                return SimplexNoise3D(pos);
            case 1:
                return 1 - pow(1 - squareNoise(pos), 2);
            case 2:
                return fbm(pos);
            case 3:
                return voronoiNoise(pos).x;
            case 4:
                return voronoiNoise(pos).y;
            case 5:
                return voronoiNoise(pos).z;
            default:
                return 0;
        }
    }

    /*
         * 0 = Disabled
         * 1 = Mix
         * 2 = Add
         * 3 = Subtract
         * 4 = divide
         * 5 = Multiply
         * 6 = Lighten
         * 7 = Darken
         * 8 = Screen
         */
        float4 blend(float4 base, float4 al, uint mode) {
            [branch] switch(mode) {
                case 0:
                    return base.rgba;
                case 1:
                    return lerp(base.rgba, al.rgba, al.a);
                case 2:
                    return lerp(base.rgba, saturate(base.rgba + al.rgba), al.a);
                case 3:
                    return lerp(base.rgba, saturate(base.rgba - al.rgba), al.a);
                case 4:
                    return lerp(base.rgba, saturate(base.rgba / al.rgba), al.a);
                case 5:
                    return lerp(base.rgba, saturate(base.rgba * al.rgba), al.a);
                case 6:
                    return lerp(base.rgba, max(base.rgba, al.rgba), al.a);
                case 7:
                    return lerp(base.rgba, min(base.rgba, al.rgba), al.a);
                case 8:
                    return lerp(base.rgba, saturate(1 - (1 - base.rgba) * (1 - al.rgba)), al.a);
                default: 
                    return float4(1,0,1,1);
            }
        }

    float4 noiseLink(NLConfig config, float3 localPos) {
        float3 mposBass = (localPos + ((frac(_Time.yyy * 0.000001))* 20000 * config.BassMoveSpeed)) * config.BassScale;
        float3 mposLowMids = (localPos + ((frac((_Time.yyy + 243) * 0.000001))* 20000 * config.MidLowMoveSpeed)) * config.MidLowScale;
        float3 mposHighMids = (localPos + ((frac((_Time.yyy + 182) * 0.000001))* 20000 * config.MidHighMoveSpeed)) * config.MidHighScale;
        float3 mposHighs = (localPos + ((frac((_Time.yyy + 223) * 0.000001))* 20000 * config.HighMoveSpeed)) * config.HighScale;

        float albass = AudioLinkData(ALPASS_AUDIOBASS);
        float allowmids = AudioLinkData(ALPASS_AUDIOLOWMIDS);
        float alhighmids = AudioLinkData(ALPASS_AUDIOHIGHMIDS);
        float alhighs = AudioLinkData(ALPASS_AUDIOTREBLE);

        if (config.demoEnabled) {
            albass = config.demoBand & 1 ? 1 : 0;
            allowmids = config.demoBand & 2 ? 1 : 0;
            alhighmids = config.demoBand & 4 ? 1 : 0;
            alhighs = config.demoBand & 8 ? 1 : 0;
        }

        float noiseBass = config.BandsEnabled & 1 || config.demoEnabled ? noiseSelect(mposBass, config.BassPatternSelection) : 0;
        float noiseLowMids = config.BandsEnabled & 2 || config.demoEnabled ? noiseSelect(mposLowMids, config.MidLowPatternSelection) : 0;
        float noiseHighMids = config.BandsEnabled & 4 || config.demoEnabled ? noiseSelect(mposHighMids, config.MidHighPatternSelection) : 0;
        float noiseHighs = config.BandsEnabled & 8 || config.demoEnabled ? noiseSelect(mposHighs, config.HighPatternSelection) : 0;

        float4 basscolor = lerp((noiseBass < (config.BassBandCenter + (config.BassBandWidth/2)) ? (noiseBass > (config.BassBandCenter - (config.BassBandWidth/2)) ? 1 : 0) : 0), noiseBass, config.BassPatternPassThrough) * albass * config.BassColor.rgba;
        float4 lowMidscolor = lerp((noiseLowMids < (config.MidLowBandCenter + (config.MidLowBandWidth/2)) ? (noiseLowMids > (config.MidLowBandCenter - (config.MidLowBandWidth/2)) ? 1 : 0) : 0), noiseLowMids, config.MidLowPatternPassThrough) * allowmids * config.MidLowColor.rgba;
        float4 highMidscolor = lerp((noiseHighMids < (config.MidHighBandCenter + (config.MidHighBandWidth/2)) ? (noiseHighMids > (config.MidHighBandCenter - (config.MidHighBandWidth/2)) ? 1 : 0) : 0), noiseHighMids, config.MidHighPatternPassThrough) * alhighmids * config.MidHighColor.rgba;
        float4 highcolor = lerp((noiseHighs < (config.HighBandCenter + (config.HighBandWidth/2)) ? (noiseHighs > (config.HighBandCenter - (config.HighBandWidth/2)) ? 1 : 0) : 0), noiseHighs, config.HighPatternPassThrough) * alhighs * config.HighColor.rgba;
        
        return blend(blend(blend(blend(config.noAudiolinkColor, basscolor, config.BassMixMode), lowMidscolor, config.MidLowMixMode), highMidscolor, config.MidHighMixMode), highcolor, config.HighMixMode);
    }

#endif