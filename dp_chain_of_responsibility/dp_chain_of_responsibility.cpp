//-----------------------------------------------------------------------------
//           Name: dp_chain_of_responsibility.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Sample source using the "Chain of Responsibility" design 
//                 pattern.
//
// The Chain of Responsibility can be represented by a mechanical coin sorting 
// bank. The idea behind this machine is a coin is deposited into the bank 
// and it goes to the first hole, if it fits, it drops into the slot. If it 
// doesn’t fit, then it moves on to the next slot and gets evaluated the same 
// way. This is very similar to the Chain of Responsibility. If it isn’t the 
// right slot, the coin gets forwarded to its successor handler. So instead of 
// having individual slots to fit all the coins, which is high coupling, it 
// has one receptacle for all the coins. Because of this reason, receptacle or 
// chain of receptacles can be removed or modified without huge conflicts. 
// This has the same drawbacks. If an invalid coin (Canadian Coin) is inserted, 
// then the coin would get stuck with no where to go, whereas if there was one 
// huge slot, the coin would get handled but must be discarded later.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------

// Event IDs
const int EVENT_PENNY            =  1;
const int EVENT_NICKEL           =  5;
const int EVENT_DIME             = 10;
const int EVENT_QUARTER          = 25;
const int EVENT_CANADIAN_QUARTER =  0;

//-----------------------------------------------------------------------------
// A base class with a simple "Chain of Responsibility" layout.
//-----------------------------------------------------------------------------
class Handler
{
public:

    Handler( Handler *successor, int event ) 
    { 
        m_successor = successor;
        m_event     = event;
    }

    virtual void Handler::handleEvent( int event )
    {
        if( m_successor != NULL )
        {
            m_successor->handleEvent( event );
        }
        else
        {
            cout << event << " - Event failed to get handled" << endl;
            cout << endl;
        }
    }

    virtual int  getEvent( void ) { return m_event; }

private:

    Handler *m_successor;
    int      m_event;
};


//-----------------------------------------------------------------------------
// We'll now derive from the base class event Handler and add the functionality
// of a coin Handler.
//-----------------------------------------------------------------------------
class CoinHandler : public Handler
{
public:

    CoinHandler::CoinHandler( CoinHandler *ch, int event )
        : Handler( ch, event )
    {
        m_coinCount = 0;
    }

    virtual void handleEvent( int event );
    virtual int  getCoinCount( void ) { return m_coinCount; }

private:
    
    int m_coinCount;
};


void CoinHandler::handleEvent( int event )
{
    if( event == getEvent() )
    {
        cout << event << " - Event Accepted" << endl;
        cout << endl;

        ++m_coinCount;
    } 
    else
    {
        cout << event << " - Event not handled, forward along chain..." << endl;

        Handler::handleEvent( event );
    }
}


//-----------------------------------------------------------------------------
// Main entry point for the test application...
//-----------------------------------------------------------------------------
void main()
{
    CoinHandler *pennySlot   = new CoinHandler( NULL,       EVENT_PENNY   );
    CoinHandler *nickelSlot  = new CoinHandler( pennySlot,  EVENT_NICKEL  );
    CoinHandler *dimeSlot    = new CoinHandler( nickelSlot, EVENT_DIME    );
    CoinHandler *quarterSlot = new CoinHandler( dimeSlot,   EVENT_QUARTER );

    //
    // Because quarterSlot is at the head of the chain, we can pass it any
    // of the events and rest assured that eventually one of the objects in 
    // the chain will handle it.
    //

    quarterSlot->handleEvent( EVENT_PENNY );
    quarterSlot->handleEvent( EVENT_NICKEL );
    quarterSlot->handleEvent( EVENT_DIME );
    quarterSlot->handleEvent( EVENT_QUARTER );
    quarterSlot->handleEvent( EVENT_CANADIAN_QUARTER );
 
    cout << "Penny Count   = " << pennySlot->getCoinCount()   << endl;
    cout << "Nickel Count  = " << nickelSlot->getCoinCount()  << endl;
    cout << "Dime Count    = " << dimeSlot->getCoinCount()    << endl;
    cout << "Quarter Count = " << quarterSlot->getCoinCount() << endl;
}
