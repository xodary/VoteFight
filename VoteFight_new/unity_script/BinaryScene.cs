using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;

public class BinaryScene : MonoBehaviour
{
    public int[] groupType;

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

    void WriteIntegers(string strHeader, int n, int[] pIntegers)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(n);
        binaryWriter.Write(pIntegers.Length);
        if (pIntegers.Length > 0) foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteBoundingBox(string strHeader, Bounds bounds)
    {
        binaryWriter.Write(strHeader);
        WriteVector(bounds.center);
        WriteVector(bounds.extents);
    }

    void WriteMatrix(Matrix4x4 matrix)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }

    void WriteMatrix(Vector3 position, Quaternion rotation, Vector3 scale)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(position, rotation, scale);
        WriteMatrix(matrix);
    }

    void WriteTransform(string strHeader, Transform current)
    {
        binaryWriter.Write(strHeader);
        WriteVector(current.localPosition);
        WriteVector(current.localEulerAngles);
        WriteVector(current.localScale);
        WriteVector(current.localRotation);
    }

    void WriteLocalMatrix(string strHeader, Transform current)
    {
        binaryWriter.Write(strHeader);
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);
        WriteMatrix(matrix);
    }

    void WriteWorldMatrix(string strHeader, Transform current)
    {
        binaryWriter.Write(strHeader);
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.position, current.rotation, current.lossyScale);
        WriteMatrix(matrix);
    }

    void WriteBoneTransforms(string strHeader, Transform[] bones)
    {
        WriteString(strHeader, bones.Length);
        if (bones.Length > 0)
        {
            foreach (Transform bone in bones)
            {
                WriteMatrix(bone.localPosition, bone.localRotation, bone.localScale);
            }
        }
    }

    void WriteBoneNames(string strHeader, Transform[] bones)
    {
        WriteString(strHeader, bones.Length);
        if (bones.Length > 0)
        {
            foreach (Transform bone in bones) WriteObjectName(bone.gameObject);
            WriteString("<BoneHierarchy>:");
            WriteBoneNameHierarchy(bones[0]);
            WriteString("</BoneHierarchy>:");
        }
    }

    void WriteBoneNameHierarchy(Transform parant)
    {
        WriteObjectName(parant);
        WriteInteger(parant.childCount);
        for (int k = 0; k < parant.childCount; k++) WriteObjectName(parant.GetChild(k));
        for (int k = 0; k < parant.childCount; k++) WriteBoneNameHierarchy(parant.GetChild(k));
    }

    void WriteMatrixes(string strHeader, Matrix4x4[] matrixes)
    {
        WriteString(strHeader, matrixes.Length);
        if (matrixes.Length > 0)
        {
            foreach (Matrix4x4 matrix in matrixes) WriteMatrix(matrix);
        }
    }

    void WriteBoneIndex(BoneWeight boneWeight)
    {
        binaryWriter.Write(boneWeight.boneIndex0);
        binaryWriter.Write(boneWeight.boneIndex1);
        binaryWriter.Write(boneWeight.boneIndex2);
        binaryWriter.Write(boneWeight.boneIndex3);
    }

    void WriteBoneWeight(BoneWeight boneWeight)
    {
        binaryWriter.Write(boneWeight.weight0);
        binaryWriter.Write(boneWeight.weight1);
        binaryWriter.Write(boneWeight.weight2);
        binaryWriter.Write(boneWeight.weight3);
    }

    void WriteBoneIndices(string strHeader, BoneWeight[] boneWeights)
    {
        WriteString(strHeader, boneWeights.Length);
        if (boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in boneWeights) WriteBoneIndex(boneWeight);
        }
    }

    void WriteBoneWeights(string strHeader, BoneWeight[] boneWeights)
    {
        WriteString(strHeader, boneWeights.Length);
        if (boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in boneWeights) WriteBoneWeight(boneWeight);
        }
    }

    void WriteFrameHierarchyInfo(Transform child)
    {
        WriteFrameInfo(child);

        WriteInteger("<Children>:", child.childCount);

        if (child.childCount > 0)
        {
            for (int k = 0; k < child.childCount; k++)
            {
                WriteFrameHierarchyInfo(child.GetChild(k));
            }
        }

        WriteString("</Frame>");
    }

    void WriteAnimationPositionHierarchy(Transform current)
    {
        WriteVector(current.localPosition);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteAnimationPositionHierarchy(current.GetChild(k));
        }
    }

    void WriteAnimationRotationHierarchy(Transform current)
    {
        WriteVector(current.localRotation);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteAnimationRotationHierarchy(current.GetChild(k));
        }
    }

    void WriteAnimationScaleHierarchy(Transform current)
    {
        WriteVector(current.localScale);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteAnimationScaleHierarchy(current.GetChild(k));
        }
    }

    void WriteAnimationMatrixHierarchy(Transform current)
    {
        WriteMatrix(current.localPosition, current.localRotation, current.localScale);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteAnimationMatrixHierarchy(current.GetChild(k));
        }
    }

    void WriteAnimationTransforms(string strHeader, int nKeyFrame, float fKeyFrameTime)
    {
        WriteString(strHeader, nKeyFrame, fKeyFrameTime);
        WriteAnimationMatrixHierarchy(transform);

        //WriteAnimationScaleHierarchy(transform);
        //WriteAnimationRotationHierarchy(transform);
        //WriteAnimationPositionHierarchy(transform);
    }

    void WriteFrameNameHierarchy(Transform current)
    {
        WriteObjectName(current.gameObject);

        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameNameHierarchy(current.GetChild(k));
        }
    }

    void WriteFrameNames(string strHeader)
    {
        WriteString(strHeader, m_nFrames);
        WriteFrameNameHierarchy(transform);
    }

    void WriteScene(Transform model, int index)
    {
        WriteString("<GroupType> " + groupType[index]);
        WriteObjectName("<FileName>", model.gameObject);
        WriteString("<Instance> " + model.childCount);
        WriteString("<IsActive> ");
        for (int i = 0; i < model.childCount; ++i)
        {
            if (model.GetChild(i).gameObject.activeSelf)
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
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name).Replace(" ", "_") + ".bin", FileMode.Create));
        
        for (int i = 0; i < transform.childCount; ++i)
        {
            Transform child = transform.GetChild(i);
            WriteScene(child, i);
        }

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

