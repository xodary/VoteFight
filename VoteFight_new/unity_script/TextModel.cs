using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows;
using System.IO;
using static UnityEditor.UIElements.ToolbarMenu;

public class TextModel : MonoBehaviour
{
    public int classType;
    public AnimationClip[] m_raAnimationClips;

    private List<string> m_rTextureNamesListForCounting = new List<string>();
    private List<string> m_rTextureNamesListForWriting = new List<string>();

    private StreamWriter m_rTextWriter = null;
    private int m_nFrames = 0;

    const float EPSILON = 1.0e-6f;

    bool IsZero(float fValue) { return ((Mathf.Abs(fValue) < EPSILON)); }
    bool IsEqual(float fA, float fB) { return (IsZero(fA - fB)); }

    bool FindTextureByName(List<string> textureNamesList, Texture texture)
    {
        if (texture)
        {
            string strTextureName = string.Copy(texture.name).Replace(" ", "_");
            for (int i = 0; i < textureNamesList.Count; i++)
            {
                if (textureNamesList.Contains(strTextureName)) return(true);
            }
            textureNamesList.Add(strTextureName);
            return(false);
        }
        else
        {
            return(true);
        }
    }

    void WriteTabs(int nLevel)
    {
        for (int i = 0; i < nLevel; i++) m_rTextWriter.Write("\t");
    }

