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