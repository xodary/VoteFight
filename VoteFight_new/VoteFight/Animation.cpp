#include "pch.h"
#include "Animation.h"
#include "AssetManager.h"
#include "Object.h"

CAnimation::CAnimation() :
	m_frameRate(),
	m_frameCount(),
	m_duration()
{
}

CAnimation::~CAnimation()
{
}

int CAnimation::GetFrameRate()
{
	return m_frameRate;
}

int CAnimation::GetFrameCount()
{
	return m_frameCount;
}

float CAnimation::GetDuration()
{
	return m_duration;
}

//=========================================================================================================================

CSkinningAnimation::CSkinningAnimation() :
	m_bonePositions(),
	m_boneRotations(),
	m_boneScales()
{
}

CSkinningAnimation::~CSkinningAnimation()
{
}

const vector<vector<vector<XMFLOAT3>>>& CSkinningAnimation::GetPositions()
{
	return m_bonePositions;
}

const vector<vector<vector<XMFLOAT3>>>& CSkinningAnimation::GetRotations()
{
	return m_boneRotations;
}

const vector<vector<vector<XMFLOAT3>>>& CSkinningAnimation::GetScales()
{
	return m_boneScales;
}

void CSkinningAnimation::Load(ifstream& in)
{
	string str;
	int skinnedMeshCount = 0;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Name>")
		{
			File::ReadStringFromFile(in, m_name);
		}
		else if (str == "<FrameRate>")
		{
			in.read(reinterpret_cast<char*>(&m_frameRate), sizeof(int));
		}
		else if (str == "<FrameCount>")
		{
			in.read(reinterpret_cast<char*>(&m_frameCount), sizeof(int));
		}
		else if (str == "<Duration>")
		{
			in.read(reinterpret_cast<char*>(&m_duration), sizeof(float));
		}
		else if (str == "<SkinnedMeshes>")
		{
			in.read(reinterpret_cast<char*>(&skinnedMeshCount), sizeof(int));
			m_bonePositions.resize(skinnedMeshCount);
			m_boneRotations.resize(skinnedMeshCount);
			m_boneScales.resize(skinnedMeshCount);
		}
		else if (str == "<ElapsedTime>")
		{
			float elapsedTime = 0.0f;

			in.read(reinterpret_cast<char*>(&elapsedTime), sizeof(float));

			for (int i = 0; i < skinnedMeshCount; ++i)
			{
				// <BoneTransformMatrix>
				File::ReadStringFromFile(in, str);

				int boneCount = 0;

				in.read(reinterpret_cast<char*>(&boneCount), sizeof(int));

				if (m_bonePositions[i].empty())
				{
					m_bonePositions[i].resize(boneCount);
					m_boneRotations[i].resize(boneCount);
					m_boneScales[i].resize(boneCount);
				}

				for (int j = 0; j < boneCount; ++j)
				{
					// localPosition, localRotation, localScale
					XMFLOAT3 transform[3] = {};

					in.read(reinterpret_cast<char*>(&transform[0]), 3 * sizeof(XMFLOAT3));
					m_bonePositions[i][j].push_back(transform[0]);
					m_boneRotations[i][j].push_back(transform[1]);
					m_boneScales[i][j].push_back(transform[2]);
				}
			}
		}
		else if (str == "</Animation>")
		{
			break;
		}
	}
}

//=========================================================================================================================

CUIAnimation::CUIAnimation() :
	m_uiPositions(),
	m_uiRotations(),
	m_uiScales(),
	m_uiMaterialColors()
{
}

CUIAnimation::~CUIAnimation()
{
}

const vector<vector<XMFLOAT3>>& CUIAnimation::GetPositions()
{
	return m_uiPositions;
}

const vector<vector<XMFLOAT3>>& CUIAnimation::GetRotations()
{
	return m_uiRotations;
}

const vector<vector<XMFLOAT3>>& CUIAnimation::GetScales()
{
	return m_uiScales;
}

const vector<vector<XMFLOAT4>>& CUIAnimation::GetColors()
{
	return m_uiMaterialColors;
}

void CUIAnimation::Load(ifstream& in)
{
	string str;
	int idx = 0;

	while (true)
	{
		File::ReadStringFromFile(in, str);

		if (str == "<Name>")
		{
			File::ReadStringFromFile(in, m_name);
		}
		else if (str == "<FrameRate>")
		{
			in.read(reinterpret_cast<char*>(&m_frameRate), sizeof(int));
		}
		else if (str == "<FrameCount>")
		{
			in.read(reinterpret_cast<char*>(&m_frameCount), sizeof(int));
		}
		else if (str == "<Duration>")
		{
			in.read(reinterpret_cast<char*>(&m_duration), sizeof(float));
		}
		else if (str == "<ElapsedTime>")
		{
			float elapsedTime = 0.0f;

			in.read(reinterpret_cast<char*>(&elapsedTime), sizeof(float));

			// <RectTransform>
			File::ReadStringFromFile(in, str);

			int uiCount = 0;

			in.read(reinterpret_cast<char*>(&uiCount), sizeof(int));

			if (m_uiPositions.empty())
			{
				m_uiPositions.resize(m_frameCount);
				m_uiRotations.resize(m_frameCount);
				m_uiScales.resize(m_frameCount);
				m_uiMaterialColors.resize(m_frameCount);
			}

			for (int i = 0; i < uiCount; ++i)
			{
				// localPosition, localRotation, localScale
				XMFLOAT3 transform[3] = {};

				in.read(reinterpret_cast<char*>(&transform[0]), 3 * sizeof(XMFLOAT3));
				m_uiPositions[idx].push_back(transform[0]);
				m_uiRotations[idx].push_back(transform[1]);
				m_uiScales[idx].push_back(transform[2]);
			}

			// <Color>
			File::ReadStringFromFile(in, str);

			// Material을 가지고 있는 UI의 수
			uiCount = 0;
			in.read(reinterpret_cast<char*>(&uiCount), sizeof(int));

			for (int i = 0; i < uiCount; ++i)
			{
				XMFLOAT4 color = {};

				in.read(reinterpret_cast<char*>(&color), sizeof(XMFLOAT4));
				m_uiMaterialColors[idx].push_back(color);
			}

			++idx;
		}
		else if (str == "</Animation>")
		{
			break;
		}
	}
}
