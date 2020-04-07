using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TextureDrawer : MonoBehaviour
{
    public RenderTexture[] renderTextures;
    private void OnGUI()
    {
        GUI.DrawTexture(new Rect(0, 0, Screen.width/2, Screen.height), renderTextures[0]);
        GUI.DrawTexture(new Rect(Screen.width / 2,0, Screen.width / 2, Screen.height), renderTextures[1]);
        //for (int i = 0; i < renderTextures.Length; i++)
        //{
        //    GUI.DrawTexture(new Rect(0, 0, Screen.width, Screen.height), renderTextures[i]);
        //}
    }
}
