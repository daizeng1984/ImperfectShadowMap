#ifndef __MyCamera_h__
#define __MyCamera_h__

// All headers are automatically included from "iglu.h"
#include "iglu.h"

class CMyCamera : public iglu::IGLUMouseInteractor
{
    enum{CAMERA_KEY_NUM = 6};
public:

    /* sets up a trackball with window size of width x height */
    CMyCamera( int width, int height );
    virtual ~CMyCamera()                                              {}

    // Functions for updating the interactor due to mouse input.           
    //    x & y are the window coordinates of the current mouse position. 
    virtual void SetOnClick( int x, int y );
    virtual void UpdateOnMotion( int x, int y );
    virtual void Release( void );

    // Update according to keyboard input, x, y are also windows coordinates
    void UpdateOnKeys(unsigned int key, int x, int y);

    // What to do when the interactor is reset
    virtual void Reset( void );

    // Set View just like IGLUMatrix4x4::LookAt( const vec3 &eye, const vec3 &at, const vec3 &up )
    //    To use this camera you should at least set this as the initial camera position.
    void SetView(const iglu::vec3 &eye, const iglu::vec3 &at, const iglu::vec3 &up);
    void SetView(const iglu::IGLUMatrix4x4 & viewMat, const iglu::vec3 &up);

    // Get frame vector
    const iglu::vec3& GetForward(){return m_forward;}
    const iglu::vec3& GetPosition(){return m_position;}
    const iglu::vec3& GetRight(){return m_right;}


    // Set speed
    void SetSpeed(float speed){m_movingSpeed = speed;}

    // Set key binding, require 6 keys in order of 'Forward' 'Backward' 'Left' 'Right' 'Up' 'Down'
    void SetKeys( char keys[CAMERA_KEY_NUM]); 

    // A pointer to a IGLUTrackball could have type IGLUTrackball::Ptr
    typedef CMyCamera *Ptr;

private:

    //Compute view matrix
    void ComputeViewMatrix();
    //e.g. if input 'a' you'll get 'A', input 'A' you get 'a', for '+' you only still get '+'
    char GetEquivalentKeys(char c);


    //Movement
    void Forward();
    void Backward();
    void StrafeLeft();
    void Up();
    void Down();
    void StrafeRight();
    void Pitch(float pitch);
    void Yaw(float yaw);
    void Roll(); //no roll

private:
    typedef void (CMyCamera::*CameraMovement) ();
    //local frame
    iglu::vec3 m_up;
    iglu::vec3 m_forward;
    iglu::vec3 m_right;
    iglu::vec3 m_position;
    iglu::int2 m_lastScreenPos;
    float m_movingSpeed;

    //TODO: more sophisticated one might be key mapping
    char m_keysBinding[CAMERA_KEY_NUM];
    CameraMovement m_cameraMove[CAMERA_KEY_NUM];
};




#endif // __MyCamera_h__
