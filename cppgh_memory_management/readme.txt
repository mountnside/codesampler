This sample was written and submitted to CodeSampler.com by Anton Ephanov who 
teaches at the Guildhall at SMU.

http://guildhall.smu.edu/about/faculty.htm

--------------------------------------------------------------------------------

 -- About the Guildhall --

The mission of The Guildhall at SMU is to provide the world's premier education 
and training center for digital game development, and to serve as a focal point 
for the digital game community.

The Guildhall is a place, a faculty, a curriculum, a student body, and a 
vision—shared by experienced game developers, industry leaders, academics, 
and aspiring professionals.

Learn more about the Guildhall at "guildhall.smu.edu".

--------------------------------------------------------------------------------

This sample demonstrates numerous advanced C++ techniques concerning memory 
management. The sample is composed of 7 units, which are described below:

Unit 1: This unit demonstrates how to properly allocate and destroy arrays of 
C++ objects.

Unit 2: This unit demonstrates usage of the placement new operator. This 
technique can be used for avoiding unnecessary dynamic memory allocations and 
implementing memory pools.

Unit 3: The unit demonstrates how to write operators new/delete for a user 
class. This technique can be used in conjunction with other optimization 
techniques such as:
	- optimizing performance by eliminating bottlenecks associated with 
	  dynamic memory allocations.
	- implementing custom allocators for reference counting

Unit 4: This unit shows how to write a custom STL compatible allocator. 
The code includes two PERFORMANCE WARNINGS that are specific to VC++'s 
implementation of STL.

Unit 5: The unit demonstrates one of many numerous ways of implementing 
reference counting. Here, we use a common base class. You can also use a 
so-called in-band memory header paired with an allocator.

Unit 6 : The unit demonstrates how the "smart pointer" idiom can be used 
together with reference counting. Smart pointer and reference counting seemed 
to be made for each other!

Unit 7: The unit demonstrates a simple allocator implementation that allows 
for memory tracking. This infrastructure can be extended to support memory 
management in a large scale C++ framework via classes of allocators. The basic 
idea is to introduce a user allocator (Base::Allocator) that includes an API 
to install a user callback. Next, right a class that provides the callback 
and tracks the memory. This technique works well with approach from unit3, 
where operator new/delete are implemented for the base class.

--------------------------------------------------------------------------------