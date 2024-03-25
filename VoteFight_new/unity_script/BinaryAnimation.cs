using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;
using UnityEngine.XR;

public class BinaryAnimation : MonoBehaviour
{
    public AnimationClip[] m_AnimationClips;

    private BinaryWriter binaryWriter = null;
    private int m_nFrames = 0;

    bool FindTextureByName(List<string> pTextureNamesList, Texture texture)
    {
        if (texture)
        {
            string strTextureName = string.Copy(texture.name).Replace(" ", "_");
            for (int i = 0; i < pTextureNamesList.Count; i++)
            {
                if (pTextureNamesList.Contains(strTextureName)) return(true);
            }
            pTextureNamesList.Add(strTextureName);
            return(false);
        }
        else
        {
            return(true);
        }
    }

    void WriteObjectName(Object obj)
    {
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(int i, Object obj)
    {
        binaryWriter.Write(i);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, Object obj)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, Object obj)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(i);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, int j, Object obj)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(i);
        binaryWriter.Write(j);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteObjectName(string strHeader, int i, Object obj, float f, int j, int k)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(i);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
        binaryWriter.Write(f);
        binaryWriter.Write(j);
        binaryWriter.Write(k);
    }

    void WriteString(string strToWrite)
    {
        binaryWriter.Write(strToWrite);
    }

    void WriteString(string strHeader, string strToWrite)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(strToWrite);
    }

    void WriteString(string strToWrite, int i)
    {
        binaryWriter.Write(strToWrite);
        binaryWriter.Write(i);
    }

    void WriteString(string strToWrite, int i, float f)
    {
        binaryWriter.Write(strToWrite);
        binaryWriter.Write(i);
        binaryWriter.Write(f);
    }

    void WriteInteger(int i)
    {
        binaryWriter.Write(i);
    }

    void WriteInteger(string strHeader, int i)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(i);
    }

    void WriteFloat(string strHeader, float f)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(f);
    }

    void WriteVector(Vector2 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
    }

    void WriteVector(string strHeader, Vector2 v)
    {
        binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector3 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
        binaryWriter.Write(v.z);
    }

    void WriteVector(string strHeader, Vector3 v)
    {
        binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Vector4 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
        binaryWriter.Write(v.z);
        binaryWriter.Write(v.w);
    }

    void WriteVector(string strHeader, Vector4 v)
    {
        binaryWriter.Write(strHeader);
        WriteVector(v);
    }

    void WriteVector(Quaternion q)
    {
        binaryWriter.Write(q.x);
        binaryWriter.Write(q.y);
        binaryWriter.Write(q.z);
        binaryWriter.Write(q.w);
    }

    void WriteVector(string strHeader, Quaternion q)
    {
        binaryWriter.Write(strHeader);
        WriteVector(q);
    }

    void WriteColor(Color c)
    {
        binaryWriter.Write(c.r);
        binaryWriter.Write(c.g);
        binaryWriter.Write(c.b);
        binaryWriter.Write(c.a);
    }

    void WriteColor(string strHeader, Color c)
    {
        binaryWriter.Write(strHeader);
        WriteColor(c);
    }

    void WriteTextureCoord(Vector2 uv)
    {
        binaryWriter.Write(uv.x);
        binaryWriter.Write(1.0f - uv.y);
    }

    void WriteVectors(string strHeader, Vector2[] vectors)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector2 v in vectors) WriteVector(v);
    }

    void WriteVectors(string strHeader, Vector3[] vectors)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector3 v in vectors) WriteVector(v);
    }

    void WriteVectors(Vector3[] vectors)
    {
        if (vectors.Length > 0) foreach (Vector3 v in vectors) WriteVector(v);
    }

    void WriteVectors(string strHeader, Vector4[] vectors)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(vectors.Length);
        if (vectors.Length > 0) foreach (Vector4 v in vectors) WriteVector(v); 
    }

    void WriteColors(string strHeader, Color[] colors)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(colors.Length);
        if (colors.Length > 0) foreach (Color c in colors) WriteColor(c);
    }

    void WriteTextureCoords(string strHeader, Vector2[] uvs)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(uvs.Length);
        if (uvs.Length > 0) foreach (Vector2 uv in uvs) WriteTextureCoord(uv);
    }

    void WriteIntegers(int[] pIntegers)
    {
        binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteIntegers(string strHeader, int[] pIntegers)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(pIntegers.Length);
        if (pIntegers.Length > 0) foreach (int i in pIntegers) binaryWriter.Write(i);
    }


    void WriteAnimationMatrixHierarchy(Transform current)
    {
        Vector3[] v = new Vector3[3];
        v[0] = current.localPosition;
        v[1] = current.localRotation.eulerAngles;
        v[2] = current.localScale;
        WriteVectors(v);
    }

    void WriteAnimationTransforms(string strHeader, Transform[] bones)
    {
        WriteString(strHeader, bones.Length);
        foreach (Transform bone in bones) WriteAnimationMatrixHierarchy(bone);
    }

    void WriteAnimationClipsInfo()
    {
        for (int j = 0; j < m_AnimationClips.Length; j++)
        {
            WriteString("<Animation>");
            WriteObjectName("<Name>", m_AnimationClips[j]);
            int nFramesPerSec = (int)m_AnimationClips[j].frameRate;
            int nKeyFrames = Mathf.CeilToInt(m_AnimationClips[j].length * nFramesPerSec);
            WriteInteger("<FrameRate>", nFramesPerSec);
            WriteInteger("<FrameCount>", nKeyFrames);
            WriteFloat("<Duration>", m_AnimationClips[j].averageDuration);
            WriteInteger("<SkinnedMeshes>", 1);

            float fFrameRate = (1.0f / nFramesPerSec), fKeyFrameTime = 0.0f;
            
            for (int k = 0; k < nKeyFrames; k++)
            {
                WriteFloat("<ElapsedTime>", fKeyFrameTime);
                m_AnimationClips[j].SampleAnimation(gameObject, fKeyFrameTime);
                SkinnedMeshRenderer renderer = transform.GetChild(0).GetComponent<SkinnedMeshRenderer>();
                WriteAnimationTransforms("<BoneTransformMatrix>", renderer.bones);
                fKeyFrameTime += fFrameRate;
            }
            WriteString("</Animation>");
        }
    }

    void Start()
    {
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name+"_Animation").Replace(" ", "_") + ".bin", FileMode.Create));

        WriteInteger("<Animations>", m_AnimationClips.Length);
        WriteAnimationClipsInfo();
        WriteString("</Animations>");

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

