using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows;
using System.IO;
using static UnityEditor.UIElements.ToolbarMenu;
using UnityEngine.UIElements;

public class TextScene : MonoBehaviour
{
    public int[] groupType;

    private List<string> m_rTextureNamesListForCounting = new List<string>();
    private List<string> m_rTextureNamesListForWriting = new List<string>();

    private StreamWriter m_rTextWriter = null;
    private int m_nFrames = 0;

    const float EPSILON = 1.0e-6f;

    bool IsZero(float fValue) { return ((Mathf.Abs(fValue) < EPSILON)); }
    bool IsEqual(float fA, float fB) { return (IsZero(fA - fB)); }

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

    void WriteVectors(int nLevel, Vector3[] vectors)
    {
        foreach (Vector3 v in vectors) m_rTextWriter.Write(v.x + " " + v.y + " " + v.z + " ");
        m_rTextWriter.WriteLine(" ");
    }


    void WriteScene(Transform model, int index)
    {
        WriteLineString(0, "<GroupType> " + groupType[index]);
        WriteObjectName(0, "<FileName>", model.gameObject);
        WriteLineString(0, "<Instance> " + model.childCount);
        WriteLineString(0, "<IsActive> ");
        for (int i = 0; i < model.childCount; ++i)
        {
            if(model.GetChild(i).gameObject.activeSelf)
                WriteString(0, "1 ");
            else
                WriteString(0, "0 ");
        }
        WriteLineString("");
        WriteLineString(0, "<Transforms> ");
        for (int i = 0; i < model.childCount; ++i)
        {
            Vector3[] v = new Vector3[3];
            v[0] = model.GetChild(i).localPosition;
            v[1] = model.GetChild(i).localRotation.eulerAngles;
            v[2] = model.GetChild(i).localScale;
            WriteVectors(0, v);
        }
    }

    void Start()
    {
        m_rTextWriter = new StreamWriter(string.Copy(gameObject.name).Replace(" ", "_") + ".txt");

        for (int i = 0; i < transform.childCount; ++i)
        {
            Transform child = transform.GetChild(i);
            WriteScene(child, i);
        }


        m_rTextWriter.Flush();
        m_rTextWriter.Close();

        print("Model Text Write Completed");
    }
}

