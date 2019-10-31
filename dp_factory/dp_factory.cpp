//-----------------------------------------------------------------------------
//           Name: dp_factory.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Sample source using the "Factory" design pattern.
//
// A Factory defines an interface for creating an object, but lets subclasses 
// decide which class to instantiate. The Factory lets a class defer 
// instantiation to subclasses.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

typedef int ProductId;

const int MINE   = 1;
const int YOURS  = 2;
const int THEIRS = 3;

//-----------------------------------------------------------------------------
// The Product class is the abstract class used by all objects that our 
// factory will be responsible for creating, or manufacturing. 
//
// This is all our factory really knows about the products it's creating
//-----------------------------------------------------------------------------
class Product
{
public:
    
    virtual void printBarCode() = 0;
};

//-----------------------------------------------------------------------------
// Now, as an example, lets create three sample products for our factory to
// create.
//-----------------------------------------------------------------------------
class MyProduct : public Product
{
public:
    
    virtual void printBarCode() { cout << "Bar Code: 00000 00001" << endl; };
};

class YourProduct : public Product
{
public:
    
    virtual void printBarCode() { cout << "Bar Code: 00000 00002" << endl; };
};

class TheirProduct : public Product
{
public:
    
    virtual void printBarCode() { cout << "Bar Code: 00000 00003" << endl; };
};

//-----------------------------------------------------------------------------
// The factory method Create(), which is a member of our Creator class, helps 
// instantiate the appropriate subclass by creating the 
// right object from a group of related classes. In our example, our subclasses
// are related through the abstract class, Product.
//
// Note that the Create() method is an example of a parameterized factory 
// method because it requires an identifier to assist in the production of the 
// correct object.
//-----------------------------------------------------------------------------
class Creator
{
public:

    virtual Product* Creator::Create( ProductId id )
    {
        if( id == MINE )  return new MyProduct;
        if( id == YOURS ) return new YourProduct;

        return 0;
    }
};

//-----------------------------------------------------------------------------
// Overriding a parameterized factory method lets you easily and selectively 
// extend or change the products that a Creator produces. You can introduce 
// new identifiers for new kinds of products, or you can associate existing 
// identifiers with different products. 
//
// For example, a subclass MyCreator could swap MyProduct and YourProduct and 
// support a new TheirProduct subclass: 
//-----------------------------------------------------------------------------
class MyCreator : public Creator
{
public:

    virtual Product* MyCreator::Create( ProductId id )
    {
        if( id == YOURS  ) return new MyProduct;    // Switch YOURS for MINE
        if( id == MINE   ) return new YourProduct;  // Switch MINE for YOURS
        if( id == THEIRS ) return new TheirProduct; // New product Added

        return Creator::Create(id); // Call to base, if the product ID 
                                    // can't be found here...
    }
};

//-----------------------------------------------------------------------------
// Main entry point for the test application...
//-----------------------------------------------------------------------------
void main( void )
{
    MyCreator mCreator;

    Product *mProd = mCreator.Create( MINE );
    Product *yProd = mCreator.Create( YOURS );
    Product *tProd = mCreator.Create( THEIRS );

    mProd->printBarCode();
    yProd->printBarCode();
    tProd->printBarCode();
}
