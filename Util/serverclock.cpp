#include "serverclock.h"

ServerClock::ServerClock(unsigned int rate) : m_cTime(), m_pTime(), m_uDelta(0), m_Frames(0), m_Rate(rate)
{

}

void ServerClock::Start()
{
#ifdef _WIN32
    int CLOCK_REALTIME = -1;
#endif
    clock_gettime(CLOCK_REALTIME, &m_pTime);
    m_Frames = 0;
}

bool ServerClock::InFrame()
{
#ifdef _WIN32
    int CLOCK_REALTIME = -1;
#endif

    // Update the current frame time
    clock_gettime(CLOCK_REALTIME, &m_cTime);

    // Calculate delta - negate seconds / convert to nano, combine with active nano and calc delta
    long long secondsDelta = (m_cTime.tv_sec-m_pTime.tv_sec)*1000*1000*1000;
    long long nanoDelta = m_cTime.tv_nsec - m_pTime.tv_nsec;
    m_pTime = m_cTime;
    m_FrameDelta = secondsDelta + nanoDelta;
    m_uDelta += m_FrameDelta;

    // Calc Milliseconds
    long micro = m_uDelta / 1000;
    long mili = micro / 1000;

    // Update frames every second
    if (mili >= 1000) {
        //printf("Ticks per second: %d\n", m_Frames);
        m_uDelta = 0;
        m_Frames = 0;
    }

    if (m_Frames < m_Rate) {
        return true;
    } else {
        return false;
    }
}

void ServerClock::Frame()
{
    // Increment frame counter
    m_Frames++;

}

void ServerClock::SetRate(unsigned int rate)
{
    m_Rate = rate;
}
