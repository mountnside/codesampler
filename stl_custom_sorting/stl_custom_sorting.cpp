//-----------------------------------------------------------------------------
//           Name: stl_custom_sorting.cpp
//         Author: Kevin Harris
//                 Inspired by the research of Anton Ephanov (aephanov@smu.edu)
//  Last Modified: 03/10/05
//    Description: This sample demonstrates how to create a custom sorting 
//                 routine for STL’s std::sort function so you can use it to 
//                 sort objects created from your own class types.
//
//                 As a concrete example, we’ll create a simple record based 
//                 state management system, which is typically found in 3D 
//                 rendering systems. We then create a custom sorting routine 
//                 to use with STL so we can sort our Records by the Textures 
//                 they use. This will allow us to group together Records that 
//                 use the same Texture so we can save ourselves the overhead 
//                 associated with changing the current Texture object.
//-----------------------------------------------------------------------------

#include <windows.h>

#include <iostream>
using namespace std;

#include <vector>
#include <algorithm>
#include <functional>

//-----------------------------------------------------------------------------
// Create a Record based state management system, which is typical of modern 
// graphic engines. It declares a Record class, which contains a State 
// structure, which contains a Texture.
//-----------------------------------------------------------------------------

struct Texture
{
    Texture() {}
};

struct State
{
    State() : m_texture(NULL) {}
    Texture* getTexture() const { return m_texture; }
    void setTexture(Texture* t) { m_texture = t; }

private:

    Texture* m_texture;
};

struct Record
{
    State* getState() const { return m_state; }
    void setState(State* s) { m_state = s;  }

private:

    State* m_state;
};

//-----------------------------------------------------------------------------
// This is our custom STL based sort function which allows our Records to be 
// sorted by the Texture they use.
//-----------------------------------------------------------------------------
struct sortByTexture : public std::binary_function< Record*, Record*, bool > 
{
    bool operator()(Record* r1, Record* r2) const 
    {
        State* s1 = r1->getState();
        State* s2 = r2->getState();

        if( s1 == NULL || s2 == NULL )
            return s1 < s2;

        Texture* t1 = s1->getTexture();
        Texture* t2 = s2->getTexture();

        return ( t1 < t2 );
    }
};

//-----------------------------------------------------------------------------
// main entry....
//-----------------------------------------------------------------------------
void main()
{
    std::vector<Record*> records;

    //
    // Create three Texture objects, which can be shared amongst the Records.
    // Then, push_back 10 Records for each of the three textures and shuffle 
    // them up so they end up staggered in the vector.
    //

    Texture* tex1 = new Texture();
    Texture* tex2 = new Texture();
    Texture* tex3 = new Texture();

    int numRecords;

    for( numRecords = 0; numRecords < 10; ++numRecords )
    {
        State* state = new State();
        state->setTexture( tex1 );

        Record* rec = new Record();
        rec->setState( state );

        records.push_back( rec );
    }

    for( numRecords = 0; numRecords < 10; ++numRecords )
    {
        State* state = new State();
        state->setTexture( tex2 );

        Record* rec = new Record();
        rec->setState( state );

        records.push_back( rec );
    }

    for( numRecords = 0; numRecords < 10; ++numRecords )
    {
        State* state = new State();
        state->setTexture( tex3 );

        Record* rec = new Record();
        rec->setState( state );

        records.push_back( rec );
    }

    random_shuffle( records.begin(), records.end() );

    //
    // Before sorting...
    //

    cout << "-- Simulated rendering before sorting --" << endl << endl;

    std::vector<Record*>::iterator itv  = records.begin();
    std::vector<Record*>::iterator itve = records.end();
    Texture *lastTexture = NULL;

    for( ; itv != itve; ++itv )
    {
        cout << "Render geometry using texture " << (*itv)->getState()->getTexture();

        if( lastTexture != (*itv)->getState()->getTexture() )
            cout << " - Texture changed... MODIFY STATE!" << endl;
        else
            cout << " - Texture unchanged... do nothing." << endl;

        lastTexture = (*itv)->getState()->getTexture();
    }

    //
    // After sorting...
    //

    std::sort( records.begin(), records.end(), sortByTexture() );

    cout << endl;
    cout << "-- Simulated rendering after sorting --" << endl << endl;

    itv  = records.begin();
    itve = records.end();
    lastTexture = NULL;

    for( ; itv != itve; ++itv )
    {
        cout << "Render geometry using texture " << (*itv)->getState()->getTexture();

        if( lastTexture != (*itv)->getState()->getTexture() )
            cout << " - Texture changed... MODIFY STATE!" << endl;
        else
            cout << " - Texture unchanged... do nothing." << endl;

        lastTexture = (*itv)->getState()->getTexture();
    }

    //
    // Done... clean it all up.
    //

    itv  = records.begin();
    itve = records.end();

    for( ; itv != itve; ++itv )
    {
        delete (*itv)->getState();
        delete *itv;
    }

    delete tex1;
    delete tex2;
    delete tex3;

    records.clear();
}
