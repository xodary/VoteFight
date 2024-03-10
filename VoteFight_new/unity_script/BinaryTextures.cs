using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;

public class BinaryTextures : MonoBehaviour
{
    public Texture[] textures;
    public int[] types;

    private BinaryWriter binaryWriter = null;
    private int m_nFrames = 0;

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

    void WriteTextures(int index, Texture texture)
    {
        WriteString("<Texture>");
        WriteInteger("<Type>", types[index]);
        WriteTextureName("<FileName>", texture);
        WriteString("</Texture>");
    }

    void Start()
    {
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name).Replace(" ", "_") + ".bin", FileMode.Create));

        WriteInteger("<Textures>", textures.Length);
        for(int i = 0; i < textures.Length; i++) 
        {
            WriteTextures(i, textures[i]);
        }
        WriteString("</Textures>");

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

