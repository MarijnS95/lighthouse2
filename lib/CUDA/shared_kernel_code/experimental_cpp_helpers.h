#pragma once

#if __cplusplus <= 201700UL
namespace std
{
template <class F, class Tuple, size_t... Is>
LH2_DEVFUNC constexpr auto apply_impl( F f, Tuple t,
									   std::index_sequence<Is...> )
{
	return f( std::get<Is>( t )... );
}

template <class F, class Tuple>
LH2_DEVFUNC constexpr auto apply( F f, Tuple t )
{
	return apply_impl(
		f, t, std::make_index_sequence<std::tuple_size<Tuple>::value>{} );
}
} // namespace std
#endif
