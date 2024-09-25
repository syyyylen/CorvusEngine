static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0 / float(NumSamples);

RWTexture2DArray<float4> PrefilterMap : register(u0);
TextureCube EnvironmentMap : register(t1);
SamplerState CubeSampler : register(s2);

cbuffer PrefilterMapFilterSettings : register(b3)
{
	float4 roughness;
};

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 sampleHammersley(uint i, uint n)
{
	return float2(float(i) / float(n), radicalInverse_VdC(i));
}

float3 sampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	float phi = TwoPI * u1;

	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float3 getSamplingVector(uint3 ThreadID)
{
	float outputWidth, outputHeight, outputDepth;
	PrefilterMap.GetDimensions(outputWidth, outputHeight, outputDepth);

    float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
    float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0;

	float3 ret = float3(1.0f, 1.0f, 1.0f);
	switch(ThreadID.z)
	{
	case 0: ret = float3(1.0,  uv.y, -uv.x); break;
	case 1: ret = float3(-1.0, uv.y,  uv.x); break;
	case 2: ret = float3(uv.x, 1.0, -uv.y); break;
	case 3: ret = float3(uv.x, -1.0, uv.y); break;
	case 4: ret = float3(uv.x, uv.y, 1.0); break;
	case 5: ret = float3(-uv.x, uv.y, -1.0); break;
	}
    return normalize(ret);
}

void computeBasisVectors(const float3 N, out float3 S, out float3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

float3 tangentToWorld(const float3 v, const float3 N, const float3 S, const float3 T)
{
	return S * v.x + T * v.y + N * v.z;
}

[numthreads(32, 32, 1)]
void Main(uint3 ThreadID : SV_DispatchThreadID)
{
	uint outputWidth, outputHeight, outputDepth;
	PrefilterMap.GetDimensions(outputWidth, outputHeight, outputDepth);
	if(ThreadID.x >= outputWidth || ThreadID.y >= outputHeight) {
		return;
	}
	
	float inputWidth, inputHeight, inputLevels;
	EnvironmentMap.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	float wt = 4.0 * PI / (6 * inputWidth * inputHeight);
	
	float3 N = getSamplingVector(ThreadID);
	float3 Lo = N;
	
	float3 S, T;
	computeBasisVectors(N, S, T);

	float3 color = 0;
	float weight = 0;

	for(int i = 0; i < NumSamples; i++)
	{
		float2 u = sampleHammersley(i, NumSamples);
		float3 Lh = tangentToWorld(sampleGGX(u.x, u.y, roughness.x), N, S, T);

		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if(cosLi > 0.0)
		{
			float cosLh = max(dot(N, Lh), 0.0);
			float pdf = ndfGGX(cosLh, roughness.x) * 0.25;
			float ws = 1.0 / (NumSamples * pdf);
			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color  += EnvironmentMap.SampleLevel(CubeSampler, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}
	color /= weight;
	
	// color = lerp(float3(1, 1, 1), float3(0, 0, 0), roughness.x);
	PrefilterMap[ThreadID] = float4(color, 1.0);
}