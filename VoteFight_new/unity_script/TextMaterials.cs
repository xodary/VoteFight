using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows;
using System.IO;
using static UnityEditor.UIElements.ToolbarMenu;

public class TextMaterials : MonoBehaviour
{
    public int[] shaders;
    public int[] stateNum;

    public GameObject[] objects;

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
        m_rTextWriter.WriteLine(string.Copy(texture.name).Replace(" ", "_"));
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

    void WriteTextures(int nLevel, Material[] materials)
    {
        for(int i = 0; i < materials.Length; i++)
        {
            Texture texture;
            if (materials[i].HasProperty("_MainTex"))
            {
                texture = materials[i].GetTexture("_MainTex");
                if(texture != null) { WriteTextureName(nLevel, "<AlbedoMap>:", texture); }
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                texture = materials[i].GetTexture("_BumpMap");
                if (texture != null) { WriteTextureName(nLevel, "<NormalMap>:", texture); }
            }
        }
    }
    void WriteMaterials(int nLevel, GameObject current, int index)
    {
        WriteLineString(nLevel, "<Material>");
        WriteObjectName(nLevel+1, "<Name>:", current);
        Material[] materials = current.GetComponent<MeshRenderer>().sharedMaterials;
        WriteTextures(nLevel + 1, materials);
        WriteLineString(nLevel+1, "<Shaders> " + shaders[index]);
        WriteLineString(nLevel+1, "<StateNum> " + stateNum[index]);
    }

    void Start()
    {
        m_rTextWriter = new StreamWriter(string.Copy(gameObject.name).Replace(" ", "_") + ".txt");

        WriteLineString("<Materials>: " + objects.Length);

        for(int i = 0; i < objects.Length; i++)
        {
            WriteMaterials(1, objects[i], i);
        }

        WriteLineString("</Materials>");
        m_rTextWriter.Flush();
        m_rTextWriter.Close();
    }
}

