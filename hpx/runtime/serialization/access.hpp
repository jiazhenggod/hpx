//  Copyright (c) 2014 Thomas Heller
//  Copyright (c) 2014-2015 Anton Bikineev
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HPX_SERIALIZATION_ACCESS_HPP
#define HPX_SERIALIZATION_ACCESS_HPP

#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/traits/polymorphic_traits.hpp>
#include <hpx/util/decay.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace hpx { namespace serialization
{
    namespace detail
    {
        template <class T> HPX_FORCEINLINE
        void serialize_force_adl(output_archive& ar, const T& t, unsigned)
        {
            serialize(ar, const_cast<T&>(t), 0);
        }

        template <class T> HPX_FORCEINLINE
        void serialize_force_adl(input_archive& ar, T& t, unsigned)
        {
            serialize(ar, t, 0);
        }
    }

    class access
    {
        template <class T>
        class has_serialize
        {
            template <class T1> static std::false_type test(...);

            // the following expression sfinae trick
            // appears to work on clang-3.4, gcc-4.9,
            // icc-16, msvc-2017 (at least)
            // note that this detection would have been much easier
            // to implement if there hadn't been an issue with gcc:
            // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82478
            template <class T1, class = decltype(
                    std::declval<typename std::remove_const<T1>::type &>().
                        serialize(std::declval<output_archive &>(), 0u))>
            static std::true_type test(int);

        public:
            static constexpr bool value = decltype(test<T>(0))::value;
        };

        template <class T>
        class serialize_dispatcher
        {
            struct intrusive_polymorphic
            {
                // both following template functions are viable
                // to call right overloaded function according to T constness
                // and to prevent calling templated version of serialize function
                static void call(hpx::serialization::input_archive& ar, T& t, unsigned)
                {
                    t.serialize(ar, 0);
                }

                static void call(hpx::serialization::output_archive& ar,
                    const T& t, unsigned)
                {
                    t.serialize(ar, 0);
                }
            };

            struct non_intrusive
            {
                // this additional indirection level is needed to
                // force ADL on the second phase of template lookup.
                // call of serialize function directly from base_object
                // finds only serialize-member function and doesn't
                // perform ADL
                template <class Archive>
                static void call(Archive& ar, T& t, unsigned)
                {
                    detail::serialize_force_adl(ar, t, 0);
                }
            };

            struct empty
            {
                template <class Archive>
                static void call(Archive&, T&, unsigned)
                {
                }
            };

            struct intrusive_usual
            {
                template <class Archive>
                static void call(Archive& ar, T& t, unsigned)
                {
                    // cast it to let it be run for templated
                    // member functions
                    const_cast<typename util::decay<T>::type&>(
                            t).serialize(ar, 0);
                }
            };

        public:
            typedef typename std::conditional<
                hpx::traits::is_intrusive_polymorphic<T>::value,
                intrusive_polymorphic,
                typename std::conditional<
                    has_serialize<T>::value,
                    intrusive_usual,
                    typename std::conditional<
                        std::is_empty<T>::value,
                        empty,
                        non_intrusive
                    >::type
                >::type
            >::type type;
        };

    public:
        template <class Archive, class T>
        static void serialize(Archive& ar, T& t, unsigned)
        {
            serialize_dispatcher<T>::type::call(ar, t, 0);
        }

        template <typename Archive, typename T> HPX_FORCEINLINE
        static void save_base_object(Archive & ar, const T & t, unsigned)
        {
            // explicitly specify virtual function
            // of base class to avoid infinite recursion
            t.T::save(ar, 0);
        }

        template <typename Archive, typename T> HPX_FORCEINLINE
        static void load_base_object(Archive & ar, T & t, unsigned)
        {
            // explicitly specify virtual function
            // of base class to avoid infinite recursion
            t.T::load(ar, 0);
        }

        template <typename T> HPX_FORCEINLINE
        static std::string get_name(const T* t)
        {
            return t->hpx_serialization_get_name();
        }
    };

}}

#endif
