//-----------------------------------------------------------------------------
//           Name: dp_publisher_subscriber.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Sample source using the Publisher/Subscriber design pattern.
//
// The Publisher/Subscriber design pattern helps to keep the state of objects 
// synchronized through the one-way propagation of change notifications. 
// This normally entails several objects called "Subscribers" registering with 
// a central object called a "Publisher". When the state of the Publisher 
// changes, the Publisher calls a notification function on each of the 
// registered Subscribers. It's then the responsibility of the Subscriber to 
// examine the Publisher's new state and act accordingly.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <list>
using namespace std;

// Forward declaration for compiler
class Publisher;

//-----------------------------------------------------------------------------
// Any object that wishes to get notified when the state of another object 
// changes, will need to derive from the Subscriber class.
//-----------------------------------------------------------------------------
class Subscriber
{
public:

    Subscriber() {};
    virtual ~Subscriber() {};
    virtual void Update( Publisher *publisherThatChanged ) = 0;
};

//-----------------------------------------------------------------------------
// Any object that wishes to notify other object of when its state changes, 
// will need to derive from the Publisher class.
//-----------------------------------------------------------------------------
class Publisher 
{
public:

    Publisher() {};
    virtual ~Publisher() {};

    virtual void Attach( Subscriber *subscriber );
    virtual void Detach( Subscriber *subscriber );
    virtual void Notify();

private:

    typedef list<Subscriber*> SubscriberList;
    SubscriberList m_subscribers;
};

void Publisher::Attach( Subscriber *subscriber )
{
    m_subscribers.push_back(subscriber);
}

void Publisher::Detach( Subscriber *subscriber )
{
    m_subscribers.remove(subscriber);
}

void Publisher::Notify()
{
    SubscriberList::iterator it;

    for( it  = m_subscribers.begin(); it != m_subscribers.end(); ++it )
	{
        (*it)->Update(this);
    }
}

//-----------------------------------------------------------------------------
// To further demonstrate the Publisher/Subscriber Pattern, we will now create
// simple class called ClockTimer which will derive from the Publisher class.
//
// Because we have derived it from Publisher, other objects that want to 
// receive time updates concerning the current time can now subscribe to 
// ClockTimer for notification.
//-----------------------------------------------------------------------------
class ClockTimer : public Publisher
{
public:

    ClockTimer();
    ~ClockTimer() {};

    virtual int GetHour()    { return m_hour; };
    virtual int GetMinutes() { return m_minutes; };
    virtual int GetSeconds() { return m_seconds; };

    void Tick();

private:

    int m_hour;
    int m_minutes;
    int m_seconds;
};

ClockTimer::ClockTimer()
{
    m_hour    = 0;
    m_minutes = 0;
    m_seconds = 0;
};

void ClockTimer::Tick()
{
    // Update internal time-keeping state
    
    //m_hour    = calculate hour here ;
    //m_minutes = calculate minutes here ;
    //m_seconds = calculate seconds here ;

    Notify();
}

//-----------------------------------------------------------------------------
// We will now create a second class called MyClock which derives from the 
// Subscriber class so it may subscribe to the ClockTimer for updates.
//-----------------------------------------------------------------------------
class MyClock: public Subscriber 
{
public:

    MyClock( ClockTimer *publisher );
    virtual ~MyClock();

    virtual void Update( Publisher *publisherThatChanged );
    virtual void RefreshTime();

private:

    ClockTimer *m_publisher;
};

MyClock::MyClock( ClockTimer *publisher ) 
{
    m_publisher = publisher;
    m_publisher->Attach(this);
}

MyClock::~MyClock()
{
    m_publisher->Detach(this);
}

void MyClock::Update( Publisher *publisherThatChanged )
{
    if( publisherThatChanged == m_publisher )
    {
        RefreshTime();
    }
}

void MyClock::RefreshTime()
{
    // Get the new values from the Publisher
    int nHour    = m_publisher->GetHour();
    int nMinutes = m_publisher->GetMinutes();
    int nSeconds = m_publisher->GetSeconds();

    // Apply new time values to MyClock...
}

//-----------------------------------------------------------------------------
// Main entry point for the test application...
//-----------------------------------------------------------------------------
void main( void )
{
    ClockTimer *timer = new ClockTimer;
    MyClock    *clock = new MyClock(timer);
}
