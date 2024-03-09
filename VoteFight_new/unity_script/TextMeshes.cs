using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Windows;
using System.IO;
using static UnityEditor.UIElements.ToolbarMenu;

public class TextMeshes : MonoBehaviour
{
    public GameObject[] meshObjects;
    
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

    void WriteTransform(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        WriteVector(current.localPosition);
        WriteVector(current.localEulerAngles);
        WriteVector(current.localScale);
        WriteVector(current.localRotation);
        m_rTextWriter.WriteLine(" ");
    }

    void WriteLocalMatrix(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);
        WriteMatrix(matrix);
        m_rTextWriter.WriteLine(" ");
    }

    void WriteWorldMatrix(int nLevel, string strHeader, Transform current)
    {
        WriteString(nLevel, strHeader + " ");
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.position, current.rotation, current.lossyScale);
        WriteMatrix(matrix);
        m_rTextWriter.WriteLine(" ");
    }

    void WriteBoneTransforms(int nLevel, string strHeader, Transform[] bones)
    {
        WriteString(nLevel, strHeader + " " + bones.Length + " ");
        if (bones.Length > 0)
        {
            foreach (Transform bone in bones)
            {
                WriteMatrix(bone.localPosition, bone.localRotation, bone.localScale);
            }
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteBoneNames(int nLevel, string strHeader, Transform[] bones)
    {
        WriteString(nLevel, strHeader + " " + bones.Length + " ");
        if (bones.Length > 0)
        {
            foreach (Transform transform in bones)
            {
                m_rTextWriter.Write(string.Copy(transform.gameObject.name).Replace(" ", "_") + " ");
            }
            m_rTextWriter.WriteLine(" ");
            WriteString(nLevel, "<BoneHierarchy>:");
            WriteBoneNameHierarchy(bones[0]);
            WriteString(nLevel, "</BoneHierarchy>:");
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteBoneNameHierarchy(Transform parant)
    {
        m_rTextWriter.Write(string.Copy(parant.gameObject.name).Replace(" ", "_") + " ");
        WriteString(parant.childCount + " ");
        for (int k = 0; k < parant.childCount; k++) m_rTextWriter.Write(string.Copy(parant.GetChild(k).gameObject.name).Replace(" ", "_") + " ");
        for (int k = 0; k < parant.childCount; k++) WriteBoneNameHierarchy(parant.GetChild(k));
    }

    void WriteMatrixes(int nLevel, string strHeader, Matrix4x4[] matrixes)
    {
        WriteString(nLevel, strHeader + " " + matrixes.Length + " ");
        if (matrixes.Length > 0)
        {
            foreach (Matrix4x4 matrix in matrixes)
            {
                WriteMatrix(matrix);
            }
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteBoneIndices(int nLevel, string strHeader, BoneWeight[] boneWeights)
    {
        WriteString(nLevel, strHeader + " " + boneWeights.Length + " ");
        if (boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in boneWeights)
            {
                m_rTextWriter.Write(boneWeight.boneIndex0 + " " + boneWeight.boneIndex1 + " " + boneWeight.boneIndex2 + " " + boneWeight.boneIndex3 + " ");
            }
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteBoneWeights(int nLevel, string strHeader, BoneWeight[] boneWeights)
    {
        WriteString(nLevel, strHeader + " " + boneWeights.Length + " ");
        if (boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in boneWeights)
            {
                m_rTextWriter.Write(boneWeight.weight0 + " " + boneWeight.weight1 + " " + boneWeight.weight2 + " " + boneWeight.weight3 + " ");
            }
        }
        m_rTextWriter.WriteLine(" ");
    }

    void WriteMeshInfo(int nLevel, Mesh mesh)
    {
        WriteLineString(nLevel, "<Mesh>: ");

        WriteObjectName(nLevel+1, "<Name>: ", mesh);

        WriteVectors(nLevel+1, "<Positions>:", mesh.vertices);
        WriteVectors(nLevel+1, "<Normals>:", mesh.normals);

        if ((mesh.normals.Length > 0) && (mesh.tangents.Length > 0))
        {
            Vector3[] tangents = new Vector3[mesh.tangents.Length];
            Vector3[] bitangents = new Vector3[mesh.tangents.Length];
            for (int i = 0; i < mesh.tangents.Length; i++)
            {
                tangents[i] = new Vector3(mesh.tangents[i].x, mesh.tangents[i].y, mesh.tangents[i].z);
                bitangents[i] = Vector3.Normalize(Vector3.Cross(mesh.normals[i], tangents[i])) * mesh.tangents[i].w;
            }

            WriteVectors(nLevel+1, "<Tangents>:", tangents);
            WriteVectors(nLevel+1, "<BiTangents>:", bitangents);
        }
        WriteTextureCoords(nLevel+1, "<TexCoords>:", mesh.uv);

        WriteLineString(nLevel+1, "<SubMeshes>: " + mesh.subMeshCount);
        if (mesh.subMeshCount > 0)
        {
            for (int i = 0; i < mesh.subMeshCount; i++)
            {
                int[] subindicies = mesh.GetTriangles(i);
                WriteIntegers(nLevel+2, "<SubMesh>: " + i, subindicies);
            }
        }

        WriteLineString(nLevel, "</Mesh>");
    }

    void WriteSkinnedMeshInfo(int nLevel, SkinnedMeshRenderer skinMeshRenderer)
    {
        Mesh mesh = skinMeshRenderer.sharedMesh;
        WriteMeshInfo(nLevel, mesh);

        WriteLineString(nLevel, "<SkinnedMesh>");
        WriteObjectName(nLevel + 1, "<Name>: ", skinMeshRenderer);
        int nBonesPerVertex = (int)skinMeshRenderer.quality; //SkinQuality.Auto:0, SkinQuality.Bone1:1, SkinQuality.Bone2:2, SkinQuality.Bone4:4
        if (nBonesPerVertex == 0) nBonesPerVertex = 4;
        WriteMatrixes(nLevel+1, "<BoneOffsetMatrixes>: " + nBonesPerVertex, skinMeshRenderer.sharedMesh.bindposes);
        WriteBoneIndices(nLevel+1, "<BoneIndices>:", skinMeshRenderer.sharedMesh.boneWeights);
        WriteBoneWeights(nLevel+1, "<BoneWeights>:", skinMeshRenderer.sharedMesh.boneWeights);
        WriteLineString(nLevel, "</SkinnedMesh>");
    }

    void WriteFrameInfo(int nLevel, Transform current)
    {
        MeshFilter meshFilter = current.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = current.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            WriteMeshInfo(nLevel + 1, meshFilter.sharedMesh);
        }
        else
        {
            SkinnedMeshRenderer skinMeshRenderer = current.gameObject.GetComponent<SkinnedMeshRenderer>();
            if (skinMeshRenderer)
            {
                WriteSkinnedMeshInfo(nLevel + 1, skinMeshRenderer);
            }
        }
    }

    void Start()
    {
        m_rTextWriter = new StreamWriter(string.Copy(gameObject.name).Replace(" ", "_") + ".txt");

        WriteLineString("<Meshes>: " + meshObjects.Length);
        foreach (GameObject obj in meshObjects)
        {
            WriteFrameInfo(1, obj.transform);
        }
        WriteString("</Meshes>");

        m_rTextWriter.Flush();
        m_rTextWriter.Close();

    }
}