     void WriteObjectName(int nLevel, string strHeader, Object obj)
    {
        WriteTabs(nLevel);
        m_rTextWriter.Write(strHeader + " ");
        m_rTextWriter.WriteLine((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

   void WriteString(string strToWrite)
    {
        m_rTextWriter.Write(strToWrite);
    }

    void WriteString(int nLevel, string strToWrite)
    {
        WriteTabs(nLevel);
        m_rTextWriter.Write(strToWrite);
    }

    void WriteLineString(string strToWrite)
    {
        m_rTextWriter.WriteLine(strToWrite);
    }

    void WriteLineString(int nLevel, string strToWrite)
    {
        WriteTabs(nLevel);
        m_rTextWriter.WriteLine(strToWrite);
    }

    void WriteTextureName(int nLevel, string strHeader, Texture texture)
    {
        WriteTabs(nLevel);
        m_rTextWriter.Write(strHeader + " ");
        if (texture)
        {
            if (FindTextureByName(m_rTextureNamesListForWriting, texture))
            {
                m_rTextWriter.WriteLine("@" + string.Copy(texture.name).Replace(" ", "_"));
            }
            else
            {
                m_rTextWriter.WriteLine(string.Copy(texture.name).Replace(" ", "_"));
            }
        }
        else
        {
            m_rTextWriter.WriteLine("null");
        }
    }

    void WriteVector(Vector2 v)
    {
        m_rTextWriter.Write(v.x + " " + v.y + " ");
    }

    void WriteVector(string strHeader, Vector2 v)
    {
        m_rTextWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector3 v)
    {
        m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " ");
    }

    void WriteVector(string strHeader, Vector3 v)
    {
        m_rTextWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector4 v)
    {
        m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " " + v.w + " ");
    }

    void WriteVector(string strHeader, Vector4 v)
    {
        m_rTextWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Quaternion q)
    {
        m_rTextWriter.Write(q.x + " " + q.y + " " + q.z + " " + q.w + " ");
    }

    void WriteVector(string strHeader, Quaternion q)
    {
        m_rTextWriter.Write(strHeader);
        WriteVector(q);
    }

    void WriteVectors(int nLevel, string strHeader, Vector2[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector2 v in vectors) m_rTextWriter.Write(v.x + " " + v.y + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteVectors(int nLevel, string strHeader, Vector3[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector3 v in vectors) m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteVectors(int nLevel, string strHeader, Vector4[] vectors)
    {
        WriteString(nLevel, strHeader + " " + vectors.Length + " ");
        if (vectors.Length > 0)
        {
            foreach (Vector4 v in vectors) m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " " + v.w + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteColors(int nLevel, string strHeader, Color[] colors)
    {
        WriteString(nLevel, strHeader + " " + colors.Length + " ");
        if (colors.Length > 0)
        {
            foreach (Color c in colors) m_rTextWriter.Write(c.r + " " + c.g + " " + c.b + " " + c.a + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteTextureCoords(int nLevel, string strHeader, Vector2[] uvs)
    {
        WriteString(nLevel, strHeader + " " + uvs.Length + " ");
        if (uvs.Length > 0)
        {
            foreach (Vector2 uv in uvs) m_rTextWriter.Write(uv.x + " " + (1.0f - uv.y) + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteIntegers(int nLevel, string strHeader, int[] integers)
    {
        WriteString(nLevel, strHeader + " " + integers.Length + " ");
        if (integers.Length > 0)
        {
            foreach (int i in integers) m_rTextWriter.Write(i + " ");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteMatrix(Matrix4x4 matrix)
    {
        m_rTextWriter.Write(matrix.m00 + " " + matrix.m10 + " " + matrix.m20 + " " + matrix.m30 + " ");
        m_rTextWriter.Write(matrix.m01 + " " + matrix.m11 + " " + matrix.m21 + " " + matrix.m31 + " ");
        m_rTextWriter.Write(matrix.m02 + " " + matrix.m12 + " " + matrix.m22 + " " + matrix.m32 + " ");
        m_rTextWriter.Write(matrix.m03 + " " + matrix.m13 + " " + matrix.m23 + " " + matrix.m33 + " ");
    }

    void WriteMatrix(Vector3 position, Quaternion rotation, Vector3 scale)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(position, rotation, scale);
        WriteMatrix(matrix);
    }

    void WriteMaterials(int nLevel, Material[] materials)
    {
        WriteLineString(nLevel, "<Materials> " + materials.Length);
        if (materials.Length > 0)
        {
            for (int i = 0; i < materials.Length; i++)
            {
                WriteLineString(nLevel+1, "<Material>");

                if (materials[i].HasProperty("_MainTex"))
                {
                    Texture mainAlbedoMap = materials[i].GetTexture("_MainTex");
                    if(mainAlbedoMap) WriteLineString(nLevel+2, mainAlbedoMap.name);
                }
                if (materials[i].HasProperty("_BumpMap"))
                {
                    Texture bumpMap = materials[i].GetTexture("_BumpMap");
                    if(bumpMap) WriteLineString(nLevel+2, bumpMap.name);
                }
            }
        }
    }

    void WriteFrameInfo(int nLevel, Transform current)
    {
        WriteLineString(nLevel + 1, "<ClassType> " + classType);
        WriteLineString(nLevel + 1, "<Name> " + gameObject.name);
        WriteLineString(nLevel + 1, "<IsActive> " + gameObject.activeSelf);
        WriteString(1, "<Transform> ");
        Vector3[] v = new Vector3[3];
        v[0] = transform.localPosition;
        v[1] = transform.localRotation.eulerAngles;
        v[2] = transform.localScale;
        WriteVectors(1, v);
        MeshFilter meshFilter = GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = GetComponent<MeshRenderer>();
        if (meshFilter && meshRenderer)
        {
            Mesh mesh = meshFilter.sharedMesh;
            WriteObjectName(nLevel + 1, "<Mesh>", mesh);
            Material[] materials = meshRenderer.materials;
            if (materials.Length > 0) WriteMaterials(1, materials);
            WriteLineString(nLevel + 1, "<Collider> " + mesh.bounds.center.x + " " + mesh.bounds.center.y + " " + mesh.bounds.center.z + " " + mesh.bounds.extents.x + " " + mesh.bounds.extents.y + " " + mesh.bounds.extents.z);
        }
        else
        {
            SkinnedMeshRenderer skinMeshRenderer = GetComponent<SkinnedMeshRenderer>();
            if (skinMeshRenderer)
            {
                WriteObjectName(nLevel + 1, "<Mesh>", skinMeshRenderer.sharedMesh);
                Material[] materials = skinMeshRenderer.materials;
                if (materials.Length > 0) WriteMaterials(1, materials);
                WriteLineString(nLevel + 1, "<Bounds>: " + skinMeshRenderer.localBounds.center.x + " " + skinMeshRenderer.localBounds.center.y + " " + skinMeshRenderer.localBounds.center.z + " " + skinMeshRenderer.localBounds.extents.x + " " + skinMeshRenderer.localBounds.extents.y + " " + skinMeshRenderer.localBounds.extents.z);
            }
        }
    }

    void WriteFrameHierarchyInfo(int nLevel, Transform current)
    {
        WriteFrameInfo(nLevel, current);

        WriteLineString(nLevel+1, "<ChildCount> " + current.childCount);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameHierarchyInfo(nLevel+2, current.GetChild(k));
        }
    }

    void WriteFrameAnimationHierarchy(Transform current)
    {
        WriteMatrix(current.localPosition, current.localRotation, current.localScale);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameAnimationHierarchy(current.GetChild(k));
        }
    }

    void WriteAnimationTransforms(int nLevel, string strHeader)
    {
        WriteString(nLevel, strHeader);
        WriteFrameAnimationHierarchy(transform);
        m_rTextWriter.WriteLine(" ");
    }

    void WriteSkinnedMeshAnimationTransforms(int nLevel, string strHeader, SkinnedMeshRenderer skinnedMeshRenderer)
    {
        WriteString(nLevel, strHeader);
        for (int i = 0; i < skinnedMeshRenderer.bones.Length; i++)
        {
            WriteMatrix(skinnedMeshRenderer.bones[i].localPosition, skinnedMeshRenderer.bones[i].localRotation, skinnedMeshRenderer.bones[i].localScale);
        }
        m_rTextWriter.WriteLine(" ");
    }


    void WriteVectors(int nLevel, Vector3[] vectors)
    {
        foreach (Vector3 v in vectors) m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " ");
        m_rTextWriter.WriteLine(" ");

    }
    void WriteAnimationClipsInfo(int nLevel)
    {
        WriteLineString(nLevel, "<AnimationSets>: " + m_raAnimationClips.Length);
#if _WITH_SKINNED_BONES_ANIMATION
        WriteSkinnedMeshFrameNames(nLevel+1, "<FrameNames>: " + m_pSkinnedMeshRenderers.Length);
#else
        // WriteFrameNames(nLevel + 1, "<FrameNames>: " + m_nFrames + " ");
#endif
        for (int j = 0; j < m_raAnimationClips.Length; j++)
        {
            int nFramesPerSec = (int)m_raAnimationClips[j].frameRate;
            int nKeyFrames = Mathf.CeilToInt(m_raAnimationClips[j].length * nFramesPerSec);
            WriteLineString(nLevel + 1, "<AnimationSet>: " + j + " " + string.Copy(m_raAnimationClips[j].name).Replace(" ", "_") + " " + m_raAnimationClips[j].length + " " + nFramesPerSec + " " + nKeyFrames);

            float fFrameRate = (1.0f / nFramesPerSec), fKeyFrameTime = 0.0f;
            for (int k = 0; k < nKeyFrames; k++)
            {
                m_raAnimationClips[j].SampleAnimation(gameObject, fKeyFrameTime);

#if _WITH_SKINNED_BONES_ANIMATION
                WriteSkinnedMeshAnimationTransforms(nLevel+2, "<Transforms>: " + k + " " + fKeyFrameTime);
#else
                WriteAnimationTransforms(nLevel + 2, "<Transforms>: " + k + " " + fKeyFrameTime + " ");
#endif
                fKeyFrameTime += fFrameRate;
            }
        }

        WriteLineString(nLevel, "</AnimationSets>");
    }

    void Start()
    {
        m_rTextWriter = new StreamWriter(string.Copy(gameObject.name).Replace(" ", "_") + ".txt");

        WriteLineString("<Frames>");
        WriteFrameHierarchyInfo(0, transform);
        WriteLineString("</Frames>");

        if (m_raAnimationClips.Length > 0)
        {
            WriteLineString("<Animator>");
            WriteAnimationClipsInfo(1);
            WriteLineString("</Animator>");
        }



        m_rTextWriter.Flush();
        m_rTextWriter.Close();

        print("Model Text Write Completed");
    }
}

