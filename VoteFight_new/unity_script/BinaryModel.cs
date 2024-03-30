using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using UnityEditor;
using System.Text;
using UnityEngine.Rendering;

public class BinaryModel : MonoBehaviour
{
    public int classType;
    public bool bAnimation = false;

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
    void WriteTextureName(Texture texture)
    {
        binaryWriter.Write(string.Copy(texture.name).Replace(" ", "_"));
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
        if (vectors.Length > 0) foreach (Vector3 v in vectors) WriteVector(v);
    }

    void WriteBoneNames(string strHeader, Transform[] bones)
    {
        WriteString(strHeader, bones.Length);
        if (bones.Length > 0)
        {
            foreach (Transform bone in bones) WriteObjectName(bone.gameObject);
            //WriteString("<BoneHierarchy>:");
            //WriteBoneNameHierarchy(bones[0]);
            //WriteString("</BoneHierarchy>:");
        }
    }

    void WriteBoneNameHierarchy(Transform parant)
    {
        WriteObjectName(parant);
        WriteInteger(parant.childCount);
        for (int k = 0; k < parant.childCount; k++) WriteObjectName(parant.GetChild(k));
        for (int k = 0; k < parant.childCount; k++) WriteBoneNameHierarchy(parant.GetChild(k));
    }

    void WriteMaterials(Material[] materials)
    {
        WriteInteger("<Materials>", materials.Length);
        WriteString("<Material>");
        foreach(Material material in materials)
        {
            WriteObjectName(material);
        }
    }

    void WriteFrameInfo(Transform current)
    {
        WriteInteger("<ClassType>", classType);
        WriteObjectName("<Name>", current);
        if(current.gameObject.activeSelf)
            WriteInteger("<IsActive>", 1);
        else
            WriteInteger("<IsActive>", 0);
        Vector3[] v = new Vector3[3];
        v[0] = current.localPosition;
        v[1] = current.localRotation.eulerAngles;
        v[2] = current.localScale;
        WriteVectors("<Transform>", v);
        MeshFilter meshFilter = current.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = current.GetComponent<MeshRenderer>();
        Mesh mesh = null;
        Material[] materials = null;
        if (meshFilter && meshRenderer)
        {
            mesh = meshFilter.mesh;
            materials = meshRenderer.materials;
        }
        else
        {
            SkinnedMeshRenderer skinMeshRenderer = current.GetComponent<SkinnedMeshRenderer>();
            if (skinMeshRenderer)
            {
                mesh = skinMeshRenderer.sharedMesh;
                materials = skinMeshRenderer.materials;
            }
        }
        if(mesh) 
            WriteObjectName("<Mesh>", mesh); 
        else
        {
            WriteString("<Mesh>");
            WriteString("Null");
        }
        if (materials != null && materials.Length > 0) 
            WriteMaterials(materials);
        else
        {
            WriteInteger("<Materials>", 0);
            WriteString("<Material>");
        }

        BoxCollider collider = current.GetComponent<BoxCollider>();
        if (collider)
        {
            Vector3[] bounds = new Vector3[2];
            bounds[0] = new Vector3(collider.bounds.center.x, collider.bounds.center.y, collider.bounds.center.z);
            bounds[1] = new Vector3(collider.bounds.extents.x, collider.bounds.extents.y, collider.bounds.extents.z);
            print(bounds[0]);
            print(bounds[1]);
            WriteVectors("<Collider>", bounds);
        }
        WriteInteger("<ChildCount>", current.childCount);
        if (current.childCount > 0)
        {
            for (int k = 0; k < current.childCount; k++) WriteFrameInfo(current.GetChild(k));
        }
        WriteString("</Frame>");
    }


    void WriteAnimationClipsInfo()
    {
        WriteObjectName("<FileName>", gameObject);
        WriteInteger("<SkinnedMeshes>", 1);
        SkinnedMeshRenderer skinnedMeshRenderer = GetComponent<SkinnedMeshRenderer>();
        if (skinnedMeshRenderer == null)
            skinnedMeshRenderer = transform.GetChild(0).GetComponent<SkinnedMeshRenderer>();
        WriteObjectName("<Name>", skinnedMeshRenderer.sharedMesh);
        WriteBoneNames("<Bones>", skinnedMeshRenderer.bones);
    }

    void Start()
    {
        binaryWriter = new BinaryWriter(File.Open(string.Copy(gameObject.name).Replace(" ", "_") + ".bin", FileMode.Create));

		WriteString("<Frames>");
        WriteFrameInfo(transform);
        WriteString("</Frames>");

        if (bAnimation == true)
        {
    		WriteString("<Animator>");
            WriteAnimationClipsInfo();
    		WriteString("</Animator>");
        }

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Model Binary Write Completed");
    }
}

