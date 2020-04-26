#include "MyCamera.h"

using namespace iglu;

CMyCamera::CMyCamera( int width, int height )
:IGLUMouseInteractor(width, height),
m_movingSpeed(1.0)
{
    Reset();
    //Default keys binding
    m_keysBinding[0] = 'w';
    m_keysBinding[1] = 's';
    m_keysBinding[2] = 'a';
    m_keysBinding[3] = 'd';
    m_keysBinding[4] = 'f';
    m_keysBinding[5] = 'v';
    m_cameraMove[0] = &CMyCamera::Forward;
    m_cameraMove[1] = &CMyCamera::Backward;
    m_cameraMove[2] = &CMyCamera::StrafeLeft;
    m_cameraMove[3] = &CMyCamera::StrafeRight;
    m_cameraMove[4] = &CMyCamera::Up;
    m_cameraMove[5] = &CMyCamera::Down;

}

void CMyCamera::SetOnClick( int x, int y )
{
    m_currentlyTracking = 1;
    m_lastScreenPos = int2(x,y);
}

void CMyCamera::UpdateOnMotion( int x, int y )
{
    if (!m_currentlyTracking)
    {
        return;
    }

    float yaw = float(m_lastScreenPos.X()- x) / m_width;
    float pitch = float( m_lastScreenPos.Y() - y) / m_height;

    Yaw(yaw*100);
    Pitch(pitch*100);

    ComputeViewMatrix();

    m_lastScreenPos = int2(x, y);
}

void CMyCamera::Release( void )
{
    m_currentlyTracking = 0; 
}

void CMyCamera::Reset( void )
{
    m_currentlyTracking = 0;
    m_matrix = IGLUMatrix4x4::Identity();
    m_inverseMatrix = IGLUMatrix4x4::Identity();
    m_hasChanged = 1;
}

void CMyCamera::UpdateOnKeys( unsigned int key, int x, int y )
{
    for (uint i = 0; i < CAMERA_KEY_NUM; ++ i )
    {
        if (key == m_keysBinding[i] || key == GetEquivalentKeys(m_keysBinding[i]))
        {
            (this->*m_cameraMove[i])();
        }
    }

    ComputeViewMatrix();
}

void CMyCamera::SetView( const iglu::vec3 &eye, const iglu::vec3 &at, const iglu::vec3 &up )
{
    //Up will remain unchanged in all the movement of camera
    m_up = up;
    //Forward is the direction for camera
    m_forward = at - eye;
    //right is the direction for strafe
    m_right = m_forward.Cross(m_up);
    //Eye position
    m_position = eye;

    ComputeViewMatrix();
}

void CMyCamera::SetView( const iglu::IGLUMatrix4x4 & viewMat, const iglu::vec3 &up)
{
    // We basically use vec*mat to get the value rather than taking
    //   the row or column of the matrix
    // Get position
    vec4 temp = vec4(0,0,0,1);
    temp = viewMat * temp;
    m_position = vec3(temp.X(), temp.Y(), temp.Z());

    // Get forward direction
    temp = vec4(0,0,1,0);
    temp = viewMat * temp;
    m_forward = vec3(temp.X(), temp.Y(), temp.Z());

    // Get right direction
    m_right = m_forward.Cross(m_up);

    ComputeViewMatrix();
}

void CMyCamera::ComputeViewMatrix()
{
    m_matrix = IGLUMatrix4x4::LookAt(m_position, m_position + m_forward, m_up);
}

void CMyCamera::Forward()
{
    m_position = m_position + m_movingSpeed * m_forward;
}

void CMyCamera::Backward()
{
    m_position = m_position - m_movingSpeed * m_forward;
}

void CMyCamera::StrafeLeft()
{
    m_position = m_position - m_movingSpeed * m_right;
}

void CMyCamera::StrafeRight()
{
    m_position = m_position + m_movingSpeed * m_right;
}

void CMyCamera::Pitch( float pitch )
{
    //Update forward
    IGLUMatrix4x4 rotate = IGLUMatrix4x4::Rotate(pitch, m_right);
    vec4 temp( m_forward.X(), m_forward.Y(), m_forward.Z(), 0);
    temp = rotate * temp;
    m_forward = vec3(temp.X(), temp.Y(), temp.Z());

    //Update right
    //m_right = m_forward.Cross(m_up);

}

void CMyCamera::Yaw( float yaw )
{
    //Update forward
    IGLUMatrix4x4 rotate = IGLUMatrix4x4::Rotate(yaw, m_up);
    vec4 temp( m_forward.X(), m_forward.Y(), m_forward.Z(), 0);
    temp = rotate * temp;
    m_forward = vec3(temp.X(), temp.Y(), temp.Z());

    //Update right
    m_right = m_forward.Cross(m_up);
}

void CMyCamera::Up()
{
     m_position = m_position + m_movingSpeed * m_up;
}

void CMyCamera::Down()
{
    m_position = m_position - m_movingSpeed * m_up;
}

char CMyCamera::GetEquivalentKeys( char c )
{
    if ( c > 'a' && c < 'z')
    {
        return toupper(c);
    }
    else if ( c > 'A' && c < 'Z')
    {
        return tolower(c);
    }
    else
    {
        return c;
    }
}

void CMyCamera::SetKeys( char keys[CAMERA_KEY_NUM] )
{
    memcpy(m_keysBinding, keys, sizeof(m_keysBinding));
}

