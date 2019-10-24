#pragma once

#include <map>

template <uint32_t _ForcedAlignment = 1>
class Flattener
{
	// std::map<const void *, uint32_t> insertedItems;
	uint32_t filled = 0, allocated_size = 0;
	char* _data = nullptr;

  public:
	enum
	{
		ForcedAlignment = _ForcedAlignment
	};

	inline Flattener()
	{
		ensureSize( 4096 );
	}

	inline uint32_t size() const
	{
		return filled;
	}

	inline void* data()
	{
		return _data;
	}

	inline void ensureSize( uint32_t requestedSize )
	{
		if ( !_data )
		{
			allocated_size = requestedSize;
			_data = (char*)malloc( allocated_size );
		}
		else if ( requestedSize >= allocated_size )
		{
			allocated_size = std::max( allocated_size * 2, requestedSize );
			_data = (char*)realloc( _data, allocated_size );
		}
	}

	char* addr( uint32_t offset )
	{
		return _data + offset * ForcedAlignment;
	}

	template <typename T>
	class reference
	{
		Flattener& container;
		uint32_t _offset;

	  public:
		reference( Flattener& f, uint32_t o ) : container( f ), _offset( o ) {}

		T* operator->()
		{
			return reinterpret_cast<T*>( container.addr( _offset ) );
		}

		T& operator*()
		{
			return *reinterpret_cast<T*>( container.addr( _offset ) );
		}

		const T& operator*() const
		{
			return *reinterpret_cast<const T*>( container.addr( _offset ) );
		}

		uint32_t offset() const
		{
			return _offset;
		}
	};

	uint32_t reserve( const uint32_t s, uint32_t align )
	{
		align = std::max( (uint32_t)ForcedAlignment, align );

		// Step 1: Calculate where to start:
		const uint32_t left = ( uint32_t )( (uint64_t)data() + filled ) % align;
		if ( left )
			filled += align - left;

		// Step 2: Record start of object
		const uint32_t location = filled;
		// Step 3: Claim memory for object:
		filled += s;
		ensureSize( filled );
		return location / ForcedAlignment;
	}

	template <typename T, typename... Args>
	reference<std::decay_t<T>> emplace_back( Args&&... args )
	{
		// Decay template argument into the underlying type:
		// (This removes const qualifiers and references)
		using simple_type = std::decay_t<T>;

		auto offset = reserve( sizeof( simple_type ), alignof( simple_type ) );

		new ( addr( offset ) ) simple_type( std::forward<Args>( args )... );

		return {*this, offset};
	}

	// Overload for emplacing an object on the stack, invoking the
	// copy/move constructor. This is a helper to resolve T
	// as the first "Args" type
	template <typename T>
	reference<std::decay_t<T>> emplace_back( T&& arg )
	{
		return emplace_back<T, T>( std::forward<T>( arg ) );
	}

	template <typename T>
	reference<std::decay_t<T>> push_back( T&& arg )
	{
		return emplace_back<T>( std::forward<T>( arg ) );
	}

	template <typename T>
	reference<std::decay_t<T>> get( uint32_t offset )
	{
		return {*this, offset};
	}

	// uint32_t IsFlattened(const void *p) const {
	// 	try {
	// 		return insertedItems.at(p);
	// 	} catch (std::out_of_range &e) {
	// 		return -1;
	// 	}
	// }

	void markFlattened( const void* p, uint32_t loc )
	{
		// insertedItems[p] = loc;
	}
};
