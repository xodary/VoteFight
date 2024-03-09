using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows;
using System.IO;
using static UnityEditor.UIElements.ToolbarMenu;
using UnityEditor;
using static TreeEditor.TextureAtlas;

public class TextTextures : MonoBehaviour
{
    public Texture[] textures;
    public int[] types;

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

    // 배열에서 특정 값의 인덱스를 찾는 함수
    int FindIndex(Texture[] arr, Texture value)
    {
        for (int i = 0; i < arr.Length; i++)
        {
            if (arr[i].name == value.name)
            {
                return i; // 값을 찾으면 해당 인덱스를 반환
            }
        }
        return -1; // 값을 찾지 못하면 -1 반환
    }
    void WriteTextures(int nLevel, Texture texture)
    {
        WriteLineString(nLevel, "<Texture>: ");
        WriteLineString(nLevel + 1, "<Type>: "+ types[FindIndex(textures, texture)]);
        WriteTextureName(nLevel + 1, "<FileName>:", texture);
        WriteLineString(nLevel, "</Texture>");
    }

    void Start()
    {
        m_rTextWriter = new StreamWriter(string.Copy(gameObject.name).Replace(" ", "_") + ".txt");

        WriteLineString("<Textures>: " + textures.Length);
        foreach (Texture tex in textures)
        {
            WriteTextures(1, tex);
        }
        WriteString("</Textures>");

        m_rTextWriter.Flush();
        m_rTextWriter.Close();
    }
}

