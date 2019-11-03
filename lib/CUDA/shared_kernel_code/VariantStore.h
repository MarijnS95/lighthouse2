#pragma once

#include <assert.h>

template <typename BaseType, typename Storage>
class other_type_iterator
{
	Storage ptr;

  public:
	typedef BaseType value_type;
	typedef size_t difference_type;
	typedef value_type& reference;
	typedef value_type* pointer;

	__device__ other_type_iterator( Storage ptr ) : ptr( ptr )
	{
	}

	__device__ reference operator*() const
	{
		return *reinterpret_cast<pointer>( ptr );
	}

	__device__ pointer operator->() const
	{
		return reinterpret_cast<pointer>( ptr );
	}

	__device__ other_type_iterator& operator++()
	{
		ptr++;
		return *this;
	}

	__device__ other_type_iterator& operator--()
	{
		ptr--;
		return *this;
	}

	__device__ bool operator==( const other_type_iterator& other ) const
	{
		return ptr == other.ptr;
	}

	__device__ bool operator!=( const other_type_iterator& other ) const
	{
		return ptr != other.ptr;
	}
};

/**
 * Stack storage for all Variants, that are all subclasses
 * of Base.
 */
template <typename Base, typename... Variants>
class VariantStore
{
	using StorageType = typename StorageRequirement<Variants...>::type;

	static constexpr auto max_elements = 8;
	StorageType stack[max_elements];
	size_t items = 0;

	// Disable warnings about dropping type attributes
	// While this doesn't contribute towards correctness,
	// it works like it should right now.
#if defined __GNUC__ && __GNUC__ >= 6
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
#endif

	typedef other_type_iterator<Base, StorageType*> iterator;
	typedef other_type_iterator<const Base, const StorageType*> const_iterator;

#if defined __GNUC__ && __GNUC__ >= 6
#pragma GCC diagnostic pop
#endif

  public:
	__device__ size_t size() const
	{
		return items;
	}

	__device__ iterator begin()
	{
		return stack;
	}

	__device__ const const_iterator begin() const
	{
		return stack;
	}

	__device__ iterator end()
	{
		return stack + items;
	}

	__device__ const const_iterator end() const
	{
		return stack + items;
	}

	__device__ Base& operator[]( size_t idx )
	{
		assert( idx < items );
		return *reinterpret_cast<Base*>( stack + idx );
	}

	__device__ const Base& operator[]( size_t idx ) const
	{
		assert( idx < items );
		return *reinterpret_cast<const Base*>( stack + idx );
	}

	// Pass type to copy proper size:
	template <typename T>
	__device__ void push_back( const T& bxdf )
	{
		new ( Reserve() ) std::decay_t<T>( bxdf );
	}

	__device__ void* Reserve()
	{
		assert( items < max_elements );
		return stack + items++;
	}
};

__device__ void compile_time_tests()
{
	struct Base
	{
		float a;
		__device__ Base( float a ) : a( a ) {}
	};

	struct Thing : public Base
	{
		int x;
		__device__ Thing( float a, int x ) : Base( a ), x( x ) {}
	};

	VariantStore<Base, Thing> store;

	store.push_back( Thing( 1.f, 1 ) );
	auto nonconst = Thing( 1.f, 1 );
	store.push_back( nonconst );
	const auto cnst = Thing( 1.f, 1 );
	store.push_back( cnst );
}
