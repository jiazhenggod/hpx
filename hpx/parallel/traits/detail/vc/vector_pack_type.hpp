//  Copyright (c) 2016 Hartmut Kaiser
//  Copyright (c) 2016 Matthias Kretz
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_PARALLEL_TRAITS_VECTOR_PACK_TYPE_VC_OCT_31_2016_1229PM)
#define HPX_PARALLEL_TRAITS_VECTOR_PACK_TYPE_VC_OCT_31_2016_1229PM

#include <hpx/config.hpp>

#if defined(HPX_HAVE_DATAPAR_VC)

#include <cstddef>
#include <type_traits>

#include <Vc/Vc>

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace parallel { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T, std::size_t N, typename Abi>
        struct vector_pack_type
        {
            typedef typename std::conditional<
                    std::is_void<Abi>::value, Vc::VectorAbi::Best<T>, Abi
                >::type abi_type;

            typedef Vc::Vector<T, abi_type> type;
        };

        template <typename T, typename Abi>
        struct vector_pack_type<T, 1, Abi>
        {
            typedef Vc::Scalar::Vector<T> type;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, std::size_t N, typename Abi>
    struct vector_pack_type
      : detail::vector_pack_type<T, N, Abi>
    {};
}}}

#endif
#endif
