using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SampleRotater : MonoBehaviour {

    public Vector3 EulerPerSecond;


	// Update is called once per frame
	void Update () {
        transform.Rotate(EulerPerSecond * Time.deltaTime, Space.Self);
	}
}
