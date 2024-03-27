using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;
using System;

public class BinaryMaterials : MonoBehaviour
{
    public String[] shaders;
    public int[] stateNum;

    public Material[] materials;

    private BinaryWriter binaryWriter = null;
    private int m_nFrames = 0;

    void WriteObjectName(string strHeader, GameObject obj)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write((obj) ? string.Copy(obj.name).Replace(" ", "_") : "null");
    }

    void WriteString(string strToWrite)
    {
        binaryWriter.Write(strToWrite);
    }

    void WriteTextureName(string strHeader, Texture texture)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(string.Copy(texture.name).Replace(" ", "_"));
    }

    void WriteInteger(string strHeader, int i)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(i);
    }
    void WriteTextures(Material mat)
    {
        Texture texture;
        if (mat.HasProperty("_MainTex"))
        {
            texture = mat.GetTexture("_MainTex");
            if (texture != null) { WriteTextureName("<AlbedoMap>", texture); }
        }
        if (mat.HasProperty("_BumpMap"))
        {
            texture = mat.GetTexture("_BumpMap");
            if (texture != null) { WriteTextureName("<NormalMap>", texture); }
        }
    }

    void WriteMaterials(Material mat, int index)
    {
        WriteString("<Material>");
        WriteString("<Name>");
        WriteString(mat.name);
        WriteTextures(mat);
        WriteInteger("<Shaders>", 1);
        WriteString(shaders[index]);
        WriteInteger("<StateNum>", stateNum[index]);
        WriteString("</Material>");
    }


    void Start()
    {
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name).Replace(" ", "_") + ".bin", FileMode.Create));

        WriteInteger("<Materials>", materials.Length);

        for (int i = 0; i < materials.Length; i++)
        {
            WriteMaterials(materials[i], i);
        }

        WriteString("</Materials>");

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

