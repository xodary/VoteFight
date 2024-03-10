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

    public GameObject[] objects;

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
    void WriteTextures(Material[] materials)
    {
        for (int i = 0; i < materials.Length; i++)
        {
            Texture texture;
            if (materials[i].HasProperty("_MainTex"))
            {
                texture = materials[i].GetTexture("_MainTex");
                if (texture != null) { WriteTextureName("<AlbedoMap>", texture); }
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                texture = materials[i].GetTexture("_BumpMap");
                if (texture != null) { WriteTextureName("<NormalMap>", texture); }
            }
        }
    }

    void WriteMaterials(GameObject current, int index)
    {
        WriteString("<Material>");
        WriteObjectName("<Name>", current);
        MeshRenderer meshRenderer = current.GetComponent<MeshRenderer>();
        Material[] materials = null;
        if (meshRenderer)
            materials = meshRenderer.sharedMaterials;
        else
            materials = current.GetComponent<SkinnedMeshRenderer>().sharedMaterials;
        WriteTextures(materials);
        WriteInteger("<Shaders>", 1);
        WriteString(shaders[index]);
        WriteInteger("<StateNum>", stateNum[index]);
        WriteString("</Material>");
    }


    void Start()
    {
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name).Replace(" ", "_") + ".bin", FileMode.Create));

        WriteInteger("<Materials>", objects.Length);

        for (int i = 0; i < objects.Length; i++)
        {
            WriteMaterials(objects[i], i);
        }

        WriteString("</Materials>");

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

