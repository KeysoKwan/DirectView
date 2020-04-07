using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[ExecuteInEditMode]
public class Preview3D : MonoBehaviour
{

    static bool preview3D = false;
#if UNITY_EDITOR
    [MenuItem("GCSeries/3DPreview")]
    static void ExportGCSeriesPackage()
    {
        preview3D = !preview3D;
        if (preview3D)
        {
            UnityEngine.Debug.Log("Creating 3d preview...");
            GCSeries.Direct3DWin temp_d3dwin = FindObjectOfType<GCSeries.Direct3DWin>();
            if (temp_d3dwin != null)
            {
                Preview3D refp = temp_d3dwin.gameObject.AddComponent<Preview3D>();
                refp.previewCams = temp_d3dwin.gameObject.GetComponentsInChildren<Camera>();
            }
        }
        else
        {
            UnityEngine.Debug.Log("Closing 3d preview...");
        }
    }
#endif
    public Camera[] previewCams;
    GCSeries.Direct3DWin m_d3dwin;
    private bool isPreviewing = false;
    // Use this for initialization
    void Awake()
    {
#if UNITY_EDITOR
        StartCoroutine(EndOfFrame());
#endif
    }


    IEnumerator EndOfFrame()
    {
        m_d3dwin = FindObjectOfType<GCSeries.Direct3DWin>();
        isPreviewing = false;
        while (m_d3dwin != null)
        {
            yield return new WaitForEndOfFrame();
            if (preview3D && !isPreviewing)
            {
                m_d3dwin.FARPreview();
                isPreviewing = true;
            }
            else if (!preview3D && isPreviewing)
            {
                GCSeries.FARDll.CloseDown();
                isPreviewing = false;
                Preview3D temp_Preview3D = FindObjectOfType<Preview3D>();
                if (temp_Preview3D != null)
                {
                    DestroyImmediate(temp_Preview3D);
                }
            }
            else if (preview3D && isPreviewing)
            {
                foreach (Camera a in previewCams)
                {
                    if (a.enabled)
                        a.Render();
                }
            }

        }
        yield break;
    }
}
