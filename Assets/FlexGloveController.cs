using System;
using System.Collections;
using System.Collections.Generic;
using BNG;
using Uduino;
using Unity.VisualScripting;
using UnityEngine;

public class FlexGloveController : MonoBehaviour
{
    public List<float> fingers = new List<float> { 0.0f, 0.0f, 0.0f, 0.0f };
    public List<float> gyro = new List<float> { 0.0f, 0.0f, 0.0f };
    public List<float> accelerometer = new List<float> { 0.0f, 0.0f, 0.0f };

    public HandPose openHandPose;
    public HandPose closedHandPose;

    [HideInInspector]
    public HandPoser handPoser;
    public HandPoseBlender blender;

    private bool calibrating = false;
    private float deltaT = 0.0f;
    private float alpha = 0.9f;

    // Start is called before the first frame update
    void Start()
    {
        handPoser = GetComponent<HandPoser>();
        blender = GetComponent<HandPoseBlender>();

        handPoser.CurrentPose = closedHandPose;
        UduinoManager.Instance.OnDataReceived += OnDataReceived;
    }
    private void OnDataReceived(string data, UduinoDevice device)
    {
        string[] split = data.Split(' ');
        string[] values = new string[split.Length - 1];
        Array.Copy(split, 1, values, 0, split.Length - 1);

        switch (split[0])
        {
            case "hand":
                ProcessFingerData(values);
                break;
            case "orientation":
                ProcessImuData(values);
                break;
        }
    }

    private void ProcessFingerData(string[] fingerData)
    {
        float thumb = 0.0f;

        for (int i = 0; i < fingerData.Length; i++)
        {
            Debug.Log(((Finger)i).ToString() + ": " + fingerData[i]);
            fingers[i] = float.Parse(fingerData[i]);

            thumb += fingers[i];
        }

        thumb /= 4.0f;

        blender.IndexValue = fingers[(int)Finger.Index];
        blender.MiddleValue = fingers[(int)Finger.Middle];
        blender.RingValue = fingers[(int)Finger.Ring];
        blender.PinkyValue = fingers[(int)Finger.Pinky];
        blender.ThumbValue = thumb;
    }

    private void ProcessImuData(string[] imuData)
    {

        gyro[0] = float.Parse(imuData[0]);
        gyro[1] = float.Parse(imuData[1]);
        gyro[2] = float.Parse(imuData[2]);
        
        accelerometer[0] = float.Parse(imuData[3]);
        accelerometer[1] = float.Parse(imuData[4]);
        accelerometer[2] = float.Parse(imuData[5]);

        Quaternion q = transform.rotation;

        Vector3 gyroVector = new Vector3(-gyro[1], -gyro[2], gyro[0]);
        
        float dTheta = deltaT * gyroVector.magnitude * 180f / (float) Math.PI;
        Quaternion deltaQ = Quaternion.AngleAxis(dTheta, gyroVector.normalized);

        q *= deltaQ;
        
        // Quaternion accQ = new Quaternion(-accelerometer[1], -accelerometer[2], accelerometer[0], 0);
        // accQ = q * accQ * Quaternion.Inverse(q);
        // accQ.Normalize();
        //
        // float phi = (float) (Math.Acos(accQ.y) * 180 / Math.PI);
        // Vector3 n = new Vector3(-accQ.z, 0, accQ.x);
        // n.Normalize();
        //
        // Quaternion qTilt = Quaternion.AngleAxis((1f - alpha) * phi, n);
        // qTilt.Normalize();
        //
        // q *= qTilt;
        q.Normalize();

        transform.rotation = q;

        deltaT = 0f;
    }

    // Update is called once per frame
    void Update()
    {
        if (!calibrating)
        {
            UduinoManager.Instance.Read("getFingers");
            UduinoManager.Instance.Read("getOrientation");
            deltaT += Time.deltaTime;
        }

        if (Input.GetKeyDown(KeyCode.Space))
        {
            UduinoManager.Instance.sendCommand(calibrating ? "endCalibrate" : "startCalibrate");
            calibrating = !calibrating;
        } else if (Input.GetKeyDown(KeyCode.Return))
        {
            transform.rotation = Quaternion.identity;
        }
    }
    
    public enum Finger
    {
        Index = 0,
        Middle,
        Ring,
        Pinky
    }
    
    
}
