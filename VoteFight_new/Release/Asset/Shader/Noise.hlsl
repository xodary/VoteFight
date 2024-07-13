// Noise.hlsl

// 해시 함수
float hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

// 선형 보간 함수
float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

// 스무딩 함수
float smoothstep(float t)
{
    return t * t * (3.0 - 2.0 * t);
}

// 2D 값 노이즈 생성 함수
float noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);

    // 네 개의 코너 값
    float a = hash(i);
    float b = hash(i + float2(1.0, 0.0));
    float c = hash(i + float2(0.0, 1.0));
    float d = hash(i + float2(1.0, 1.0));

    // 보간
    float2 u = smoothstep(f);
    return lerp(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}
